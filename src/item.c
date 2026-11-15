#include "simple_logger.h"

#include "gfc_config_def.h"

#include "player.h"
#include "item.h"

void item_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);
void item_jump_refresh_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);
void item_dash_refresh_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);

Entity* spawn_item_from_def(const char* itemName, GFC_Vector3D position)
{
	const char* str;
	SJson* itemDef;
	Entity* self;
	if (!itemName)
	{
		return NULL;
	}
	itemDef = gfc_config_def_get_by_name("items", itemName);
	if (!itemDef)
	{
		slog("cannot load item with name %s", itemName);
		return NULL;
	}
	self = entity_new();
	if (!self)
	{
		slog("failed to create entity for item %s", itemName);
		return NULL;
	}
	str = sj_object_get_string(itemDef, "model");
	if (str)
	{
		self->model = model_load(str);
		if (!self->model)
		{
			slog("failed to load model %s", str);
			entity_free(self);
			return NULL;
		}
	}
	str = sj_object_get_string(itemDef, "name");
	if (str) {
		gfc_line_cpy(self->name, str);
	}
	self->bounds = gfc_box_from_config(sj_object_get_value(itemDef, "bounds"));
	self->position = position;
	self->team = ETT_item;
	self->touch = item_get_touch_function_by_name(self->name);
	return self;
}

void* item_get_touch_function_by_name(GFC_TextLine name)
{
	if (!name)
	{
		slog("cannot find a touch function for an item with no name");
		return NULL;
	}
	if (gfc_strlcmp(name, "Steve") == 0)
	{
		return item_touch;
	}
	if (gfc_strlcmp(name, "Jump Refresh") == 0)
	{
		return item_jump_refresh_touch;
	}
	if (gfc_strlcmp(name, "Dash Refresh") == 0)
	{
		return item_dash_refresh_touch;
	}
	else
	{
		slog("touch function for item %s not found", name);
		return NULL;
	}
	
}
void item_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
{
	if (!self || !other)
	{
		return;
	}

	if (entity_check_team(other, ETT_player))
	{
		slog("give player item %s", self->name);
		entity_free(self);
		return;
	}
}

void item_jump_refresh_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
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
		slog("give player item %s", self->name);

		data = (PlayerEntityData*)other->data;
		data->jumps_remaining++;
		entity_free(self);
		return;
	}
}

void item_dash_refresh_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec)
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
		slog("give player item %s", self->name);

		data = (PlayerEntityData*)other->data;
		data->dashes_remaining++;
		entity_free(self);
		return;
	}
}