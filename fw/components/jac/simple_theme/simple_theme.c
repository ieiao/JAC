#include <string.h>

#include "simple_theme.h"
#include "display.h"
#include "u8g2.h"

static char common_buffer[64];
void dispart(struct element const *p, struct jac_info *info)
{
    unsigned int i = 0;
    u8g2_t *u8g2 = info->u8g2;

    while (p[i].type) {
        const uint8_t *old_font = NULL;
        struct element const *ele = &p[i++];
        char *ptr;
        uint16_t width;
        uint16_t x, y;

        if (ele->ptr)
            ptr = (char *)ele->ptr;
        else
            ptr = common_buffer;

        if (ele->fill) {
            if (ptr == common_buffer)
                memset(common_buffer, 0, sizeof(common_buffer));
            ele->fill(ptr, info);
        }

        switch (ele->type) {
        case ELEMENT_TYPE_ACTION: {
            if (ele->param >= ACTION_REFRESH_DEFAULT) {
                if (ele->param == ACTION_REFRESH_SMART) {
                    display_refresh(info->u8g2,
                            ((info->dt.minutes % 5) == 0) ? \
                            ACTION_REFRESH_DEFAULT : \
                            ACTION_REFRESH_QUICK);
                } else {
                    display_refresh(info->u8g2, ele->param);
                }
            } else if (ele->param == ACTION_CLEAR) {
                u8g2_ClearBufferV(u8g2, 0xff);
            } else if (ele->param == ACTION_DRAW_COLOR_WHITE) {
                u8g2_SetDrawColor(u8g2, 1);
            } else if (ele->param == ACTION_DRAW_COLOR_BLACK) {
                u8g2_SetDrawColor(u8g2, 0);
            }
            break;
        }

        case ELEMENT_TYPE_STRING_2X:
        case ELEMENT_TYPE_STRING: {
                if (ele->font) {
                    old_font = u8g2->font;
                    u8g2_SetFont(u8g2, (const uint8_t *)*ele->font);
                }

                x = ele->x;
                y = ele->y;

                switch (ele->param) {
                case HORI_ALIGN_CENTER:
                    width = u8g2_GetUTF8Width(u8g2, ptr);
                    if (ele->type == ELEMENT_TYPE_STRING_2X)
                        width <<= 1;
                    if (ele->w > width)
                        x += (ele->w - width) / 2;
                    break;

                case HORI_ALIGN_RIGHT:
                    width = u8g2_GetUTF8Width(u8g2, ptr);
                    if (ele->type == ELEMENT_TYPE_STRING_2X)
                        width <<= 1;
                    if (ele->w > width)
                        x += ele->w - width;
                    break;

                default:
                    break;
                };

                if (ele->type == ELEMENT_TYPE_STRING_2X)
                    u8g2_DrawUTF8X2(u8g2, x, y, ptr);
                else
                    u8g2_DrawUTF8(u8g2, x, y, ptr);

                if (old_font)
                    u8g2_SetFont(u8g2, old_font);
            break;
        }

        case ELEMENT_TYPE_BITMAP: {
            if (ele->param) {
                /* Data needs cast type conversion. */
                u8g2_DrawXBM(u8g2, ele->x, ele->y,
                             ele->w, ele->h, *((uint8_t **)ele->ptr));
            } else
                u8g2_DrawXBM(u8g2, ele->x, ele->y,
                             ele->w, ele->h, ele->ptr);
            break;
        }
        default:
            break;
        }

    }
}


