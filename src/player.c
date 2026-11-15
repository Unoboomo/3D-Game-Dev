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

void player_change_movement_state(PlayerMovementState state);

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

		data->movement_state = PMS_Normal;

		data->max_air_jumps = PLAYER_MAX_AIR_JUMPS;
		data->jumps_remaining = PLAYER_MAX_AIR_JUMPS;
		data->jumped_this_frame = 0;
		data->jump_velocity = PLAYER_STANDING_JUMP_VELOCITY;

		data->max_air_dashes = PLAYER_MAX_AIR_DASHES;
		data->dashes_remaining = PLAYER_MAX_AIR_DASHES;
		data->dashed_this_frame = 0;
		data->dash_exhausted = 0;

		data->reduced_gravity = 0;
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
	float current_speed;
	if (!self || !self->data)
	{
		return 1;
	}
	data = (PlayerEntityData*)self->data;
	
	//will eventually wrap this in if !grounded

	if (gfc_input_command_down("dash") && (self->grounded || data->dashes_remaining > 0)) // && stamina > min required to dash
	{
		if (!data->dash_exhausted)
		{
			time_set_time_scale(BULLET_TIME_TIME_SCALE);
		}
		else
		{
			data->dash_exhausted = 1;
			time_set_time_scale(REAL_TIME_TIME_SCALE);
		}
	}
	else
	{
		time_set_time_scale(REAL_TIME_TIME_SCALE);
		if (self->grounded)
		{
			data->dash_exhausted = 0;
		}
	}

	if (gfc_input_command_released("dash"))
	{
		data->dashed_this_frame = 1;
	}
	//end of wrap

	if (gfc_input_command_down("crouch"))
	{
		if (data->movement_state == PMS_Normal && self->grounded)
		{
			//crouching/sliding scale change:
			current_speed = (self->velocity.x * self->velocity.x) + (self->velocity.y * self->velocity.y);
			if (current_speed > (PLAYER_SLIDING_SPEED_THRESHOLD * PLAYER_SLIDING_SPEED_THRESHOLD))
			{

				player_change_movement_state(PMS_Sliding);
				//give speed boost here or in the function
			}
			else
			{ 
				player_change_movement_state(PMS_Crouching);
			}
		}
	}
	else //(!gfc_input_command_down("crouch"))
	{
		if (data->movement_state == PMS_Sliding || data->movement_state == PMS_Crouching)
		{ 
			player_change_movement_state(PMS_Normal);
		}
	}

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
	//GFC_Vector3D offset;
	float ground_snap_offset;
	GFC_Vector3D floor;
	float scaled_delta_time;
	GFC_Vector3D scaled_physics;
	GFC_TextLine debug;
	float current_speed;

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
		if (data->movement_state == PMS_Sliding)
		{
			acceleration = PLAYER_SLIDING_ACCELERATION;
			friction = PLAYER_SLIDING_FRICTION;
			speed_soft_cap = PLAYER_GROUND_SPEED_SOFT_CAP;
		}
		else
		{
			if (data->movement_state == PMS_Crouching)
			{
				acceleration = PLAYER_GROUND_ACCELERATION;
				friction = PLAYER_CROUCH_FRICTION;
				speed_soft_cap = PLAYER_CROUCH_SPEED_SOFT_CAP;
			}
			else
			{
				acceleration = PLAYER_GROUND_ACCELERATION;
				friction = PLAYER_GROUND_FRICTION;
				speed_soft_cap = PLAYER_GROUND_SPEED_SOFT_CAP;
			}
		}
		
		data->jumps_remaining = data->max_air_jumps;
		data->dashes_remaining = data->max_air_dashes;
	}
	else //(!self->grounded)
	{
		speed_soft_cap = PLAYER_AIR_SPEED_SOFT_CAP;
		friction = PLAYER_AIR_FRICTION;
		acceleration = PLAYER_AIR_ACCELERATION;
	}

	// more granular movement while slowed down to dodge and stuff
	if (gfc_input_command_down("dash"))
	{
		speed_soft_cap *= PLAYER_BULLET_TIME_SPEED_SCALE;
		acceleration *= PLAYER_BULLET_TIME_ACCELERATION_SCALE;
		friction *= PLAYER_BULLET_TIME_FRICTION_SCALE;
	}

	if (data->dashed_this_frame)
	{
		data->dashed_this_frame = 0;
		if (self->grounded || data->dashes_remaining > 0)
		{
			if (!self->grounded)
			{
				data->dashes_remaining--;
			}

			//dash logic
			if (gfc_vector3d_is_zero(data->wish_direction) || !self->grounded)
			{ //if we're not grounded or not holding a direction, dash forward
				data->dash_direction = gfc_vector3d(0, 1, 0);
				gfc_vector3d_rotate_about_z(&data->dash_direction, data->cameraRotation.x);
			}
			else //(self->grounded && !gfc_vector3d_is_zero(data->wish_direction))
			{
				gfc_vector3d_copy(data->dash_direction, data->wish_direction);

			}
			self->velocity.x = data->dash_direction.x * PLAYER_DASH_VELOCITY;
			self->velocity.y = data->dash_direction.y * PLAYER_DASH_VELOCITY;

			//then resent z velocity so we dont instantly crater
			if (!self->grounded && self->velocity.z < 0)
			{
				self->velocity.z = 0;
			}
		}
		
	}

	player_get_movement_state_as_GFC_Textline(debug);
	//slog("player state: %s", debug);
	slog_sync();

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
	if (gfc_vector3d_is_zero(data->wish_direction))
	{
		//if no wasd held, dont increase velocity
	}
	else 
	{
		//wasd is held
		if (data->movement_state == PMS_Sliding && self->grounded)
		{
			// when sliding and on the ground, wasd does nothing, you cant increase your velocity with them
		}
		else //(data->movement_state != PMS_Sliding || !self->grounded)
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
	}


	//3. apply gravity
	if (!self->grounded)
	{
		if (data->reduced_gravity)
		{
			self->velocity.z -= PLAYER_REDUCED_GRAVITY * scaled_delta_time;
		}
		else
		{
			self->velocity.z -= PLAYER_GRAVITY * scaled_delta_time;
		}
		
	}

	//3.5 jump
	if (data->jumped_this_frame)
	{
		data->jumped_this_frame = 0;
		if (self->grounded || data->jumps_remaining > 0)
		{
			if (!self->grounded)
			{
				data->jumps_remaining--;
			}

			//jump logic
			self->velocity.z = data->jump_velocity;
			self->grounded = 0;
		}
	}

	//4. do movement
	gfc_vector3d_scale(scaled_physics, self->velocity, scaled_delta_time);
	gfc_vector3d_add(self->position, self->position, scaled_physics);

	//reset grounded, so we do collisions, then edge check only if we need to
	self->grounded = 0;
	data->reduced_gravity = 0;

	//do any bounds checks with other entities
	entity_collide_all(self);
	
	//snap the player to the level only if they're not moving upwards and if they're not already grounded (so we dont snap through entities)
	if (self->velocity.z <= 0 && !self->grounded)
	{
		gfc_vector3d_copy(edge.b, self->position);
		ground_snap_offset = self->bounds.z - 2; //bottom of bounding box minus 2 so slopes/steps work
		edge.b.z += ground_snap_offset;

		gfc_vector3d_copy(edge.a, self->position);
		ground_snap_offset = self->bounds.z + 8;
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
	
	current_speed = (self->velocity.x * self->velocity.x) + (self->velocity.y * self->velocity.y);
	if (data->movement_state == PMS_Sliding)
	{
		if (self->grounded && current_speed < (PLAYER_SLIDING_SPEED_THRESHOLD * PLAYER_SLIDING_SPEED_THRESHOLD))
		{
			player_change_movement_state(PMS_Crouching);
		}
	}
	else if (data->movement_state == PMS_Crouching)
	{
		if (self->grounded && current_speed > (PLAYER_SLIDING_SPEED_THRESHOLD * PLAYER_SLIDING_SPEED_THRESHOLD))
		{
			player_change_movement_state(PMS_Sliding);
		}
	}

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

void player_change_movement_state(PlayerMovementState state)
{
	PlayerEntityData* data;
	if (!the_player || !the_player->data)
	{
		slog("cannot change movement state of nonexistent player");
		return;
	}
	data = (PlayerEntityData*)the_player->data;

	if (state == PMS_Normal) //transitioning to
	{
		if (data->movement_state == PMS_Sliding || data->movement_state == PMS_Crouching) //transitioning from
		{
			the_player->scale.z *= 2;
			the_player->bounds.d *= 2;
			data->movement_state = PMS_Normal;
			data->jump_velocity = PLAYER_STANDING_JUMP_VELOCITY;
			return;
		}
	}
	else if (state == PMS_Sliding) //transitioning to
	{
		if (data->movement_state == PMS_Normal) //transitioning from
		{
			the_player->scale.z *= 0.5;
			the_player->bounds.d *= 0.5;
			data->movement_state = PMS_Sliding;
			data->jump_velocity = PLAYER_SLIDING_JUMP_VELOCITY;
			return;
		}
		else if (data->movement_state == PMS_Crouching) //transitioning from
		{
			data->movement_state = PMS_Sliding;
			data->jump_velocity = PLAYER_SLIDING_JUMP_VELOCITY;
			return;
		}
	}
	else if (state == PMS_Crouching) //transitioning to
	{
		if (data->movement_state == PMS_Normal) //transitioning from
		{
			the_player->scale.z *= 0.5;
			the_player->bounds.d *= 0.5;
			data->movement_state = PMS_Crouching;
			data->jump_velocity = PLAYER_CROUCHING_JUMP_VELOCITY;
			return;
		}
		else if (data->movement_state == PMS_Sliding) //transitioning from
		{
			data->movement_state = PMS_Crouching;
			data->jump_velocity = PLAYER_CROUCHING_JUMP_VELOCITY;
			return;
		}
	}
}

void player_get_movement_state_as_GFC_Textline(GFC_TextLine text)
{
	PlayerEntityData* data;
	if (!the_player)
	{
		slog("cannot get movement state for nonexistent player");
		return;
	}
	if (!the_player->data)
	{
		slog("the player has no data, cannot get movement state");
	}

	data = (PlayerEntityData*)the_player->data;

	if (data->movement_state == PMS_Normal)
	{
		gfc_line_cpy(text, "Normal");
	}
	else if (data->movement_state == PMS_Dashing)
	{
		gfc_line_cpy(text, "Dashing");
	}
	else if (data->movement_state == PMS_Sliding)
	{
		gfc_line_cpy(text, "Sliding");
	}
	else if (data->movement_state == PMS_Crouching)
	{
		gfc_line_cpy(text, "Crouching");
	}
	else
	{
		gfc_line_cpy(text, "Error");
	}
}
