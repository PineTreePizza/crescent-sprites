#pragma once

#include <string>
#include <utility>
#include <vector>

#include "utils/json_helper.h"
#include "utils/singleton.h"

struct TextureAsset {
    TextureAsset() = default;

    explicit TextureAsset(const nlohmann::json& texture)
        : file_path(JsonHelper::Get<std::string>(texture, "file_path")),
          wrap_s(JsonHelper::Get<std::string>(texture, "wrap_s")),
          wrap_t(JsonHelper::Get<std::string>(texture, "wrap_t")),
          filter_min(JsonHelper::Get<std::string>(texture, "filter_min")),
          filter_mag(JsonHelper::Get<std::string>(texture, "filter_mag")) {}

    TextureAsset(std::string filePath,
                 std::string wrapS = "clamp_to_border",
                 std::string wrapT = "clamp_to_border",
                 std::string filterMin = "nearest",
                 std::string filterMag = "nearest")
        : file_path(std::move(filePath)),
          wrap_s(std::move(wrapS)),
          wrap_t(std::move(wrapT)),
          filter_min(std::move(filterMin)),
          filter_mag(std::move(filterMag)) {}

    std::string file_path;
    std::string wrap_s;
    std::string wrap_t;
    std::string filter_min;
    std::string filter_mag;
};

struct AudioSourceAsset {
    AudioSourceAsset() = default;

    explicit AudioSourceAsset(const nlohmann::json& audioSource) :
        file_path(JsonHelper::Get<std::string>(audioSource, "file_path")) {}

    AudioSourceAsset(std::string filePath)
        : file_path(std::move(filePath)) {}

    std::string file_path;
};

struct FontAsset {
    FontAsset() = default;

    explicit FontAsset(const nlohmann::json& font) :
        file_path(JsonHelper::Get<std::string>(font, "file_path")),
        uid(JsonHelper::Get<std::string>(font, "uid")),
        size(JsonHelper::Get<int>(font, "size")) {}

    std::string file_path;
    std::string uid;
    int size = -1;
};

struct ProjectAssets {
    std::vector<TextureAsset> textures;
    std::vector<AudioSourceAsset> audioSources;
    std::vector<FontAsset> fonts;

    void SetAssets(const nlohmann::json& assetsJson);
};

struct ProjectInputAction {
    std::string name;
    int deviceId = 0;
    std::vector<std::string> values;
};

struct ProjectInputs {
    std::vector<ProjectInputAction> actions;

    void SetInputs(const nlohmann::json& inputsJsonArray);
};

class ProjectProperties : public Singleton<ProjectProperties> {
  public:
    std::string gameTitle;
    std::string initialNodePath;
    int windowWidth;
    int windowHeight;
    int resolutionWidth;
    int resolutionHeight;
    int targetFPS;
    bool areCollidersVisible = false;
    ProjectAssets assets;
    ProjectInputs inputs;
    std::string projectPath;

    ProjectProperties(singleton);

    nlohmann::ordered_json ToJson() const;
    void ResetToDefault();

    void LoadPropertiesFromConfig(const char* filePath);
    void PrintProperties() const;
    void UpdateTextureAsset(const TextureAsset& textureAsset);
    void UpdateAudioSourceAsset(const AudioSourceAsset& audioSourceAsset);
    TextureAsset& GetTextureAsset(const std::string& texturePath);
    bool HasTextureWithPath(const std::string& path) const;
    bool HasAudioSourceWithPath(const std::string& path) const;
    std::string GetPathRelativeToProjectPath(const std::string& relativePath);
};
