#include "node_component.h"

#include <string.h>

#include "../seika/src/memory/se_mem.h"

NodeComponent* node_component_create() {
    NodeComponent* nodeComponent = SE_MEM_ALLOCATE(NodeComponent);
    nodeComponent->name[0] = '\0';
    nodeComponent->type = NodeBaseType_INVALID;
    return nodeComponent;
}

void node_component_delete(NodeComponent* nodeComponent) {
    SE_MEM_FREE(nodeComponent);
}

NodeComponent* node_component_copy(const NodeComponent* nodeComponent) {
    NodeComponent* copiedNode = SE_MEM_ALLOCATE(NodeComponent);
    memcpy(copiedNode, nodeComponent, sizeof(NodeComponent));
    return copiedNode;
}

NodeBaseType node_get_base_type(const char* baseName) {
    if (strcmp(baseName, RBE_NODE_NODE_STRING) == 0) {
        return NodeBaseType_NODE;
    } else if (strcmp(baseName, RBE_NODE_NODE2D_STRING) == 0) {
        return NodeBaseType_NODE2D;
    } else if (strcmp(baseName, RBE_NODE_SPRITE_STRING) == 0) {
        return NodeBaseType_SPRITE;
    } else if (strcmp(baseName, RBE_NODE_ANIMATED_SPRITE_STRING) == 0) {
        return NodeBaseType_ANIMATED_SPRITE;
    } else if (strcmp(baseName, RBE_NODE_TEXT_LABEL_STRING) == 0) {
        return NodeBaseType_TEXT_LABEL;
    } else if (strcmp(baseName, RBE_NODE_COLLIDER2D_STRING) == 0) {
        return NodeBaseType_COLLIDER2D;
    } else if (strcmp(baseName, RBE_NODE_COLOR_RECT_STRING) == 0) {
        return NodeBaseType_COLOR_RECT;
    }
    return NodeBaseType_INVALID;
}

NodeBaseInheritanceType node_get_type_inheritance(NodeBaseType type) {
    switch (type) {
    case NodeBaseType_NODE:
        return NodeBaseInheritanceType_NODE;
    case NodeBaseType_NODE2D:
        return NodeBaseInheritanceType_NODE2D;
    case NodeBaseType_SPRITE:
        return NodeBaseInheritanceType_SPRITE;
    case NodeBaseType_ANIMATED_SPRITE:
        return NodeBaseInheritanceType_ANIMATED_SPRITE;
    case NodeBaseType_TEXT_LABEL:
        return NodeBaseInheritanceType_TEXT_LABEL;
    case NodeBaseType_COLLIDER2D:
        return NodeBaseInheritanceType_COLLIDER2D;
    case NodeBaseType_COLOR_RECT:
        return NodeBaseInheritanceType_COLOR_RECT;
    default:
        break;
    }
    return NodeBaseInheritanceType_INVALID;
}

const char* node_get_base_type_string(NodeBaseType type) {
    switch (type) {
    case NodeBaseType_NODE:
        return RBE_NODE_NODE_STRING;
    case NodeBaseType_NODE2D:
        return RBE_NODE_NODE2D_STRING;
    case NodeBaseType_SPRITE:
        return RBE_NODE_SPRITE_STRING;
    case NodeBaseType_ANIMATED_SPRITE:
        return RBE_NODE_ANIMATED_SPRITE_STRING;
    case NodeBaseType_TEXT_LABEL:
        return RBE_NODE_TEXT_LABEL_STRING;
    case NodeBaseType_COLLIDER2D:
        return RBE_NODE_COLLIDER2D_STRING;
    case NodeBaseType_COLOR_RECT:
        return RBE_NODE_COLOR_RECT_STRING;
    default:
        break;
    }
    return NULL;
}
