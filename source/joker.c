#include <tonc.h>

#include "joker.h"
#include "joker_gfx.h"
#include "graphic_utils.h"
#include "card.h"
#include "soundbank.h"

#include <maxmod.h>
#include <stdlib.h>
#include <string.h>

#define JOKER_SCORE_TEXT_Y 48

const static u8 edition_price_lut[MAX_EDITIONS] =
{
    0, // BASE_EDITION
    2, // FOIL_EDITION
    3, // HOLO_EDITION
    5, // POLY_EDITION
    5, // NEGATIVE_EDITION
};

/* So for the card objects, I needed them to be properly sorted
   which is why they let you specify the layer index when creating a new card object.
   Since the cards would overlap a lot in your hand, If they weren't sorted properly, it would look like a mess.
   The joker objects are functionally identical to card objects, so they use the same logic.
   But I'm going to use a simpler approach for the joker objects
   since I'm lazy and sorting them wouldn't look good enough to warrant the effort.
*/
static bool used_layers[MAX_JOKER_OBJECTS] = {false}; // Track used layers for joker sprites
// TODO: Refactor sorting into SpriteObject?

void joker_init()
{
    GRIT_CPY(&pal_obj_mem[PAL_ROW_LEN * JOKER_PB], joker_gfxPal);
}

Joker *joker_new(u8 id)
{
    if (id >= get_joker_registry_size()) return NULL;

    Joker *joker = malloc(sizeof(Joker));
    const JokerInfo *jinfo = get_joker_registry_entry(id);

    joker->id = id;
    joker->modifier = BASE_EDITION; // TODO: Make this random later
    joker->value = jinfo->base_value + edition_price_lut[joker->modifier]; // Base value + edition price
    joker->rarity = jinfo->rarity;
    joker->processed = false;

    return joker;
}

void joker_destroy(Joker **joker)
{
    if (*joker == NULL) return;
    free(*joker);
    *joker = NULL;
}

JokerEffect joker_get_score_effect(Joker *joker, Card *scored_card)
{
    const JokerInfo *jinfo = get_joker_registry_entry(joker->id);
    if (!jinfo) return (JokerEffect){0};

    return jinfo->effect(joker, scored_card);
}

// JokerObject methods
JokerObject *joker_object_new(Joker *joker)
{
    JokerObject *joker_object = malloc(sizeof(JokerObject));

    int layer = 0;
    for (int i = 0; i < MAX_JOKER_OBJECTS; i++)
    {
        if (!used_layers[i])
        {
            layer = i;
            used_layers[i] = true; // Mark this layer as used
            break;
        }
    }

    joker_object->joker = joker;
    joker_object->sprite_object = sprite_object_new();

    int tile_index = JOKER_TID + (layer * JOKER_SPRITE_OFFSET);
    memcpy32(&tile_mem[4][tile_index], &joker_gfxTiles[joker->id * TILE_SIZE * JOKER_SPRITE_OFFSET], TILE_SIZE * JOKER_SPRITE_OFFSET);
    sprite_object_set_sprite
    (
        joker_object->sprite_object, 
        sprite_new
        (
            ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, 
            ATTR1_SIZE_32, 
            tile_index, 
            JOKER_PB,
            JOKER_STARTING_LAYER + layer
        )
    );

    

    return joker_object;
}

void joker_object_destroy(JokerObject **joker_object)
{
    if (*joker_object == NULL) return;

    int layer = sprite_get_layer(joker_object_get_sprite(*joker_object)) - JOKER_STARTING_LAYER;
    used_layers[layer] = false;
    sprite_object_destroy(&(*joker_object)->sprite_object); // Destroy the sprite
    joker_destroy(&(*joker_object)->joker); // Destroy the joker
    free(*joker_object);
    *joker_object = NULL;
}

void joker_object_update(JokerObject *joker_object)
{
    CardObject *card_object = (CardObject *)joker_object;
    card_object_update(card_object);
}

void joker_object_shake(JokerObject *joker_object, mm_word sound_id)
{
    sprite_object_shake(joker_object->sprite_object, sound_id);
}

bool joker_object_score(JokerObject *joker_object, Card* scored_card, int *chips, int *mult, int *xmult, int *money, bool *retrigger)
{
    if (joker_object->joker->processed == true) return false; // If the joker has already been processed, return false

    JokerEffect joker_effect = joker_get_score_effect(joker_object->joker, scored_card);

    if (memcmp(&joker_effect, &(JokerEffect){0}, sizeof(JokerEffect)) != 0)
    {
        *chips += joker_effect.chips;
        *mult += joker_effect.mult;
        // TODO: XMult
        *money += joker_effect.money;
        // TODO: Retrigger

        tte_set_pos(fx2int(joker_object->sprite_object->x) + 8, JOKER_SCORE_TEXT_Y); // Offset of 16 pixels to center the text on the card

        char score_buffer[12];

        if (joker_effect.chips > 0)
        {
            tte_set_special(0xD000); // Blue
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.chips);
        }
        else if (joker_effect.mult > 0)
        {
            tte_set_special(0xE000); // Red
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.mult);
        }
        else if (joker_effect.money > 0)
        {
            tte_set_special(0xC000); // Yellow
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.money);
        }

        tte_write(score_buffer);

        joker_object->joker->processed = true; // Mark the joker as processed
        joker_object_shake(joker_object, SFX_CARD_SELECT); // TODO: Add a sound effect for scoring the joker

        return true;
    }

    return false;
}


void joker_object_set_selected(JokerObject* joker_object, bool selected)
{
    if (joker_object == NULL)
        return;
    sprite_object_set_selected(joker_object->sprite_object, selected);
}

bool joker_object_is_selected(JokerObject* joker_object)
{
    if (joker_object == NULL)
        return false;
    return sprite_object_is_selected(joker_object->sprite_object);
}

Sprite* joker_object_get_sprite(JokerObject* joker_object)
{
    if (joker_object == NULL)
        return NULL;
    return sprite_object_get_sprite(joker_object->sprite_object);
}
