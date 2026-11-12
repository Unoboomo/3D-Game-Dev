#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

/**
* @brief spawns an item from def
*/
Entity* spawn_item_from_def(const char* itemName, GFC_Vector3D position);


#endif