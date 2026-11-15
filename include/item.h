#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

/**
* @brief spawns an item from def
*/
Entity* spawn_item_from_def(const char* itemName, GFC_Vector3D position);

/**
 * @brief gets an the touch function for an item of the specified name
 * @param name: [in] the item to get the touch function of
 * @return a function pointer to the touch function or NULL on error or failed to find
 */
void* item_get_touch_function_by_name(GFC_TextLine name);

#endif