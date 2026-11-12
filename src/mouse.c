#include "simple_logger.h"

#include "gfc_config_def.h"

#include "mouse.h"

extern int __DEBUG;

typedef struct
{
	float sensitivity;
}MouseSystem;

static MouseSystem mouse_system = { 0 };

void mouse_system_close();

float mouse_get_sensitivity_from_config();

/**
 * @brief clamps an angle to -pi to pi
 * @param angle the angle of the yaw in radians
 * @return the clamped angle
 */
float mouse_clamp_yaw(float angle);

/**
 * @brief clamps an angle to -99*pi/200, 99*pi/200
 * @param angle the angle of the pitch in radians
 * @return the clamped angle
 */
float mouse_clamp_pitch(float angle);

void mouse_system_init()
{
	if (mouse_system.sensitivity) {
		slog("cannot initialize mouse system, already initialized");
		return;
	}
	
	mouse_system.sensitivity = mouse_get_sensitivity_from_config();
	if (!mouse_system.sensitivity)
	{
		slog("cannot function without the mouse");
		slog_sync();
		mouse_system_close();
		exit(-1);
		return;
	}

	if (__DEBUG)slog("mouse system initialized");
	atexit(mouse_system_close);
}

void mouse_system_close()
{
	memset(&mouse_system, 0, sizeof(MouseSystem));
	if (__DEBUG)slog("entity system closed");
}


float mouse_get_sensitivity_from_config()
{
	SJson* mouseCfg;
	float sensitivity;

	mouseCfg = gfc_config_def_get_by_name("mouse_settings", "sensitivity");
	if (!mouseCfg)
	{
		slog("cannot load mouse_settings to get sensitivity");
		return 0;
	}
	if (!sj_object_get_float(mouseCfg, "sensitivity", &sensitivity))
	{
		slog("cannot determine mouse sensitivity");
		return 0;
	}
	if (sensitivity <= 0)
	{
		slog("mouse sensitivity cannot be 0 or negative");
		return 0;
	}
	return sensitivity;
}

float mouse_clamp_yaw(float angle)
{
	float temp;
	temp = fmodf(angle + GFC_PI, GFC_2PI);

	if (temp < 0)
	{
		temp += GFC_2PI;
	}
	return temp - GFC_PI;
}

float mouse_clamp_pitch(float angle)
{
	float pos_clamp_value = GFC_HALF_PI * 0.99;
	float neg_clamp_value = -pos_clamp_value;
	if (angle > pos_clamp_value)
	{
		return pos_clamp_value;
	}
	if (angle < neg_clamp_value)
	{
		return neg_clamp_value;
	}
	return angle;
}

void mouse_update_camera(GFC_Vector3D* camera_rotation)
{
	int deltaX, deltaY;
	SDL_GetRelativeMouseState(&deltaX, &deltaY);

	camera_rotation->x = mouse_clamp_yaw(camera_rotation->x - mouse_system.sensitivity * deltaX);
	camera_rotation->y = mouse_clamp_pitch(camera_rotation->y - mouse_system.sensitivity * deltaY);
	return;
}