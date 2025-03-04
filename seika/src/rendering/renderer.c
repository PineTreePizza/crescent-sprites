#include "renderer.h"

#include <stdlib.h>

#include "render_context.h"
#include "shader.h"
#include "shader_source.h"
#include "../data_structures/se_hash_map.h"
#include "../data_structures/se_static_array.h"
#include "../utils/se_assert.h"

#define SE_RENDER_TO_FRAMEBUFFER
#define SE_RENDER_LAYER_BATCH_MAX 200
#define SE_RENDER_LAYER_BATCH_ITEM_MAX (SE_RENDER_LAYER_BATCH_MAX / 2)

#ifdef SE_RENDER_TO_FRAMEBUFFER
#include "frame_buffer.h"
#endif

typedef struct TextureCoordinates {
    GLfloat sMin;
    GLfloat sMax;
    GLfloat tMin;
    GLfloat tMax;
} TextureCoordinates;

void sprite_renderer_initialize();
void sprite_renderer_finalize();
void sprite_renderer_update_resolution();
void sprite_renderer_draw_sprite(const Texture* texture, const Rect2* sourceRect, const Size2D* destSize, const Color *color, bool flipX, bool flipY, TransformModel2D* globalTransform);
void font_renderer_initialize();
void font_renderer_finalize();
void font_renderer_update_resolution();
void font_renderer_draw_text(const Font* font, const char* text, float x, float y, float scale, const Color* color);

TextureCoordinates renderer_get_texture_coordinates(const Texture* texture, const Rect2* drawSource, bool flipX, bool flipY);
void renderer_print_opengl_errors();

static GLuint spriteQuadVAO;
static GLuint spriteQuadVBO;

static Shader* spriteShader = NULL;
static Shader* fontShader = NULL;

static float resolutionWidth = 800.0f;
static float resolutionHeight = 600.0f;

// Sprite Batching
typedef struct SpriteBatchItem {
    Texture* texture;
    Rect2 sourceRect;
    Size2D destSize;
    Color color;
    bool flipX;
    bool flipY;
    TransformModel2D* globalTransform;
} SpriteBatchItem;

typedef struct FontBatchItem {
    Font* font;
    const char* text;
    float x;
    float y;
    float scale;
    Color color;
} FontBatchItem;

// Render Layer - Arranges draw order by z index
typedef struct RenderLayer {
    SpriteBatchItem spriteBatchItems[SE_RENDER_LAYER_BATCH_ITEM_MAX];
    FontBatchItem fontBatchItems[SE_RENDER_LAYER_BATCH_ITEM_MAX];
    size_t spriteBatchItemCount;
    size_t fontBatchItemCount;
} RenderLayer;

SE_STATIC_ARRAY_CREATE(RenderLayer, SE_RENDER_LAYER_BATCH_MAX, render_layer_items);
SE_STATIC_ARRAY_CREATE(int, SE_RENDER_LAYER_BATCH_MAX, active_render_layer_items_indices);

// Renderer
void se_renderer_initialize(int inWindowWidth, int inWindowHeight, int inResolutionWidth, int inResolutionHeight) {
    resolutionWidth = (float) inResolutionWidth;
    resolutionHeight = (float) inResolutionHeight;
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    se_render_context_initialize();
    se_renderer_update_window_size((float) inWindowWidth, (float) inWindowHeight);
    sprite_renderer_initialize();
    font_renderer_initialize();
    // Initialize framebuffer
#ifdef SE_RENDER_TO_FRAMEBUFFER
    SE_ASSERT_FMT(se_frame_buffer_initialize(inWindowWidth, inWindowHeight), "Framebuffer didn't initialize!");
#endif
    // Set initial data for render layer
    for (size_t i = 0; i < SE_RENDER_LAYER_BATCH_MAX; i++) {
        render_layer_items[i].spriteBatchItemCount = 0;
        render_layer_items[i].fontBatchItemCount = 0;
    }
}

void se_renderer_finalize() {
    font_renderer_finalize();
    sprite_renderer_finalize();
    se_render_context_finalize();
#ifdef SE_RENDER_TO_FRAMEBUFFER
    se_frame_buffer_finalize();
#endif
}

