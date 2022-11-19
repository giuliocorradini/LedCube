#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

enum Axis_t {
    X_AXIS, Y_AXIS, Z_AXIS
};

enum ShiftDirection_t {
    POS_X, NEG_X, POS_Z, NEG_Z, POS_Y, NEG_Y
};

enum Effect_t {
    RAIN, PLANE_BOING, SEND_VOXELS, WOOP_WOOP, CUBE_JUMP, GLOW, TEXT, LIT
};

struct Cube_t {
    uint8_t regs[5][5];
    enum Effect_t current_effect;
};

#ifdef __cplusplus
}
#endif