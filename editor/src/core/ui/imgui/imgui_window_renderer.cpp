#include "imgui_window_renderer.h"

#include "imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"

#include "../seika/src/rendering/renderer.h"
#include "../seika/src/rendering/frame_buffer.h"

#include "../../editor_context.h"

const Color WINDOW_BACKGROUND_COLOR = {0.1f, 0.1f, 0.1f, 1.0f };

void ImGuiHelper::WindowRenderer::Render(const std::vector<TextureRenderTarget>& textureRenderTargets, const std::vector<FontRenderTarget>& fontRenderTargets) {
    // New frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(EditorContext::Get()->window);
    // Queue draw calls within engine code paths
    // Textures
    for (auto& target : textureRenderTargets) {
        if (target.texture) {
            se_renderer_queue_sprite_draw_call(target.texture, target.sourceRect, target.destSize, target.color,
                                               target.flipX, target.flipY, target.globalTransform, target.zIndex);
        }
    }
    // Fonts
    for (auto& target : fontRenderTargets) {
        if (target.font) {
            se_renderer_queue_font_draw_call(target.font, target.text.c_str(), target.position.x, target.position.y,
                                             target.scale, target.color, target.zIndex);
        }
    }
    // Flush queued calls and render to framebuffer
    se_renderer_process_and_flush_batches_just_framebuffer(&WINDOW_BACKGROUND_COLOR);
    // Add screen texture from framebuffer to draw list
    const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    ImGui::GetWindowDrawList()->AddImage(
        (ImTextureID) se_frame_buffer_get_color_buffer_texture(),
        cursorPos,
        ImVec2(cursorPos.x + windowSize.x, cursorPos.y + windowSize.y),
        ImVec2(0.0f, 1.0f),
        ImVec2(1.0f, 0.0f)
    );
}
