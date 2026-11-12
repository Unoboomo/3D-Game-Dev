#include "simple_logger.h"

#include "utils.h"
#include "level.h"

#include "entity.h"

extern int __DEBUG;

typedef struct
{
	Uint32 entity_count;
	Entity* entity_list;
}EntitySystem;

static EntitySystem entity_system = { 0 };

void entity_system_close();

Entity* entity_initialize(Uint32 entity_system_index);

void entity_update(Entity* self);

void entity_think(Entity* self);

void entity_system_init(Uint32 max_entities)
{
	if (entity_system.entity_count != 0) {
		slog("cannot initialize entity system, already initialized");
		return;
	}
	if (!max_entities)
	{
		slog("cannot initialize entity system for Zero entities");
		return;
	}
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), max_entities);
	if (!entity_system.entity_list) {
		slog("failed to allocate global entity list for %i entities", max_entities);
		return;
	}
	entity_system.entity_count = max_entities;

	if (__DEBUG)slog("entity system initialized");
	atexit(entity_system_close);
}

void entity_system_close()
{
	entity_system_free_all();
	if (entity_system.entity_list) {
		free(entity_system.entity_list);
		entity_system.entity_list = NULL;
	}
	memset(&entity_system, 0, sizeof(EntitySystem));
	if (__DEBUG)slog("entity system closed");
}

Entity* entity_new()
{
	int i;
	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			return entity_initialize(i);
		}
	}
	slog("entity_new: no free slots for new entities");
	return NULL;
}

Entity* entity_initialize(Uint32 entity_system_index)
{
	//boiler plate initilization goes here
	entity_system.entity_list[entity_system_index]._inuse = 1;
	entity_system.entity_list[entity_system_index].scale = gfc_vector3d(1, 1, 1);
	entity_system.entity_list[entity_system_index].color_mod = GFC_COLOR_WHITE;

	return &entity_system.entity_list[entity_system_index];
}

void entity_load(const char* filename);

void entity_free(Entity* self)
{
	if (!self)
	{
		slog("cannot free an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot free an entity that is not in use");
		return;
	}

	if (self->free)
	{
		self->free(self);
	}
	if (self->model)
	{
		model_free(self->model);
	}
	memset(self, 0, sizeof(Entity));
}

void entity_update(Entity* self)
{
	if (!self)
	{
		slog("cannot update an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot update an entity that is not in use");
		return;
	}
	if (self->update)
	{
		if (self->update(self))
		{
			return;
		}
	}
	//generic updating goes here
}

void entity_think(Entity* self)
{
	if (!self)
	{
		slog("cannot think an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot think an entity that is not in use");
		return;
	}
	if (self->think)
	{
		if (self->think(self))
		{
			return;
		}
	}
	//generic thinking goes here
}

void entity_queue_render(Entity* self)
{
	if (!self)
	{
		slog("cannot update an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot update an entity that is not in use");
		return;
	}
	if (self->draw)
	{
		if (self->draw(self)) //if we want to draw using our own draw, return 1
		{
			return;
		}
	}

	gfc_matrix4_from_vectors(self->mat, self->position, self->rotation, self->scale);
	model_queue_render(self->model, self->mat, self->color_mod);
}

void entity_system_free_all() 
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot free entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (entity_system.entity_list[i]._inuse)
		{
			entity_free(&entity_system.entity_list[i]);
		}
	}
}

void entity_system_update_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot update entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_update(&entity_system.entity_list[i]);
	}
}

void entity_system_think_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot think entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_think(&entity_system.entity_list[i]);
	}
}

void entity_system_queue_render_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot render entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_queue_render(&entity_system.entity_list[i]);
	}
}

Uint8 entity_collision_other(Entity* self, Entity* other, GFC_Vector3D *min_translation)
{
	//	BOX_COLLISION_NONE = 0,
	//	BOX_COLLISION_POS_X = 1 << 0,
	//	BOX_COLLISION_POS_Y = 1 << 1,
	//	BOX_COLLISION_POS_Z = 1 << 2,
	//	BOX_COLLISION_NEG_X = 1 << 3,
	//	BOX_COLLISION_NEG_Y = 1 << 4,
	//	BOX_COLLISION_NEG_Z = 1 << 5,
	//	BOX_COLLISION_ERROR = 1 << 6,

	GFC_Box a, b;

	if ((!self) || (!other))
	{
		return BOX_COLLISION_ERROR;
	}
	a = self->bounds;
	b = other->bounds;
	gfc_vector3d_add(a, a, self->position);
	gfc_vector3d_add(b, b, other->position);
	return utils_box_overlap(a, b, min_translation);
}

