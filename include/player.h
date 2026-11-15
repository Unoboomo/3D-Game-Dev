#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef enum
{
	PMS_Normal,
	PMS_Dashing,
	PMS_Sliding,
	PMS_Crouching,
	PMS_Error,
}PlayerMovementState;

typedef struct PlayerEntityData_s
{
	float					cameraFollowDistance;
	GFC_Vector3D			cameraRotation;
	GFC_Vector3D			cameraFocusOffset;

	GFC_Vector3D			wish_direction;

	PlayerMovementState		movement_state;

	int						max_air_jumps;
	int						jumps_remaining;
	int						jumped_this_frame;
	float					jump_velocity;

	int						max_air_dashes;
	int						dashes_remaining;
	int						dashed_this_frame;
	GFC_Vector3D			dash_direction;
	int						dash_exhausted;

	int						reduced_gravity;

	float					stamina_recovery_rate;
	float					max_stamina;
	float					current_stamina;
	int						bullet_time_active;

	float					air_control_scale;
	float					max_speed_scale;

	Uint8					wall_run_side;
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

#define PLAYER_BULLET_TIME_SPEED_SCALE			2.0f
#define PLAYER_BULLET_TIME_ACCELERATION_SCALE	4.0f
#define PLAYER_BULLET_TIME_FRICTION_SCALE		4.0f

#define BULLET_TIME_TIME_SCALE					0.2f
#define REAL_TIME_TIME_SCALE					1.0f

#define PLAYER_GROUND_SPEED_SOFT_CAP			150.0f
#define PLAYER_AIR_SPEED_SOFT_CAP				100.0f

#define PLAYER_GROUND_ACCELERATION				8.0f
#define PLAYER_AIR_ACCELERATION					4.0f

#define PLAYER_GROUND_FRICTION					8.0f
#define PLAYER_AIR_FRICTION						0.5f

#define PLAYER_SLIDING_SPEED_THRESHOLD			50.0f
#define PLAYER_SLIDING_FRICTION					1.0f
#define PLAYER_SLIDING_ACCELERATION				1.0f

#define PLAYER_CROUCH_SPEED_SOFT_CAP			PLAYER_SLIDING_SPEED_THRESHOLD
#define PLAYER_CROUCH_FRICTION					PLAYER_GROUND_FRICTION


#define PLAYER_VELOCITY_EPSILON					1.0f
#define PLAYER_GRAVITY							500.0f
#define PLAYER_REDUCED_GRAVITY					(PLAYER_GRAVITY * 0.3)

#define PLAYER_STANDING_JUMP_VELOCITY			150.0f
#define PLAYER_SLIDING_JUMP_VELOCITY			PLAYER_STANDING_JUMP_VELOCITY
#define PLAYER_CROUCHING_JUMP_VELOCITY			(PLAYER_STANDING_JUMP_VELOCITY * 0.7)

#define PLAYER_MAX_AIR_JUMPS					1
#define PLAYER_MAX_AIR_DASHES					1

#define PLAYER_DASH_VELOCITY					300.0f

#define PLAYER_MAX_STAMINA                     100.0f
#define PLAYER_STAMINA_RECOVERY_RATE            20.0f
#define PLAYER_BULLET_TIME_STAMINA_DRAIN_RATE   40.0f
#define PLAYER_DASH_STAMINA_COST                20.0f

/**
* @brief spawn a new player entity
* @param position where to spawn it
* @return NULL on error, or a pointer to the spawned player entity
*/
Entity* player_new(GFC_Vector3D position);

/**
 * @brief gets the player state as a gfc_textline so it can be sloged
 * @param text: [out] the gfc_textline the state is copied to
 */
void player_get_movement_state_as_GFC_Textline(GFC_TextLine text)
;

/**
* @brief get the player singleton
*/
Entity* player_get_the();

float player_get_current_stamina();

float player_get_max_stamina();
#endif