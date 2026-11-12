#include "simple_logger.h"

#include "gfc_config_def.h"

#include "player.h"
#include "item.h"

int item_think(Entity* self);

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
	self->think = item_think;
	self->bounds = gfc_box_from_config(sj_object_get_value(itemDef, "bounds"));
	self->position = position;
	return self;
}

int item_think(Entity* self)
{
	if (!self)
	{
		return;
	}
	if (entity_collision_other(self, player_get_the(), NULL))
	{
		//will eventually go in touch function
		slog("give player item");
		entity_free(self);
		return 1;
	}
	return 0;
}