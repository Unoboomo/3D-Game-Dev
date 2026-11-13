#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_color.h"
#include "gfc_list.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"
#include "gfc_text.h"
#include "gfc_types.h"
#include "gfc_vector.h"

#include "model.h"

typedef enum { //entities of the same type do not collide with each other
	ETT_none = 0,
	ETT_player = 1 << 0,
	ETT_enemy = 1 << 1,
	ETT_item = 1 << 2,
	ETT_world = 1 << 3,
	ETT_MAX = 1 << 4,
}EntityTeamType;

//typedef enum {
//	ECL_none = 0,
//	ECL_World = 1,
//	ECL_Entity = 2,
//	ECL_Item = 4,
//	ECL_ALL = 7
//}EntityCollisionLayers;

typedef struct Entity_s
{
	Uint8					_inuse;				//memory management flag

	//Uint8					player;				//1 if player, 0 otherwise

	GFC_TextLine			name;								//name of the entity for debugging
	EntityTeamType			team;								//which team this entity is on


	Model *model;								//graphical representation of the entity

	GFC_Box			bounds;
	GFC_Vector3D	position;					//the position of the entity in space (the entity is centered on this point)
	GFC_Vector3D	velocity;					//how fast the entity is moving
	GFC_Vector3D	acceleration;				//how fast the velocity changes 

	float floorHeight;							//point directly below us
	Uint8 grounded;								//is the entity on the floor?

	GFC_Vector3D rotation;	//x - pitch (around sideways vector), y - roll (around forward vector), z - yaw (around up vector)
	GFC_Vector3D scale;

	GFC_Matrix4	mat;
	GFC_Color color_mod;


	
	int		(*think)(struct Entity_s* self);	//pointer to a think function
	int		(*update)(struct Entity_s* self);	//pointer to a update function (all entities can think based on the same gamestate, then update all)
	int		(*draw)(struct Entity_s* self);
	void	(*free)(struct Entity_s* self);		//pointer to a free function

	/**
	 * @brief function called when this entity touches another entity
	 * @param self: [in] the entity who owns the touch function
	 * @param other: [in] the entity that collided with self
	 * @param collision_bitmask: [in] the collision face relative to self 
	 * @param min_translation_vec: [in] the minimum translation needed to apply to self to make self and other not collide
	 */
	void	(*touch)(struct Entity_s* self, struct Entity_s* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);	

	void* data;								//the storage for unique entity type data, (if set, you must specify a free function)			
}Entity;


/**
* @brief initialize the entity manager
* @param max upper limit for how many entities can exist at once
*/
void entity_system_init(Uint32 max);

/**
* @breif allocates a new empty entity
* @return NULL on error, or a blank entity
*/
Entity* entity_new();

/**
* @brief free a previously created entity after it is no longer needed
* @param *self a pointer to the entity to free
*/
void entity_free(Entity* self);

/**
* @brief queue render an entity
* @param *self a pointer to the entity to queue render
*/
void entity_queue_render(Entity* self);

/**
* @brief free all entities in the manager
* @note does not close the entity manager
*/
void entity_system_free_all();

/**
* @brief Run all entity update functions
*/
void entity_system_update_all();

/**
* @brief Run all entity think functions
*/
void entity_system_think_all();

/**
* @brief queue render all entities in the manager
*/
void entity_system_queue_render_all();

/**
 * @brief check if two entities are colliding
 * @param self: [in] the entity doing the checking, collision directions are with respect to this entity 
 * @param other: [in] the entity we check against
 * @param min_translation: [out] the minimum translation needed to apply to self to make self and other not collide
 * @return a bitmask encoding the direction collision occured
 * 
 * NOTE: this is post-move bounds checking
 * WARNING: can return BOX_COLLISION_ERROR, check for it
 */
Uint8 entity_collision_other(Entity* self, Entity* other, GFC_Vector3D *min_translation);

/**
 * @brief checks if an entity is outside the world bounds, given an offset from the entity's current position
 * @param self [in] the entity to check bounds against
 * @param offset [in] the offset from the current position you want to check against
 * @param min_translation_vec [in] the vector the aabb needs to change by to not be outside the level
 * @return a bitmask representing the directions the bounding box is outside the level
 * 
 * NOTE: this is pre-move bounds checking (offset != {0}) or post-move bounds checking (pass through min_translation_vec)
 * 
 * WARNING: can return LEVEL_BOUNDS_ERROR, check for it
 */
