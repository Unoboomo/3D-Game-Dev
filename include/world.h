#ifndef __WORLD_H__
#define __WORLD_H__

#include "entity.h"

#define BOUNCE_PAD_VELOCITY 300.0f

/**
 * @brief gets an the touch function for an world_entity of the specified name
 * @param name: [in] the world_entity to get the touch function of
 * @return a function pointer to the touch function or NULL on error or failed to find
 */
void* world_get_touch_function_by_name(GFC_TextLine name);

#endif 

