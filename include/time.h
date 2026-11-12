#ifndef __TIME_H__
#define __TIME_H__


void time_system_init();

/**
 * @brief get the real elapsed time since last frame, used for menuing, camera, unscaled movement, etc.
 * @return the real delta
 */
float time_get_real_delta();

/**
 * @brief get the scaled elapsed time since last frame (bullet time), used for most physics, movement, in-game timers, etc.
 * @return the scaled delta
 */
float time_get_scaled_delta();

/**
 * @brief change the current time scale
 * @param time_scale the time scale to change to, non-negative float. 0 stops time, 1 is normal time
 */
void time_set_time_scale(float time_scale);

/**
 * @brief gets the current time scale
 * @return the current time scale
 */
float time_get_time_scale();

/**
 * @brief updates the timing info for the next frame
 */
void time_frame_update();

#endif