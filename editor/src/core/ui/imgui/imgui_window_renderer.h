#pragma once

#include <string>
#include <vector>

#include "../seika/src/math/se_math.h"

struct Texture;
struct Font;

namespace ImGuiHelper {
struct TextureRenderTarget {
    Texture *texture = nullptr;
    Rect2 sourceRect{0.0f, 0.0f, 0.0f, 0.0f};
    Size2D destSize = {0.0f, 0.0f};
    Color color = {1.0f, 1.0f, 1.0f, 1.0f};
    bool flipX = false;
    bool flipY = false;
    TransformModel2D *globalTransform = nullptr;
    int zIndex = 0;
};

struct FontRenderTarget {
    Font* font = nullptr;
    std::string text;
    Vector2 position = { 0.0f, 0.0f };
    float scale = 1.0f;
    Color color = {1.0f, 1.0f, 1.0f, 1.0f};
    int zIndex = 0;
};

namespace WindowRenderer {
void Render(const std::vector<TextureRenderTarget> &textureRenderTargets, const std::vector<FontRenderTarget>& fontRenderTargets);
} // namespace WindowRenderer
}
