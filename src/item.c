#include "simple_logger.h"

#include "gfc_config_def.h"

#include "player.h"
#include "item.h"

void item_touch(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);

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
	self->touch = item_touch;
	return self;
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
		return 1;
	}
}
