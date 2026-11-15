#include "simple_logger.h"

#include "gfc_config_def.h"

#include "utils.h"
#include "player.h"
#include "item.h"

#include "world.h"

void world_bounce_pad_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);
void world_reduced_gravity_field_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);
void world_breakable_wall_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);


void* world_get_touch_function_by_name(GFC_TextLine name)
{
	if (!name)
	{
		slog("cannot find a touch function for an world_entity with no name");
		return NULL;
	}
	if (gfc_strlcmp(name, "Bounce Pad") == 0)
	{
		return world_bounce_pad_touch;
	}
	if (gfc_strlcmp(name, "Reduced Gravity Field") == 0)
	{
		return world_reduced_gravity_field_touch;
	}
	if (gfc_strlcmp(name, "Breakable Wall") == 0)
	{
		return world_breakable_wall_touch;
	}
	else
	{
		slog("touch function for world_entity %s not found", name);
		return NULL;
	}
}

void world_bounce_pad_touch(Entity * self, Entity * other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
{
	if (!self || !other)
	{
		return;
	}

	if (entity_check_team(other, ETT_player))
	{
		slog("bounce that boi");
		other->velocity.z = BOUNCE_PAD_VELOCITY;
		other->grounded = 0;
		return;
	}
}

void world_reduced_gravity_field_touch(Entity * self, Entity * other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
{
	PlayerEntityData* data;
	if (!self || !other)
	{
		return;
	}

	if (entity_check_team(other, ETT_player))
	{
		if (!other->data)
		{
			return;
		}

		data = (PlayerEntityData*)other->data;

		data->reduced_gravity = 1;
		return;
	}
}

void world_breakable_wall_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
{
	PlayerEntityData* data;
	Uint8 horizontal_mask;
	if (!self || !other)
	{
		return;
	}

	if (entity_check_team(other, ETT_player))
	{
		if (!other->data)
		{
			return;
		}
		horizontal_mask = BOX_COLLISION_POS_X | BOX_COLLISION_NEG_X | BOX_COLLISION_POS_Y | BOX_COLLISION_NEG_Y;
		if (!(collision_bitmask & horizontal_mask))
		{
			return;
		}

		data = (PlayerEntityData*)other->data;

		if (data->movement_state != PMS_Sliding)
		{
			return;
		}
		slog("break that wall");
		entity_free(self);

		return;
	}
}
