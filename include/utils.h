#ifndef __UTILS_H__
#define __UTILS_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_color.h"
#include "gfc_primitives.h"
#include "gfc_shape.h"


//UTILITY FUNCTIONS, mostly changes to gfc functions

typedef enum
{
	BOX_COLLISION_NONE = 0,
	BOX_COLLISION_POS_X = 1 << 0,
	BOX_COLLISION_POS_Y = 1 << 1,
	BOX_COLLISION_POS_Z = 1 << 2,
	BOX_COLLISION_NEG_X = 1 << 3,
	BOX_COLLISION_NEG_Y = 1 << 4,
	BOX_COLLISION_NEG_Z = 1 << 5,
	BOX_COLLISION_ERROR = 1 << 6,
}BoxCollisionType;

/**
 * @brief determines if two boxes collide
 * @param a: [in] the box to determine collision direction with respect to
 * @param b: [in] the other box
 * @param min_translation: [out] the minimum translation needed to apply to a to make a and b not collide
 * @return a bitmask encoding the direction collision occured
 * 
 * SEE: BoxCollisionType in utils.h
 */
Uint8 utils_box_overlap(GFC_Box a, GFC_Box b, GFC_Vector3D *min_translation);

/**
 * @brief inverts the collision directions of a bitmask
 * @param bitmask: [in] the bitmask to invert
 * @return the inverted bitmask
 */
Uint8 utils_invert_bitmask(Uint8 bitmask);

#endif 
