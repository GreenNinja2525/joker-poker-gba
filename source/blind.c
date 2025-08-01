#include <tonc.h>

#include "blind.h"
#include "blinds_gfx.h"
#include "graphic_utils.h"

// Palettes for the blinds (Transparency, Text Color, Shadow, Highlight, Main Color) Use this: http://www.budmelvin.com/dev/15bitconverter.html
static const u16 small_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x7FFF, 0x34A1, 0x5DCB, 0x5104, 0x55A0, 0x2D01, 0x34E0};
static const u16 big_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x2527, 0x15F5, 0x36FC, 0x1E9C, 0x01B4, 0x0D0A, 0x010E};
static const u16 boss_blind_token_palette[PAL_ROW_LEN] = {0x0000, 0x2CC9, 0x3D0D, 0x5E14, 0x5171, 0x4D0F, 0x2CC8, 0x3089}; // This variable is temporary, each boss blind will have its own unique palette

void blind_init()
{
    // Blind graphics (fighting grit every step of the way as usual)
    GRIT_CPY(&tile_mem[4][SMALL_BLIND_TID], blinds_gfxTiles);

    memcpy16(&pal_obj_mem[PAL_ROW_LEN * SMALL_BLIND_PB], &small_blind_token_palette, sizeof(small_blind_token_palette) / 2);
    memcpy16(&pal_obj_mem[PAL_ROW_LEN * BIG_BLIND_PB], &big_blind_token_palette, sizeof(big_blind_token_palette) / 2);
    // Boss Blind (This is temporary. Each boss blind is unique and will have to have its own graphics and palette which will probably be stored in some huge array)
    memcpy16(&pal_obj_mem[PAL_ROW_LEN * BOSS_BLIND_PB], &boss_blind_token_palette, sizeof(boss_blind_token_palette) / 2);
}

int blind_get_requirement(int type, int ante)
{
    if (ante < 0 || ante > MAX_ANTE)
    {
        ante = 0; // Ensure ante is within valid range
    }

    switch (type)
    {
        case SMALL_BLIND:
            return ante_lut[ante];
        case BIG_BLIND:
            return (ante_lut[ante] * 3) / 2; // X1.5
        case BOSS_BLIND:
            return ante_lut[ante] * 2; // X2
        default:
            return 0; // Invalid type
    }
}

int blind_get_reward(int type)
{
    switch (type)
    {
        case SMALL_BLIND:
            return 3;
        case BIG_BLIND:
            return 4;
        case BOSS_BLIND:
            return 5;
        default:
            return 0; // Invalid type
    }
}

u16 blind_get_color(int type, int index)
{
    switch (type)
    {
        case SMALL_BLIND:
            return small_blind_token_palette[index];
        case BIG_BLIND:
            return big_blind_token_palette[index];
        case BOSS_BLIND:
            return boss_blind_token_palette[index];
        default:
            return 0; // Invalid type
    }
}

Sprite *blind_token_new(int type, int x, int y, int sprite_index)
{
    Sprite *sprite = NULL;

    switch (type)
    {
        case SMALL_BLIND:
            sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP, ATTR1_SIZE_32x32, SMALL_BLIND_TID, SMALL_BLIND_PB, sprite_index);
            break;
        case BIG_BLIND:
            sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP, ATTR1_SIZE_32x32, BIG_BLIND_TID, BIG_BLIND_PB, sprite_index);
            break;
        case BOSS_BLIND:
            sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP, ATTR1_SIZE_32x32, BOSS_BLIND_TID, BOSS_BLIND_PB, sprite_index);
            break;
        default:
            return NULL;
    }

    sprite_position(sprite, x, y);

    return sprite;
}
