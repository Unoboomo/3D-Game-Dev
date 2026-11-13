#include "simple_logger.h"

#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "mouse.h"
#include "level.h"
#include "time.h"
#include "utils.h"
#include "player.h"

int player_think(Entity* self);
int player_update(Entity* self);
void player_free(Entity* self);

void player_sync_camera(Entity* self);

static Entity* the_player = NULL;

Entity* player_get_the()
{
	return the_player;
}

Entity* player_new(GFC_Vector3D position) {
	Entity* self;
	PlayerEntityData* data;
	const char* str;
	if (the_player)
	{
		return the_player;
	}

	self = entity_new();
	if (!self) {
		slog("failed to create a new player entity");
		return NULL;
	}

	self->model = model_load("models/steve/steve.model");
	self->position = position;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;
	self->team = ETT_player;

	self->bounds = gfc_box(-2, -2, -0, 4, 4, 8);
	self->scale = gfc_vector3d(2, 2, 2);
	utils_scale_box(self->scale, &(self->bounds));

	data = gfc_allocate_array(sizeof(PlayerEntityData), 1);
	if (data)
	{
		data->cameraFollowDistance = 50;
		data->cameraRotation.x = 0;
		data->cameraRotation.y = GFC_PI/6;
		data->cameraFocusOffset.z = self->bounds.d * 0.6;

		data->max_air_jumps = PLAYER_MAX_AIR_JUMPS;
		data->jumps_remaining = PLAYER_MAX_AIR_JUMPS;
		data->jumped_this_frame = 0;
	}
	gfc_line_cpy(self->name, "Player");
	self->data = data;
	the_player = self;
	return self;
}

void player_free(Entity* self) {
	PlayerEntityData* data;
	if (!self || !self->data)
	{
		return;
	}

	data = (PlayerEntityData*)self->data;
	//clean up data

	//free the data
	free(data);
	self->data = NULL;
	the_player = NULL;
}

/**
 * @brief determines the player's desiered inputs
 * @param self: [in] the pointer to the player entity
 * @return 1 if the entity has completed all thinking, 0 if default thinking is also desired
 */
int player_think(Entity* self)
{
	GFC_Vector3D move = { 0 };
	PlayerEntityData* data;

	if (!self || !self->data)
	{
		return 1;
	}
	data = (PlayerEntityData*)self->data;
	
	//will eventually wrap this in if !grounded
	if (gfc_input_command_down("dash"))
	{
		time_set_time_scale(BULLET_TIME_TIME_SCALE);
	}
	else
	{
		time_set_time_scale(REAL_TIME_TIME_SCALE);
	}

	if (gfc_input_command_released("dash"))
	{
		data->dashed_this_frame = 1;
	}
	//end of wrap

	if (gfc_input_command_pressed("jump"))
	{
		data->jumped_this_frame = 1;
	}

	mouse_update_camera(&(data->cameraRotation));

	if (gfc_input_command_down("item_up"))
	{
		move.y += 1;
	}
	if (gfc_input_command_down("item_down"))
	{
		move.y += -1;
	}
	if (gfc_input_command_down("item_right"))
	{
		move.x += 1;
	}
	if (gfc_input_command_down("item_left"))
	{
		move.x += -1;
	}
	
	if (move.x || move.y)
	{
		gfc_vector3d_normalize(&move);

		gfc_vector3d_rotate_about_z(&move, data->cameraRotation.x);

		gfc_vector3d_copy(data->wish_direction, move);

	}
	else
	{
		gfc_vector3d_copy(data->wish_direction, gfc_vector3d(0,0,0));
	}
	//gf2d_mouse_get_movement for mouse control camera
	return 1;
}