Uint8 entity_level_bounds_check(Entity* self, GFC_Vector3D offset, GFC_Vector3D* min_translation_vec)
{
	//	LEVEL_BOUNDS_NONE = 0,
	//	LEVEL_BOUNDS_POS_X = 1 << 0,
	//	LEVEL_BOUNDS_POS_Y = 1 << 1,
	//	LEVEL_BOUNDS_POS_Z = 1 << 2,
	//	LEVEL_BOUNDS_NEG_X = 1 << 3,
	//	LEVEL_BOUNDS_NEG_Y = 1 << 4,
	//	LEVEL_BOUNDS_NEG_Z = 1 << 5,
	//	LEVEL_BOUNDS_ERROR = 1 << 6,

	GFC_Box aabb;
	if (!self)
	{
		return LEVEL_BOUNDS_ERROR;
	}

	aabb = self->bounds;
	gfc_vector3d_add(aabb, self->bounds, self->position);
	gfc_vector3d_add(aabb, aabb, offset);
	return level_bounds_check(level_get_the(), aabb, min_translation_vec);
}

Uint8 entity_level_bounds_enforce(Entity* self)
{
	GFC_Vector3D temp = { 0 };
	Uint8 bitmask;

	bitmask = entity_level_bounds_check(self, gfc_vector3d(0, 0, 0), &temp);

	if (bitmask && !(bitmask & LEVEL_BOUNDS_ERROR))
	{
		if ((bitmask & LEVEL_BOUNDS_POS_X) && (self->velocity.x > 0))
		{
			self->velocity.x = 0;
		}
		else if ((bitmask & LEVEL_BOUNDS_NEG_X) && (self->velocity.x < 0))
		{
			self->velocity.x = 0;
		}
		if ((bitmask & LEVEL_BOUNDS_POS_Y) && (self->velocity.y > 0))
		{
			self->velocity.y = 0;
		}
		else if ((bitmask & LEVEL_BOUNDS_NEG_Y) && (self->velocity.y < 0))
		{
			self->velocity.y = 0;
		}
		if ((bitmask & LEVEL_BOUNDS_POS_Z) && (self->velocity.z > 0))
		{
			self->velocity.z = 0;
		}
		else if ((bitmask & LEVEL_BOUNDS_NEG_Z) && (self->velocity.z < 0))
		{
			self->velocity.z = 0;
		}
		gfc_vector3d_add(self->position, self->position, temp);
	}

	return bitmask;
}

Uint8 entity_check_team(Entity* self, EntityTeamType team) {
	if (!self) {
		return ETT_none;
	}
	return self->team & team;
}

void entity_collide_all(Entity* self)
{
	int i;
	Entity* other;
	GFC_Vector3D min_translation, other_min_translation;
	Uint8 bitmask, other_bitmask;
	if (!self || !self->_inuse)
	{
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{


		other = &entity_system.entity_list[i];
		if (!other->_inuse || self == other)
		{
			continue;
		}


		//if on the same team, dont collide (for now)
		if (entity_check_team(self, other->team))
		{ 
			continue;
		}

		//if (entity_check_layer()) {}

		//do collision check
		bitmask = entity_collision_other(self, other, &min_translation);
		if (bitmask && !(bitmask & BOX_COLLISION_ERROR)) //we had a collision;
		{

			//its a world collision, treat it as such
			if (entity_check_team(other, ETT_world))
			{
				//first, move self out of other entity
				gfc_vector3d_add(self->position, self->position, min_translation);

				//we collided in the x direction
				if ((bitmask & BOX_COLLISION_POS_X) && self->velocity.x > 0)
				{
					self->velocity.x = 0;
					self->acceleration.x = 0;

					//wall run shennanigans
				}
				else if ((bitmask & BOX_COLLISION_NEG_X) && self->velocity.x < 0)
				{
					self->velocity.x = 0;
					self->acceleration.x = 0;

					//wall run shennaniganss
				}
				if ((bitmask & BOX_COLLISION_POS_Y) && self->velocity.y > 0)
				{
					self->velocity.y = 0;
					self->acceleration.y = 0;

					//wall run shennanigans
				}
				else if ((bitmask & BOX_COLLISION_NEG_Y) && self->velocity.y < 0)
				{
					self->velocity.y = 0;
					self->acceleration.y = 0;

					//wall run shennaniganss
				}
				if ((bitmask & BOX_COLLISION_POS_Z) && self->velocity.z > 0) // we hit the top of our head
				{
					self->velocity.z = 0;
					self->acceleration.z = 0;
					self->grounded = 0;
				}
				if ((bitmask & BOX_COLLISION_NEG_Z) && self->velocity.z <= 0) // we hit our feet
				{
					self->velocity.z = 0;
					self->acceleration.z = 0;
					self->grounded = 1;
				}
			}
			if (self->touch)
			{
				self->touch(self, other, bitmask, min_translation);
			}
			if (!self->_inuse)
			{
				return;
			}

			if (other->_inuse && other->touch)
			{
				other_bitmask = utils_invert_bitmask(bitmask);
				gfc_vector3d_negate(other_min_translation, min_translation);
				other->touch(other, self, other_bitmask, other_min_translation);
			}
		}
		else if (bitmask & BOX_COLLISION_ERROR)
		{
			slog("collision error between %s and %s", self->name, other->name);
		}
	}
}
