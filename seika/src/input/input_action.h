#pragma once

#include <stddef.h>
#include <stdbool.h>

#define MAX_INPUT_ACTIONS 32
#define MAX_INPUT_VALUES 4

typedef enum InputActionType {
    InputActionType_KEYBOARD,
    InputActionType_MOUSE,
    InputActionType_GAMEPAD
} InputActionType;

typedef struct InputAction {
    size_t keyboardValueCount;
    size_t mouseValueCount;
    size_t gamepadValueCount;
    int keyboardValues[MAX_INPUT_VALUES];
    char* mouseValues[MAX_INPUT_VALUES];
    char* gamepadValues[MAX_INPUT_VALUES];
    int lastScancodePressed;
    bool isActionPressed;
    bool isActionJustPressed;
    bool isActionJustReleased;
    int deviceId;
} InputAction;

InputAction* se_input_action_create_new_input_action(const char* actionName, int deviceId);
