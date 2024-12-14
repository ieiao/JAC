#ifndef __SIMPLE_THEME_H__
#define __SIMPLE_THEME_H__

#include <stdint.h>
#include "jac.h"

enum {
    ELEMENT_TYPE_NONE = 0,
    ELEMENT_TYPE_STRING,
    ELEMENT_TYPE_BITMAP,
    ELEMENT_TYPE_ACTION,
};

/* Only used with ELEMENT_TYPE_STRING. */
enum {
    ALIGN_NONE = 0,
    HORI_ALIGN_LEFT,        /* Not Implemented */
    HORI_ALIGN_CENTER,
    HORI_ALIGN_RIGHT,       /* Not Implemented */
    VERT_ALIGN_TOP,         /* Not Implemented */
    VERT_ALIGN_CENTER,      /* Not Implemented */
    VERT_ALIGN_BOTTOM,      /* Not Implemented */
    DISPLAY_2X,
};

enum {
    ACTION_NONE = 0,
    ACTION_CLEAR,
    ACTION_DRAW_COLOR_WHITE,
    ACTION_DRAW_COLOR_BLACK,
    ACTION_REFRESH_DEFAULT,
    ACTION_REFRESH_QUICK,
    ACTION_REFRESH_SMART,
    ACTION_REFRESH_PART,
    ACTION_REFRESH_MODE1,
    ACTION_REFRESH_MODE2,
    ACTION_REFRESH_MODE3,
};

struct element {
    uint8_t     type;
    uint8_t     param;
    uint16_t    x;
    uint16_t    y;
    uint16_t    w;
    uint16_t    h;
    uint8_t     *ptr;
    void        (*fill)(char *p, void *d);
    const uint8_t **font;
};

struct simple_theme {
    /*
     * Used for dispaly primary info and state, must impelment.
     */
    struct element const *primary;

    /*
     * Used when battery is dead.
     */
    struct element const *battery_dead;

    /*
     * Used when into powersaving mode.
     */
    struct element const *powersaving;

    /*
     * Used waitting user device connect in config mode.
     */
    struct element const *config_wait;

    /*
     * Used when device not setting done.
     */
    struct element const *welcome;

    /*
     * Used when resources initialization failed.
     */
    struct element const *core_failure;
};

void dispart(struct element const *p, struct jac_info *info);

#endif
