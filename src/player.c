#include "simple_logger.h"

#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "mouse.h"
#include "level.h"
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

	if (the_player)
	{
		return the_player;
	}

	self = entity_new();
	if (!self) {
		slog("failed to create a new player entity");
		return NULL;
	}

	self->model = model_load("models/dino/dino.model");
	self->position = position;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	self->bounds = gfc_box(-7.16433, -4.18962, -6.57308, 14.3287, 9.0455, 10.4108);

	data = gfc_allocate_array(sizeof(PlayerEntityData), 1);
	if (data)
	{
		data->cameraFollowDistance = 50;
		data->cameraRotation.x = 0;
		data->cameraRotation.y = GFC_PI/6;

	}
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

int player_think(Entity* self)
{
	GFC_Vector3D move = { 0 };
	PlayerEntityData* data;

	if (!self || !self->data)
	{
		return 0;
	}
	data = (PlayerEntityData*)self->data;
	/*
	if (gfc_input_command_down("panup"))
	{
		data->cameraRotation.y += 0.05;
	}
	if (gfc_input_command_down("pandown"))
	{
		data->cameraRotation.y -= 0.05;
	}
	if (gfc_input_command_down("panright"))
	{
		data->cameraRotation.x -= 0.05;
	}
	if (gfc_input_command_down("panleft"))
	{
		data->cameraRotation.x += 0.05;
	}
	*/

	mouse_update_camera(&(data->cameraRotation));
	if (gfc_input_command_down("item_up"))
	{
		move.y = 1;
	}
	if (gfc_input_command_down("item_down"))
	{
		move.y = -1;
	}
	if (gfc_input_command_down("item_right"))
	{
		move.x = 1;
	}
	if (gfc_input_command_down("item_left"))
	{
		move.x = -1;
	}
	
	if (move.x || move.y)
	{
		gfc_vector3d_normalize(&move);
		gfc_vector3d_rotate_about_z(&move, data->cameraRotation.x);
		self->velocity = move;
	}
	else
	{
		self->velocity = gfc_vector3d(0,0,0);
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

	if (!self || !self->data)
	{
		return 0;
	}
	data = (PlayerEntityData*)self->data;

	//if (!entity_move_check((self, self->velocity)))
	{
		//gfc_vector3d_set()
	}


	

	//do movement
	gfc_vector3d_add(self->position, self->position, self->velocity);

	//snap the player to the level only if they're not moving upwards
	if (self->velocity.z > 0)
	{
		;
	}
	else
	{
		ground_snap_offset = self->bounds.z;
		gfc_vector3d_copy(edge.b, self->position);
		edge.b.z += ground_snap_offset;
		ground_snap_offset = self->bounds.d * 0.75;
		gfc_vector3d_copy(edge.a, edge.b);
		edge.a.z += ground_snap_offset;
		if (level_line_test(level_get_the(), edge, &floor))
		{
			self->position.z = floor.z + (-self->bounds.z); //shoudl come from stats in player config file
			self->velocity.z = 0;
			self->grounded = 1;
		}
		else {
			self->grounded = 0;
		}
	}

	//do any bounds checks with other entities
	
	
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
	GFC_Vector3D camera_offset = { 0, -1, 0 };
	
	if (!self || !self->data)
	{
		return;
	}
	data = (PlayerEntityData*)self->data;

	view = gf3d_vgraphics_get_view_matrix();

	gfc_vector3d_scale(camera_offset, camera_offset, data->cameraFollowDistance);
	gfc_vector3d_rotate_about_x(&camera_offset, data->cameraRotation.y);

	gfc_vector3d_rotate_about_z(&camera_offset, data->cameraRotation.x);
	gfc_vector3d_add(camera_offset, camera_offset, self->position);

	//setup view matrix for new position
	gfc_matrix4_view(
		*view,
		camera_offset,
		self->position,
		gfc_vector3d(0, 0, 1)
	);
}