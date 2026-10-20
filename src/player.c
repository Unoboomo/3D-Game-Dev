#include "simple_logger.h"

#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "player.h"
//#include "level.h"

int player_think(Entity* self);
int player_update(Entity* self);
void player_free(Entity* self);


Entity* player_new(GFC_Vector3D position) {
	Entity* self;
	PlayerEntityData* data;

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
	data = gfc_allocate_array(sizeof(PlayerEntityData), 1);
	if (data)
	{
		data->cameraFollowDistance = 50;
		data->cameraRotation.x = 0;
		data->cameraRotation.y = GFC_PI/6;

	}
	self->data = data;

	return self;
}

int player_think(Entity* self)
{
	//GFC_Edge3D edge;
	//GFC_Vector3D *floor, offset;
	PlayerEntityData* data;
	GFC_Matrix4* view;
	GFC_Vector3D move = { 0 };
	//GFC_Vector3D offset = { 0, -1, 0};

	if (!self || !self->data)
	{
		return 0;
	}
	data = (PlayerEntityData*)self->data;
	
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

	//offset = gfc_vector3d(0, 0, 100);
	//gfc_vector3d_add(edge.a, self->position, offset);
	//offset = gfc_vector3d(0, 0, -100);
	//gfc_vector3d_add(edge.b, self->position, offset);
	//
	//if (level_line_test(level_get_the(), edge, &floor))
	//{
	//	self->position.z = floor->z + 7; //shoudl come from stats in player config file
	//}
	
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
	GFC_Matrix4* view;
	GFC_Vector3D move = { 0 };
	GFC_Vector3D offset = { 0, -1, 0 };

	if (!self || !self->data)
	{
		return 0;
	}
	data = (PlayerEntityData*)self->data;

	self->rotation.z = data->cameraRotation.x + GFC_PI;

	view = gf3d_vgraphics_get_view_matrix();

	gfc_vector3d_scale(offset, offset, data->cameraFollowDistance);
	gfc_vector3d_rotate_about_x(&offset, data->cameraRotation.y);

	gfc_vector3d_rotate_about_z(&offset, data->cameraRotation.x);
	gfc_vector3d_add(offset, offset, self->position);

	gfc_vector3d_add(self->position, self->position, self->velocity);

	//setup view matrix for new position (this shoudl technically be setup after all updating is done)
	gfc_matrix4_view(
		*view,
		offset,
		self->position,
		gfc_vector3d(0, 0, 1)
	);

}

void player_free(Entity* self) {
	PlayerEntityData* data;
	if (!self || !self->data)
	{
		return 0;
	}
	
	data = (PlayerEntityData *) self->data;
	//clean up data

	//free the data
	self->data = NULL;
}