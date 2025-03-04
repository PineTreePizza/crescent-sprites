#include "game_properties.h"

#include "../seika/src/utils/logger.h"
#include "../seika/src/memory/se_mem.h"
#include "../seika/src/utils/se_assert.h"

#include "scripting/python/cre_py.h"

static CREGameProperties* properties = NULL;

CREGameProperties* cre_game_props_create() {
    CREGameProperties* props = SE_MEM_ALLOCATE(CREGameProperties);
    props->gameTitle = NULL;
    props->windowWidth = 800;
    props->windowHeight = 600;
    props->resolutionWidth = props->windowWidth;
    props->resolutionHeight = props->windowHeight;
    props->targetFPS = 66;
    props->initialScenePath = NULL;
    props->areCollidersVisible = false;
    props->audioSourceCount = 0;
    props->textureCount = 0;
    props->fontCount = 0;
    props->inputActionCount = 0;
    return props;
}

void cre_game_props_initialize(CREGameProperties* initialProps) {
    properties = initialProps;
}

void cre_game_props_finalize() {
    if (properties != NULL) {
        SE_MEM_FREE(properties);
        properties = NULL;
    }
}

CREGameProperties* cre_game_props_get() {
    return properties;
}

CREGameProperties* cre_game_props_get_or_default() {
    if (properties == NULL) {
        properties = cre_game_props_create();
    }
    return properties;
}

void cre_game_props_print() {
    if (properties == NULL) {
        se_logger_error("No game properties set, not printing!");
        return;
    }
    se_logger_debug(
        "game properties:\n    game_title = %s\n    resolution_width = %d\n    resolution_height = %d\n    window_width = %d\n    window_height = %d\n    target_fps = %d",
        properties->gameTitle, properties->resolutionWidth, properties->resolutionHeight, properties->windowWidth,
        properties->windowHeight, properties->targetFPS);
}