int player_update(Entity* self)
{
	PlayerEntityData* data;
	Uint8 bitmask;
	GFC_Edge3D edge = { 0 };
	GFC_Vector3D offset;
	float ground_snap_offset;
	GFC_Vector3D floor;
	float scaled_delta_time;
	GFC_Vector3D scaled_physics;

	float friction;
	float friction_multiplier;
	float acceleration;
	float speed_soft_cap;
	float current_wish_direction_speed;
	float speed_difference;
	float acceleration_speed;

	if (!self || !self->data)
	{
		return 0;
	}
	data = (PlayerEntityData*)self->data;

	scaled_delta_time = time_get_scaled_delta();

	//Start Movement calculations:

	if (self->grounded)
	{
		speed_soft_cap = PLAYER_SPEED_SOFT_CAP_GROUND;
		friction = PLAYER_FRICTION_GROUND;
		acceleration = PLAYER_ACCELERATION_GROUND;
		data->jumps_remaining = data->max_air_jumps;
	}
	else

	{
		speed_soft_cap = PLAYER_SPEED_SOFT_CAP_AIR;
		friction = PLAYER_FRICTION_AIR;
		acceleration = PLAYER_ACCELERATION_AIR;
	}

	// more granular movement while slowed down to dodge and stuff
	if (gfc_input_command_down("dash"))
	{
		speed_soft_cap *= PLAYER_BULLET_TIME_SPEED_SCALE;
		acceleration *= PLAYER_BULLET_TIME_ACCELERATION_SCALE;
	}
	if (data->dashed_this_frame)
	{
		data->dashed_this_frame = 0;
		if (!gfc_vector3d_is_zero(data->wish_direction))
		{
			gfc_vector3d_copy(data->dash_direction, data->wish_direction);
		}
		else
		{
			data->dash_direction = gfc_vector3d(0, 1, 0);
			gfc_vector3d_rotate_about_z(&data->dash_direction, data->cameraRotation.x);
		}
		self->velocity.x = data->dash_direction.x * PLAYER_DASH_VELOCITY;
		self->velocity.y = data->dash_direction.y * PLAYER_DASH_VELOCITY;

		//then resent z velocity so we dont instantly crater
		if (!self->grounded && self->velocity.z < 0)
		{
			self->velocity.z = 0;
		}
	}
	//1. apply velocity dampening:
	friction_multiplier = 1 - (friction * scaled_delta_time);
	if (friction_multiplier < 0)
	{
		friction_multiplier = 0;
	}

	self->velocity.x *= friction_multiplier;
	self->velocity.y *= friction_multiplier;

	if (fabsf(self->velocity.x) < PLAYER_VELOCITY_EPSILON)
	{
		self->velocity.x = 0;
	}
	if (fabsf(self->velocity.y) < PLAYER_VELOCITY_EPSILON)
	{
		self->velocity.y = 0;
	}
	
	//2. apply acceleration for wish_direction
	
	if (!gfc_vector3d_is_zero(data->wish_direction))
	{
		current_wish_direction_speed = (data->wish_direction.x * self->velocity.x) + (data->wish_direction.y * self->velocity.y);
		speed_difference = speed_soft_cap - current_wish_direction_speed;
		if (speed_difference > 0)
		{
			acceleration_speed = acceleration * scaled_delta_time * speed_soft_cap;
			if (acceleration_speed > speed_difference)
			{
				acceleration_speed = speed_difference;
			}
			self->velocity.x += data->wish_direction.x * acceleration_speed;
			self->velocity.y += data->wish_direction.y * acceleration_speed;
		}
	}

	//3. apply gravity
	if (!self->grounded)
	{
		self->velocity.z -= PLAYER_GRAVITY * scaled_delta_time;
	}

	//3.5 jump
	if (data->jumped_this_frame)
	{
		data->jumped_this_frame = 0;
		if (self->grounded)
		{
			self->velocity.z = PLAYER_JUMP_VELOCITY;
			self->grounded = 0;
		}
		else if (data->jumps_remaining > 0)
		{
			self->velocity.z = PLAYER_JUMP_VELOCITY;
			self->grounded = 0;
			data->jumps_remaining--;
		}
	}

	//4. do movement
	gfc_vector3d_scale(scaled_physics, self->velocity, scaled_delta_time);
	gfc_vector3d_add(self->position, self->position, scaled_physics);

	//reset grounded, so we do collisions, then edge check only if we need to
	self->grounded = 0;

	//do any bounds checks with other entities
	entity_collide_all(self);
	
	//snap the player to the level only if they're not moving upwards and if they're not already grounded (so we dont snap through entities)
	if (self->velocity.z <= 0 && !self->grounded)
	{
		gfc_vector3d_copy(edge.b, self->position);
		ground_snap_offset = self->bounds.z - 2; //bottom of bounding box minus 2 so slopes/steps work
		edge.b.z += ground_snap_offset;

		gfc_vector3d_copy(edge.a, self->position);
		ground_snap_offset = self->bounds.d * 0.5;
		edge.a.z += ground_snap_offset;

		if (level_line_test(level_get_the(), edge, &floor))
		{
			self->position.z = floor.z - self->bounds.z; //shoudl come from stats in player config 
			self->velocity.z = 0;
			self->grounded = 1;
		}
		else {
			self->grounded = 0;
		}
	}

	//then check against world bounds
	bitmask = entity_level_bounds_enforce(self);
	
	//sync player rotation with the camera
	self->rotation.z = data->cameraRotation.x + GFC_PI;

	player_sync_camera(self);
	return 1;
}

void player_sync_camera(Entity* self)
{
	PlayerEntityData* data;
	GFC_Matrix4* view;
	GFC_Vector3D camera_position_offset = { 0, -1.5,0 };
	GFC_Vector3D camera_focus_offset = { 0 };
	
	if (!self || !self->data)
	{
		return;
	}
	data = (PlayerEntityData*)self->data;

	view = gf3d_vgraphics_get_view_matrix();

	gfc_vector3d_scale(camera_position_offset, camera_position_offset, data->cameraFollowDistance);
	gfc_vector3d_rotate_about_x(&camera_position_offset, data->cameraRotation.y);

	gfc_vector3d_rotate_about_z(&camera_position_offset, data->cameraRotation.x);
	gfc_vector3d_add(camera_position_offset, camera_position_offset, self->position);

	gfc_vector3d_add(camera_focus_offset, self->position, data->cameraFocusOffset);
	//setup view matrix for new position
	gfc_matrix4_view(
		*view,
		camera_position_offset,
		camera_focus_offset,
		gfc_vector3d(0, 0, 1)
	);
}