void se_renderer_update_window_size(float windowWidth, float windowHeight) {
    const int width = (int) windowWidth;
    const int height = (int) windowHeight;
    RenderContext* renderContext = se_render_context_get();
    renderContext->windowWidth = width;
    renderContext->windowHeight = height;
#ifdef SE_RENDER_TO_FRAMEBUFFER
    se_frame_buffer_resize_texture(width, height);
#endif
}

void update_active_render_layer_index(int zIndex) {
    const size_t sizeBefore = SE_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    SE_STATIC_ARRAY_ADD_IF_UNIQUE(active_render_layer_items_indices, zIndex);
    const size_t sizeAfter = SE_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    if (sizeBefore != sizeAfter) {
        SE_STATIC_ARRAY_SORT_INT(active_render_layer_items_indices);
    }
}

void se_renderer_queue_sprite_draw_call(Texture* texture, Rect2 sourceRect, Size2D destSize, Color color, bool flipX, bool flipY, TransformModel2D* globalTransform, int zIndex) {
    if (texture == NULL) {
        se_logger_error("NULL texture, not submitting draw call!");
        return;
    }
    SpriteBatchItem item = { .texture = texture, .sourceRect = sourceRect, .destSize = destSize, .color = color, .flipX = flipX, .flipY = flipY, .globalTransform = globalTransform };
    const int arrayZIndex = se_math_clamp_int(zIndex + SE_RENDER_LAYER_BATCH_MAX / 2, 0, SE_RENDER_LAYER_BATCH_MAX - 1);
    // Update sprite batch item on render layer
    render_layer_items[arrayZIndex].spriteBatchItems[render_layer_items[arrayZIndex].spriteBatchItemCount++] = item;
    // Update active render layer indices
    update_active_render_layer_index(arrayZIndex);
}

void se_renderer_queue_font_draw_call(Font* font, const char* text, float x, float y, float scale, Color color, int zIndex) {
    if (font == NULL) {
        se_logger_error("NULL font, not submitting draw call!");
        return;
    }

    FontBatchItem item = { .font = font, .text = text, .x = x, .y = y, .scale = scale, .color = color };
    const int arrayZIndex = se_math_clamp_int(zIndex + SE_RENDER_LAYER_BATCH_MAX / 2, 0, SE_RENDER_LAYER_BATCH_MAX - 1);
    // Update font batch item on render layer
    render_layer_items[arrayZIndex].fontBatchItems[render_layer_items[arrayZIndex].fontBatchItemCount++] = item;
    // Update active render layer indices
    update_active_render_layer_index(arrayZIndex);
}

void se_renderer_flush_batches() {
    for (size_t i = 0; i < active_render_layer_items_indices_count; i++) {
        const size_t layerIndex = active_render_layer_items_indices[i];
        // Sprite
        for (size_t spriteIndex = 0; spriteIndex < render_layer_items[layerIndex].spriteBatchItemCount; spriteIndex++) {
            sprite_renderer_draw_sprite(
                render_layer_items[layerIndex].spriteBatchItems[spriteIndex].texture,
                &render_layer_items[layerIndex].spriteBatchItems[spriteIndex].sourceRect,
                &render_layer_items[layerIndex].spriteBatchItems[spriteIndex].destSize,
                &render_layer_items[layerIndex].spriteBatchItems[spriteIndex].color,
                render_layer_items[layerIndex].spriteBatchItems[spriteIndex].flipX,
                render_layer_items[layerIndex].spriteBatchItems[spriteIndex].flipY,
                render_layer_items[layerIndex].spriteBatchItems[spriteIndex].globalTransform
            );
        }
        render_layer_items[layerIndex].spriteBatchItemCount = 0;
        // Font
        for (size_t fontIndex = 0; fontIndex < render_layer_items[layerIndex].fontBatchItemCount; fontIndex++) {
            font_renderer_draw_text(
                render_layer_items[layerIndex].fontBatchItems[fontIndex].font,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].text,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].x,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].y,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].scale,
                &render_layer_items[layerIndex].fontBatchItems[fontIndex].color
            );
        }
        render_layer_items[layerIndex].fontBatchItemCount = 0;
    }

    SE_STATIC_ARRAY_EMPTY(render_layer_items);
    SE_STATIC_ARRAY_EMPTY(active_render_layer_items_indices);
}

