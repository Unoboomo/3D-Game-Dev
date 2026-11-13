#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"


/**
 * first: dampen in velocity by current friction: v_damp = v - v * friction * dtime, if v_damp and v are diff signs, v_damp = 0
 * then: acceleration
 */
typedef struct PlayerEntityData_s
{
	float			cameraFollowDistance;
	GFC_Vector3D	cameraRotation;
	GFC_Vector3D	cameraFocusOffset;

	GFC_Vector3D	wish_direction;


	int				jumps_remaining;
	int				max_air_jumps;
	int				jumped_this_frame;

	int				dashed_this_frame;
	GFC_Vector3D	dash_direction;

	float			stamina;
	float			stamina_recovery_rate;


	Uint8			wall_run_side;
	//other player specific data

	//Uint8	jump_buffer;			//gives the player a #frame jump buffer so they are not frame perfect
	//Uint8	wall_jump_buffer;		//gives the player a #frame wall jump buffer so they are not frame perfect
	//int		last_x_collision_dir;	// holds the last world collision x direction, + for right, - for left

	//Uint8	ground_pound_delay;		//freezes the player's movement to perform the ground pound
	//Uint8	ground_pounding;		//is the player ground pounding?
	//Uint8	ground_pound_recovery;	//freezes the player's movement while recovering from the ground

	//Uint8	crouching;				//is the player crouching?

	//int		coin_count;
	//int		lives_count;
}PlayerEntityData;

#define PLAYER_BULLET_TIME_SPEED_SCALE			1.5f
#define PLAYER_BULLET_TIME_ACCELERATION_SCALE	2.0f

#define BULLET_TIME_TIME_SCALE					0.2f
#define REAL_TIME_TIME_SCALE					1.0f

#define PLAYER_SPEED_SOFT_CAP_GROUND			200.0f
#define PLAYER_SPEED_SOFT_CAP_AIR				150.0f

#define PLAYER_ACCELERATION_GROUND				8.0f
#define PLAYER_ACCELERATION_AIR					4.0f

#define PLAYER_FRICTION_GROUND					8.0f
#define PLAYER_FRICTION_AIR						0.5f

#define PLAYER_VELOCITY_EPSILON					1.0f
#define PLAYER_GRAVITY							500.0f

#define PLAYER_JUMP_VELOCITY					150.0f
#define PLAYER_MAX_AIR_JUMPS					1

#define PLAYER_DASH_VELOCITY					300.0f

/**
* @brief spawn a new player entity
* @param position where to spawn it
* @return NULL on error, or a pointer to the spawned player entity
*/
Entity* player_new(GFC_Vector3D position);

/**
* @brief get the player singleton
*/
Entity* player_get_the();

#endif