#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <SDL.h>
#include "gfc_vector.h"


/**
 * @brief initialize the mouse system so the mouse can be used for camera movement
 */
void mouse_system_init();

/**
 * @brief updates the camera rotation based on mouse input
 * @param camera_rotation [out] the rotation to be updated
 */
void mouse_update_camera(GFC_Vector3D* camera_rotation);

#endif