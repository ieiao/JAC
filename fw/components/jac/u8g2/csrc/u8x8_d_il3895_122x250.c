/*

  u8x8_d_HINK_E0213A04.c


  IL3895: 122x250
  command 
    0x22: assign actions
    0x20: execute actions
  
  action for command 0x022 are (more or less guessed)
    bit 7:	Enable Clock
    bit 6:	Enable Charge Pump
    bit 5:	Load Temparture Value (???)
    bit 4:	Load LUT (???)
    bit 3:	Initial Display (???)
    bit 2:	Pattern Display --> Requires about 945ms with the LUT from below
    bit 1:	Disable Charge Pump
    bit 0:	Disable Clock

    Disable Charge Pump and Clock require about 267ms
    Enable Charge Pump and Clock require about 10ms

  Notes:
    - Introduced a refresh display message, which copies RAM to display
    - Charge pump and clock are only enabled for the transfer RAM to display
    - U8x8 will not really work because of the two buffers in the SSD1606, however U8g2 should be ok.

*/


#include "u8x8.h"
#include "simple_theme.h"

/* HINK-E0213A04 2.13" EPD */
static const uint8_t u8x8_d_il3895_122x250_HINK_E0213A04_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_CAA(0x01, 0xf9, 0x00),
    U8X8_CA(0x3a, 0x06),
    U8X8_CA(0x3b, 0x0b),
    U8X8_CA(0x11, 0x03),
    U8X8_CAA(0x44, 0x00, 0x0f),
    U8X8_CAA(0x45, 0x00, 0xf9),
    U8X8_CA(0x2c, 0x19),
    U8X8_CA(0x3c, 0x03),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_full_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x22), U8X8_A(0x55), U8X8_A(0xaa),
    U8X8_A(0x55), U8X8_A(0xaa), U8X8_A(0x55),
    U8X8_A(0xaa), U8X8_A(0x11), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x10),
    U8X8_A(0x1e), U8X8_A(0x10), U8X8_A(0x1e),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x1e),
    U8X8_A(0x01), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_quick_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x22), U8X8_A(0x55), U8X8_A(0xaa),
    U8X8_A(0x55), U8X8_A(0xaa), U8X8_A(0x55),
    U8X8_A(0xaa), U8X8_A(0x11), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x10),
    U8X8_A(0x1e), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x1e),
    U8X8_A(0x01), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_part_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x88), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x4f), U8X8_A(0x01),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_mode1_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x11), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x12), U8X8_A(0x01),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_mode2_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x11), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x08), U8X8_A(0x01),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_refresh_mode3_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x32),
    U8X8_A(0x11), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x02), U8X8_A(0x01),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),
    U8X8_A(0x00), U8X8_A(0x00),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_powersave0_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_CA(0x22, 0xc0),
    U8X8_C(0x20),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_powersave1_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_CA(0x22, 0x03),
    U8X8_C(0x20),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_deepsleep_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_CA(0x10, 0x01),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static const uint8_t u8x8_d_il3895_to_display_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_CA(0x22, 0x04),
    U8X8_C(0x20),
    U8X8_END_TRANSFER(),
    U8X8_END()
};

static void __set_window(u8x8_t *u8x8, uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye)
{
    u8x8_cad_SendCmd(u8x8, 0x44);
    u8x8_cad_SendArg(u8x8, xs);
    u8x8_cad_SendArg(u8x8, xe);

    u8x8_cad_SendCmd(u8x8, 0x45);
    u8x8_cad_SendArg(u8x8, ys);
    u8x8_cad_SendArg(u8x8, ye);

    u8x8_cad_SendCmd(u8x8, 0x4e);
    u8x8_cad_SendArg(u8x8, xs);

    u8x8_cad_SendCmd(u8x8, 0x4f);
    u8x8_cad_SendArg(u8x8, ys);
}

static void u8x8_d_il3895_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr)
{
    uint16_t xs, xe, ys, ye, c, i;
    uint8_t *ptr;

    u8x8_cad_StartTransfer(u8x8);

    ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    xs = ((u8x8_tile_t *)arg_ptr)->x_pos;
    xe = xs + (c * arg_int) - 1;
    ys = ((u8x8_tile_t *)arg_ptr)->y_pos * 8;
    ye = ys + 7;
    if (ye > 249)
        ye = 249;

    __set_window(u8x8, xs, xe, ys, ye);
    u8x8_cad_SendCmd(u8x8, 0x24);
    for (i = 0; i <= (ye&0x7); i++)
        u8x8_cad_SendData(u8x8, c,
            ptr + u8x8->display_info->tile_width * i);

    u8x8_cad_EndTransfer(u8x8);
}

static const u8x8_display_info_t u8x8_il3895_122x250_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,

  /* post_chip_enable_wait_ns = */ 120,
  /* pre_chip_disable_wait_ns = */ 60,
  /* reset_pulse_width_ms = */ 20,
  /* post_reset_wait_ms = */ 10,
  /* sda_setup_time_ns = */ 50,
  /* sck_pulse_width_ns = */ 100,
  /* sck_clock_hz = */ 4000000UL,
  /* spi_mode = */ 0,
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,
  /* tile_width = */ 16,	/* 16*8 = 128 */
  /* tile_hight = */ 32,	/* 32*8 = 256 */
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 122,
  /* pixel_height = */ 250
};

uint8_t u8x8_d_il3895_122x250(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t refresh_mode = ACTION_REFRESH_DEFAULT;
    switch(msg)
    {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
        u8x8_d_helper_display_setup_memory(u8x8, &u8x8_il3895_122x250_display_info);
        break;

    case U8X8_MSG_DISPLAY_INIT:
        u8x8_d_helper_display_init(u8x8);
        u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_122x250_HINK_E0213A04_init_seq);
        u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_full_init_seq);
        u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_powersave0_seq);
        break;

    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
        if (arg_int == 0)
            u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_powersave0_seq);
        else if (arg_int == 1)
            u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_powersave1_seq);
        else if (arg_int == 2)
            u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_deepsleep_seq);
        break;

    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
        break;

    case U8X8_MSG_DISPLAY_DRAW_TILE:
        u8x8_d_il3895_draw_tile(u8x8, arg_int, arg_ptr);
        break;

    case U8X8_MSG_DISPLAY_REFRESH:
        if (arg_int != refresh_mode) {
            if (arg_int == ACTION_REFRESH_QUICK)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_quick_init_seq);
            else if (arg_int == ACTION_REFRESH_DEFAULT)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_full_init_seq);
            else if (arg_int == ACTION_REFRESH_PART)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_part_init_seq);
            else if (arg_int == ACTION_REFRESH_MODE1)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_mode1_init_seq);
            else if (arg_int == ACTION_REFRESH_MODE2)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_mode2_init_seq);
            else if (arg_int == ACTION_REFRESH_MODE3)
                u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_refresh_mode3_init_seq);
            refresh_mode = arg_int;
        }
        u8x8_cad_SendSequence(u8x8, u8x8_d_il3895_to_display_seq);
        break;

    default:
        return 0;
    }

    return 1;
}