Uint8 entity_level_bounds_check(Entity* self, GFC_Vector3D offset, GFC_Vector3D* min_translation_vec);

/**
 * @brief if an entity is outside the world bounds, enforce world bounds, and reset velocities if they are pointed outside the world bounds
 * @param self [in] the entity to check bounds against
 * @return the collision bitmask if you want to know the axis of collision
 * 
 * NOTE: This is post-move bounds enforcement
 */
Uint8 entity_level_bounds_enforce(Entity* self);

/**
* @brief checks if the entity's team matches with another team
* @param *self a pointer to the entity to check the team of
* @param team the team to check against
* @return the matching bitmask (will be 0 if no overlap, a non-zero otherwise)
*/
Uint8 entity_check_team(Entity* self, EntityTeamType team);

/**
* @brief checks an entity's prospective position for collision against other entities,
* @param *self a pointer to the entity to check with
*/
void entity_collide_all(Entity* self);

/**
* @brief spawns an entity from def
* @param entityName: [in] the name of the entity in the def to spawn
* @param position: [in] where to spawn the entity
*/
Entity* entity_spawn_from_def(const char* entityName, GFC_Vector3D position);






/**
* @brief updates the position of an entity from its velocity and ensures it is still in world bounds
* @param *self a pointer to the entity to update the position of
*/
void entity_update_position(Entity* self);

/**
 * @brief
 * @param self: [in] the entity doing the checking, collision_bitmask directions are with respect to this entity
 * @param other: [in] the entity we check against
 * @param min_translation: [in] the minimum translation needed to apply to self to make self and other not collide
 * @param collision_bitmask: [in] a bitmask encoding the direction collision occured
 *
 * NOTE: this is post-move bounds checking
 * WARNING: can return BOX_COLLISION_ERROR, check for it
 */
//void entity_collide_with_world_entity_physics_resolve(Entity* self, Entity* other, Uint8 collision_bitmask, GFC_Vector3D min_translation_vec);

void entity_system_free_all_but_player();

/**
* @brief Draw all entities in a list
* @param entities a GFC_List of entity pointers
*/
void entity_system_draw_list();

/**
* @brief configure an entity based on provided name in a config def
* @param self the entity to config
* @param name the name of the entity
*/
void entity_configure_from_def(Entity* self, const char* name);

/**
* @brief get the bounds of an entity based on provided name in a config def
* @param name the name of the entity
* @return returns a Rect of the bounds of the entity
*/
//GFC_Rect entity_bounds_from_def(const char* name);

/**
* @brief configure an entity based on provided filename that contains a config
* @param self the entity to config
* @param filename the filename of the json providing entity information
*/
void entity_configure_from_file(Entity* self, const char* filename);

/**
* @brief configure an entity based on provided json config
* @param self the entity to config
* @param json the json providing entity information
*/
void entity_configure_from_json(Entity* self, SJson* json);

SJson* entity_save_all_to_config();





/**
* @brief checks the entity layers collide with the other layers
* @param *self a pointer to the entity to check the layers of
* @param layer the layers to check against
* @return the matching bitmask (will be 0 if no overlap, a non-zero otherwise)
*/
//Uint8 entity_check_layer(Entity* self, EntityCollisionLayers layer);

/**
* @brief updates an entity's layer bitmask with a new layer
* @param *self a pointer to the entity to update the layers of
* @param layer the layer(s) to add
*/
//void entity_set_collision_layer(Entity* self, EntityCollisionLayers layer);

/**
* @brief updates an entity's layer bitmask by removing a layer
* @param *self a pointer to the entity to update the layers of
* @param layer the layer(s) to remove
*/
//void entity_remove_collision_layer(Entity* self, EntityCollisionLayers layer);

/**
* @brief gets a list of all entitys with a specific entity_name
* @param the entity name to look for
* @return a List of pointers to entities
* @note the gfc_list returned must be cleaned up with gfc_list_delete
*/
GFC_List* entity_find_by_name(const char* entity_name);

/**
* @brief checks to see a rect collides with any entities
* @param bounds the rect being checked (in worldspace)
* @return 1 if colliding, 0 if not
*/
//Uint8 entity_test_collision_rect(GFC_Rect bounds);

Entity* entity_get_player();

#endif