void se_renderer_process_and_flush_batches(const Color* backgroundColor) {
#ifdef SE_RENDER_TO_FRAMEBUFFER
    se_frame_buffer_bind();
#endif

    // Clear framebuffer with background color
    glClearColor(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    se_renderer_flush_batches();

#ifdef SE_RENDER_TO_FRAMEBUFFER
    se_frame_buffer_unbind();

    // Clear screen texture background
    static Color screenBackgroundColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glClearColor(screenBackgroundColor.r, screenBackgroundColor.g, screenBackgroundColor.b, screenBackgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw screen texture from framebuffer
    Shader* screenShader = se_frame_buffer_get_screen_shader();
    shader_use(screenShader);
    glBindVertexArray(se_frame_buffer_get_quad_vao());
    glBindTexture(GL_TEXTURE_2D, se_frame_buffer_get_color_buffer_texture());	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
}

#ifdef SE_RENDER_TO_FRAMEBUFFER
void se_renderer_process_and_flush_batches_just_framebuffer(const Color* backgroundColor) {
    se_frame_buffer_bind();

    // Clear framebuffer with background color
    glClearColor(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    se_renderer_flush_batches();

    se_frame_buffer_unbind();
}
#endif

// --- Sprite Renderer --- //
void sprite_renderer_initialize() {
    GLfloat vertices[] = {
        //id          // positions       // texture coordinates // color
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f
    };

    // Initialize render data
    glGenVertexArrays(1, &spriteQuadVAO);
    glGenBuffers(1, &spriteQuadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, spriteQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(spriteQuadVAO);
    // id attribute
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*) NULL);
    glEnableVertexAttribArray(0);
    // position attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // texture coords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    // color attribute
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // compile shaders
    spriteShader = shader_compile_new_shader(OPENGL_SHADER_SOURCE_VERTEX_SPRITE, OPENGL_SHADER_SOURCE_FRAGMENT_SPRITE);
    sprite_renderer_update_resolution();
    shader_set_int(spriteShader, "sprite", 0);
}

void sprite_renderer_finalize() {}

void sprite_renderer_update_resolution() {
    mat4 proj = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    glm_ortho(0.0f, resolutionWidth, resolutionHeight, 0.0f, -1.0f, 1.0f, proj);
    shader_use(spriteShader);
    shader_set_mat4_float(spriteShader, "projection", &proj);
}

// TODO: Just need to pass in destination size instead of rect2
void sprite_renderer_draw_sprite(const Texture* texture, const Rect2* sourceRect, const Size2D* destSize, const Color* color, bool flipX, bool flipY, TransformModel2D* globalTransform) {
    glDepthMask(false);

    glm_scale(globalTransform->model, (vec3) {
        destSize->w, destSize->h, 1.0f
    });

    glBindVertexArray(spriteQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, spriteQuadVBO);

    shader_use(spriteShader);
    shader_set_mat4_float(spriteShader, "models[0]", &globalTransform->model); // TODO: Batch models
    const int VERTEX_ITEM_COUNT = 1;
    const int NUMBER_OF_VERTICES = 6;
    const float SPRITE_ID = 0.0f;
    TextureCoordinates textureCoords = renderer_get_texture_coordinates(texture, sourceRect, flipX, flipY);

    const float determinate = glm_mat4_det(globalTransform->model);
    const int vertsStride = 9;
    GLfloat verts[54]; // TODO: fix magic number
    for (int i = 0; i < VERTEX_ITEM_COUNT * NUMBER_OF_VERTICES; i++) {
        bool isSMin;
        bool isTMin;
        if (determinate >= 0.0f) {
            isSMin = i == 0 || i == 2 || i == 3 ? true : false;
            isTMin = i == 1 || i == 2 || i == 5 ? true : false;
        } else {
            isSMin = i == 1 || i == 2 || i == 5 ? true : false;
            isTMin = i == 0 || i == 2 || i == 3 ? true : false;
        }
        const int row = i * vertsStride;
        verts[row + 0] = SPRITE_ID;
        verts[row + 1] = isSMin ? 0.0f : 1.0f;
        verts[row + 2] = isTMin ? 0.0f : 1.0f;
        verts[row + 3] = isSMin ? textureCoords.sMin : textureCoords.sMax;
        verts[row + 4] = isTMin ? textureCoords.tMin : textureCoords.tMax;
        verts[row + 5] = color->r;
        verts[row + 6] = color->g;
        verts[row + 7] = color->b;
        verts[row + 8] = color->a;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, VERTEX_ITEM_COUNT * NUMBER_OF_VERTICES);

    renderer_print_opengl_errors();

    glBindVertexArray(0);
    glDepthMask(true);
}

// --- Font Renderer --- //
void font_renderer_initialize() {
    if (FT_Init_FreeType(&se_render_context_get()->freeTypeLibrary)) {
        se_logger_error("Unable to initialize FreeType library!");
    }
    fontShader = shader_compile_new_shader(OPENGL_SHADER_SOURCE_VERTEX_FONT, OPENGL_SHADER_SOURCE_FRAGMENT_FONT);
    font_renderer_update_resolution();
}

void font_renderer_finalize() {
    FT_Done_FreeType(se_render_context_get()->freeTypeLibrary);
}

void font_renderer_update_resolution() {
    mat4 proj = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    glm_ortho(0.0f, resolutionWidth, -resolutionHeight, 0.0f, -1.0f, 1.0f, proj);
    shader_use(fontShader);
    shader_set_mat4_float(fontShader, "projection", &proj);
}

void font_renderer_draw_text(const Font* font, const char* text, float x, float y, float scale, const Color* color) {
    Vector2 currentScale = { scale, scale };
    shader_use(fontShader);
    shader_set_vec4_float(fontShader, "textColor", color->r, color->g, color->b, color->a);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);

    // Iterate through all characters
    char* c = (char*) &text[0];
    const size_t textLength = strlen(text);
    for (size_t i = 0; i < textLength; i++) {
        Character ch = font->characters[(int) *c];
        const float xPos = x + (ch.bearing.x * currentScale.x);
        const float yPos = -y - (ch.size.y - ch.bearing.y) * currentScale.x; // Invert Y because othographic projection is flipped
        const float w = ch.size.x * currentScale.x;
        const float h = ch.size.y * currentScale.y;
        // Update VBO for each characters
        GLfloat verts[6][4] = {
            {xPos,     yPos + h, 0.0f, 0.0f},
            {xPos,     yPos,     0.0f, 1.0f},
            {xPos + w, yPos,     1.0f, 1.0f},

            {xPos,     yPos + h, 0.0f, 0.0f},
            {xPos + w, yPos,     1.0f, 1.0f},
            {xPos + w, yPos + h, 1.0f, 0.0f}
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureId);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (float) (ch.advance >> 6) * currentScale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        ++c;
    }
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Misc --- //
TextureCoordinates renderer_get_texture_coordinates(const Texture* texture, const Rect2* drawSource, bool flipX, bool flipY) {
    // S
    GLfloat sMin, sMax;
    if (flipX) {
        sMax = (drawSource->x + 0.5f) / (float) texture->width;
        sMin = (drawSource->x + drawSource->w - 0.5f) / (float) texture->width;
    } else {
        sMin = (drawSource->x + 0.5f) / (float) texture->width;
        sMax = (drawSource->x + drawSource->w - 0.5f) / (float) texture->width;
    }
    // T
    GLfloat tMin, tMax;
    if (flipY) {
        tMax = (drawSource->y + 0.5f) / (float) texture->height;
        tMin = (drawSource->y + drawSource->h - 0.5f) / (float) texture->height;
    } else {
        tMin = (drawSource->y + 0.5f) / (float) texture->height;
        tMax = (drawSource->y + drawSource->h - 0.5f) / (float) texture->height;
    }
    TextureCoordinates textureCoords = { sMin, sMax, tMin, tMax };
    return textureCoords;
}

void renderer_print_opengl_errors() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("err = %d\n", err);
        switch (err) {
        case GL_NO_ERROR:
            printf("GL_NO_ERROR\n");
            break;
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM\n");
            break;
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE\n");
            break;
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION\n");
            break;
        default:
            printf("default\n");
            break;
        }
    }
}
