#include "imgui_helper.h"
#include "imgui_internal.h"

#include <utility>

#include "../seika/src/utils/logger.h"

#include "../../asset_browser.h"

static ImGuiHelper::Context* context = ImGuiHelper::Context::Get();

//--- MenuBar ---//
void ImGuiHelper::BeginMainMenuBar(const ImGuiHelper::MenuBar& menuBar) {
    // Create Menu Bar
    if (ImGui::BeginMainMenuBar()) {
        // Create Menus
        for (const ImGuiHelper::Menu& menu : menuBar.menus) {
            if (ImGui::BeginMenu(menu.name)) {
                // Create Menu Items
                for (const ImGuiHelper::MenuItem& menuItem : menu.menuItems) {
                    if (ImGui::MenuItem(menuItem.name, menuItem.keyboardShortcut)) {
                        menuItem.callbackFunc(context);
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
}

//--- Popup Modal ---//
void ImGuiHelper::BeginPopupModal(const ImGuiHelper::PopupModal& popupModal) {
    if (popupModal.position.has_value()) {
        ImGui::SetNextWindowPos(*popupModal.position, ImGuiCond_Once);
    }
    if (popupModal.size.has_value()) {
        ImGui::SetNextWindowSize(*popupModal.size, ImGuiCond_Once);
    }
    if (ImGui::BeginPopupModal(popupModal.name, popupModal.open, popupModal.windowFlags)) {
        popupModal.callbackFunc(context);
        ImGui::EndPopup();
    }
}

//--- Input Text ---//
ImGuiHelper::InputText::InputText(const std::string &label, std::string &value, int labelIndex)
    : buffer(std::make_unique<char[]>(bufferSize + 1)),
      label(label),
      value(value) {
    internalLabel = "###" + std::to_string(labelIndex) + label;
    SetValue(value);
}

void ImGuiHelper::InputText::SetValue(std::string value) {
    if (value.size() > bufferSize) {
        value = value.substr(bufferSize);
    }
    std::ranges::copy(value, buffer.get());
    buffer[value.size()] = '\0';
}

std::string ImGuiHelper::InputText::GetValue() const {
    return { buffer.get() };
}

const char* ImGuiHelper::InputText::GetInternalLabel() const {
    return internalLabel.c_str();
}

bool ImGuiHelper::InputText::HasValue() const {
    if (buffer) {
        return true;
    }
    return false;
}

void ImGuiHelper::BeginInputText(const InputText& inputText) {
    if (!inputText.label.empty()) {
        ImGui::Text("%s", inputText.label.c_str());
        ImGui::SameLine();
    }
    if (ImGui::InputText(inputText.GetInternalLabel(), inputText.buffer.get(), inputText.bufferSize, inputText.flags)) {
        inputText.value = inputText.GetValue();
    }
}

//--- Drag Int ---//
ImGuiHelper::DragInt::DragInt(std::string label, int& value, int labelIndex)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char* ImGuiHelper::DragInt::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginDragInt(const DragInt& dragInt) {
    if (!dragInt.label.empty()) {
        ImGui::Text("%s", dragInt.label.c_str());
        ImGui::SameLine();
    }
    ImGui::DragInt(dragInt.GetInternalLabel(), &dragInt.value, dragInt.valueSpeed, dragInt.valueMin, dragInt.valueMax);
}

//--- Drag Float ---//
ImGuiHelper::DragFloat::DragFloat(std::string label, float &value, int labelIndex)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char* ImGuiHelper::DragFloat::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginDragFloat(const DragFloat& dragFloat) {
    if (!dragFloat.label.empty()) {
        ImGui::Text("%s", dragFloat.label.c_str());
        ImGui::SameLine();
    }
    ImGui::DragFloat(dragFloat.GetInternalLabel(), &dragFloat.value, dragFloat.valueSpeed, dragFloat.valueMin, dragFloat.valueMax, dragFloat.format);
}

//--- Drag Float 2 ---//
ImGuiHelper::DragFloat2::DragFloat2(std::string label, float* value, int labelIndex)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char* ImGuiHelper::DragFloat2::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginDragFloat2(const DragFloat2& dragFloat2) {
    if (!dragFloat2.label.empty()) {
        ImGui::Text("%s", dragFloat2.label.c_str());
        ImGui::SameLine();
    }
    ImGui::DragFloat2(dragFloat2.GetInternalLabel(), dragFloat2.value, dragFloat2.valueSpeed, dragFloat2.valueMin, dragFloat2.valueMax, dragFloat2.format);
}

//--- DragFloat4 ---//
ImGuiHelper::DragFloat4::DragFloat4(std::string label, float *value, int labelIndex)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char *ImGuiHelper::DragFloat4::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginDragFloat4(const DragFloat4& dragFloat4) {
    if (!dragFloat4.label.empty()) {
        ImGui::Text("%s", dragFloat4.label.c_str());
        ImGui::SameLine();
    }
    ImGui::DragFloat4(dragFloat4.GetInternalLabel(), dragFloat4.value, dragFloat4.valueSpeed, dragFloat4.valueMin, dragFloat4.valueMax, dragFloat4.format);
}

//--- ColorEdit4 ---//
ImGuiHelper::ColorEdit4::ColorEdit4(std::string label, float *value, int labelIndex)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char *ImGuiHelper::ColorEdit4::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginColorEdit4(const ImGuiHelper::ColorEdit4 &colorEdit4) {
    if (!colorEdit4.label.empty()) {
        ImGui::Text("%s", colorEdit4.label.c_str());
        ImGui::SameLine();
    }
    ImGui::ColorEdit4(colorEdit4.GetInternalLabel(), colorEdit4.value, colorEdit4.flags);
}

//--- CheckBox ---//
ImGuiHelper::CheckBox::CheckBox(std::string label, bool &value)
    : label(std::move(label)),
      value(value) {
    internalLabel = "##" + this->label;
}

const char* ImGuiHelper::CheckBox::GetInternalLabel() const {
    return internalLabel.c_str();
}

void ImGuiHelper::BeginCheckBox(const CheckBox& checkBox) {
    if (!checkBox.label.empty()) {
        ImGui::Text("%s", checkBox.label.c_str());
        ImGui::SameLine();
    }
    ImGui::Checkbox(checkBox.GetInternalLabel(), &checkBox.value);
}

//--- ComboBox ---//
ImGuiHelper::ComboBox::ComboBox(std::string label, const std::vector<std::string> &items, std::function<void(const char* newItem)> onSelectionChangeCallback, int labelIndex)
    : label(std::move(label)),
      items(items),
      onSelectionChangeCallback(onSelectionChangeCallback ? std::move(onSelectionChangeCallback) : nullptr) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
}

const char* ImGuiHelper::ComboBox::GetInternalLabel() const {
    return internalLabel.c_str();
}

const char* ImGuiHelper::ComboBox::GetSelectedItem() const {
    if (selectedIndex < items.size()) {
        return items[selectedIndex].c_str();
    }
    return nullptr;
}

void ImGuiHelper::ComboBox::SetSelected(const std::string& itemToSelect, bool executeCallbacks) {
    for (int i = 0; i < items.size(); i++) {
        if (items[i] == itemToSelect) {
            selectedIndex = i;
            if (onSelectionChangeCallback && executeCallbacks) {
                onSelectionChangeCallback(items[i].c_str());
            }
            return;
        }
    }
    se_logger_error("Combo Box failed to select item '%s'", itemToSelect.c_str());
}

void ImGuiHelper::BeginComboBox(ImGuiHelper::ComboBox &comboBox) {
    if (!comboBox.label.empty()) {
        ImGui::Text("%s", comboBox.label.c_str());
        ImGui::SameLine();
    }
    const char* selectedItemText = comboBox.GetSelectedItem();
    if (ImGui::BeginCombo(comboBox.GetInternalLabel(), selectedItemText, 0)) {
        for (int i = 0; i < comboBox.items.size(); i++) {
            const bool is_selected = (comboBox.selectedIndex == i);
            if (ImGui::Selectable(comboBox.items[i].c_str(), is_selected)) {
                comboBox.selectedIndex = i;
                if (comboBox.onSelectionChangeCallback) {
                    comboBox.onSelectionChangeCallback(comboBox.GetSelectedItem());
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

//--- AssetBrowserComboBox ---//
ImGuiHelper::AssetBrowserComboBox::AssetBrowserComboBox(std::string label, std::string inExtension, std::function<void(const char *)> onSelectionChangeCallback, int labelIndex)
    : label(std::move(label)),
      extension(std::move(inExtension)),
      onSelectionChangeCallback(onSelectionChangeCallback ? std::move(onSelectionChangeCallback) : nullptr) {
    internalLabel = "##" + std::to_string(labelIndex) + this->label;
    AssetBrowser* assetBrowser = AssetBrowser::Get();
    assetBrowserHandle = assetBrowser->RegisterRefreshCallback([this](const FileNode& rootNode) {
        RefreshListFromBrowser();
    });
    RefreshListFromBrowser();
}

ImGuiHelper::AssetBrowserComboBox::~AssetBrowserComboBox() {
    AssetBrowser* assetBrowser = AssetBrowser::Get();
    assetBrowser->UnregisterRefreshCallback(assetBrowserHandle);
}

const char* ImGuiHelper::AssetBrowserComboBox::GetInternalLabel() const {
    return internalLabel.c_str();
}

const char* ImGuiHelper::AssetBrowserComboBox::GetSelectedItem() const {
    if (selectedIndex < items.size()) {
        return items[selectedIndex].c_str();
    }
    return nullptr;
}

void ImGuiHelper::AssetBrowserComboBox::SetSelected(const std::string& itemToSelect, bool executeCallbacks) {
    for (int i = 0; i < items.size(); i++) {
        if (items[i] == itemToSelect) {
            selectedIndex = i;
            if (onSelectionChangeCallback && executeCallbacks) {
                onSelectionChangeCallback(items[i].c_str());
            }
            return;
        }
    }
    se_logger_error("Asset Browser Combo Box failed to select item '%s'", itemToSelect.c_str());
}

void ImGuiHelper::AssetBrowserComboBox::RefreshListFromBrowser() {
    items.clear();
    items.emplace_back("<none>");
    static AssetBrowser* assetBrowser = AssetBrowser::Get();
    if (assetBrowser->fileCache.extensionToFileNodeMap.count(extension) > 0) {
        for (auto& fileNode : assetBrowser->fileCache.extensionToFileNodeMap[extension]) {
            items.emplace_back(fileNode.GetRelativePath());
        }
    }
}

void ImGuiHelper::BeginAssetBrowserComboBox(ImGuiHelper::AssetBrowserComboBox &comboBox) {
    if (!comboBox.label.empty()) {
        ImGui::Text("%s", comboBox.label.c_str());
        ImGui::SameLine();
    }
    const char* selectedItemText = comboBox.GetSelectedItem();
    if (ImGui::BeginCombo(comboBox.GetInternalLabel(), selectedItemText, 0)) {
        for (int i = 0; i < comboBox.items.size(); i++) {
            const bool is_selected = (comboBox.selectedIndex == i);
            if (ImGui::Selectable(comboBox.items[i].c_str(), is_selected)) {
                comboBox.selectedIndex = i;
                if (comboBox.onSelectionChangeCallback) {
                    comboBox.onSelectionChangeCallback(comboBox.GetSelectedItem());
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

//--- TreeNode ---//
void ImGuiHelper::BeginTreeNode(const ImGuiHelper::TreeNode &treeNode) {
    if (ImGui::TreeNodeEx(treeNode.label.c_str(), treeNode.flags)) {
        if (treeNode.callbackFunc) {
            treeNode.callbackFunc(context);
        }
        ImGui::TreePop();
    }
}

//--- Window ---//
void ImGuiHelper::BeginWindow(const ImGuiHelper::Window& window) {
    if (window.position.has_value()) {
        ImGui::SetNextWindowPos(*window.position, window.windowCond);
    }
    if (window.size.has_value()) {
        ImGui::SetNextWindowSize(*window.size, window.windowCond);
    }
    ImGui::Begin(window.name.c_str(), window.open, window.windowFlags);
    if (window.callbackFunc) {
        window.callbackFunc(context);
    }
}

void ImGuiHelper::BeginWindowWithEnd(const ImGuiHelper::Window& window) {
    BeginWindow(window);
    ImGui::End();
}

//--- DockSpace ---//
void ImGuiHelper::DockSpace::Run(bool runWindows) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    dockSpaceId = ImGui::GetID(id.c_str());
    ImVec2 dockPosition = viewport->Pos;
    ImVec2 dockSize = viewport->Size;
    dockPosition.y += 10.0f;
    dockSize.y -= dockPosition.y;

    ImGui::SetNextWindowPos(dockPosition, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
    ImGui::Begin("DockSpace Windows", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking
                );

    if (onMainWindowUpdateCallback) {
        onMainWindowUpdateCallback();
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    if (!hasBuilt) {
        // Run windows to have them defined...
        for (const auto& dockSpaceWindow : windows) {
            ImGuiHelper::BeginWindowWithEnd(dockSpaceWindow.window);
        }

        ImGui::DockBuilderRemoveNode(dockSpaceId); // clear any previous layout
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, dockSize);
        ImGui::DockBuilderSetNodePos(dockSpaceId, dockPosition);

        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.3f, nullptr, &dockSpaceId);
        ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.3f, nullptr, &dockSpaceId);
        ImGuiID dockLeftDownId = ImGui::DockBuilderSplitNode(dockLeftId, ImGuiDir_Down, 0.3f, nullptr, &dockLeftId);
        ImGuiID dockDownId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.3f, nullptr, &dockSpaceId);

        for (const auto& dockSpaceWindow : windows) {
            ImGuiID dockId = dockSpaceId;
            switch (dockSpaceWindow.position) {
            case DockSpacePosition::Main: {
                dockId = dockSpaceId;
                break;
            }
            case DockSpacePosition::Left: {
                dockId = dockLeftId;
                break;
            }
            case DockSpacePosition::Right: {
                dockId = dockRightId;
                break;
            }
            case DockSpacePosition::LeftDown: {
                dockId = dockLeftDownId;
                break;
            }
            case DockSpacePosition::Down: {
                dockId = dockDownId;
                break;
            }
            }
            ImGui::DockBuilderDockWindow(dockSpaceWindow.window.name.c_str(), dockId);
        }
        ImGui::DockBuilderFinish(dockSpaceId);
        hasBuilt = true;
    }

    if (runWindows) {
        for (const auto& dockSpaceWindow : windows) {
            ImGuiHelper::BeginWindowWithEnd(dockSpaceWindow.window);
        }
    }
    ImGui::End();
}

//--- StaticPopupModalManager ---//
void ImGuiHelper::StaticPopupModalManager::QueueOpenPopop(PopupModal* popupModal) {
    if (!popupModal->hasRegistered) {
        popupModal->hasRegistered = true;
        framePopupModals.emplace_back(popupModal);
    }
    popupModalsToOpen.emplace_back(popupModal);
}

void ImGuiHelper::StaticPopupModalManager::Flush() {
    for (auto* popupModal : popupModalsToOpen) {
        ImGui::OpenPopup(popupModal->name);
    }
    popupModalsToOpen.clear();
    for (auto* popupModal : framePopupModals) {
        ImGuiHelper::BeginPopupModal(*popupModal);
    }
}
