#include "simple_logger.h"

#include "gfc_config.h"

#include "utils.h"

Uint8 utils_box_overlap(GFC_Box a, GFC_Box b, GFC_Vector3D* min_translation)
{
    
    float overlap_pos_x;
    float overlap_pos_y;
    float overlap_pos_z;
    float overlap_neg_x;
    float overlap_neg_y;
    float overlap_neg_z;

    float x_collision_overlap;
    Uint8 x_collision_direction;
    float y_collision_overlap;
    Uint8 y_collision_direction;
    float z_collision_overlap;
    Uint8 z_collision_direction;

    if (min_translation)
    {
        *min_translation = gfc_vector3d(0,0,0);
    }

    if (a.w <= 0 || a.h <= 0 || a.d <= 0 || b.w <= 0 || b.h <= 0 || b.d <= 0)
    {
        slog("box with negative or zero length provided");
        return BOX_COLLISION_ERROR;
    }

    overlap_neg_x = b.x + b.w - a.x;
    if (overlap_neg_x <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    overlap_neg_y = b.y + b.h - a.y;
    if (overlap_neg_y <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    overlap_neg_z = b.z + b.d - a.z;
    if (overlap_neg_z <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    overlap_pos_x = a.x + a.w - b.x;
    if (overlap_pos_x <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    overlap_pos_y =  a.y + a.h - b.y;
    if (overlap_pos_y <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    overlap_pos_z = a.z + a.d - b.z;
    if (overlap_pos_z <= 0)
    {
        return BOX_COLLISION_NONE;
    }

    if (overlap_neg_x < overlap_pos_x)
    {
        x_collision_overlap = overlap_neg_x;
        x_collision_direction = BOX_COLLISION_NEG_X;
    }
    else
    {
        x_collision_overlap = overlap_pos_x;
        x_collision_direction = BOX_COLLISION_POS_X;
    }
    if (overlap_neg_y < overlap_pos_y)
    {
        y_collision_overlap = overlap_neg_y;
        y_collision_direction = BOX_COLLISION_NEG_Y;
    }
    else
    {
        y_collision_overlap = overlap_pos_y;
        y_collision_direction = BOX_COLLISION_POS_Y;
    }
    if (overlap_neg_z < overlap_pos_z)
    {
        z_collision_overlap = overlap_neg_z;
        z_collision_direction = BOX_COLLISION_NEG_Z;
    }
    else
    {
        z_collision_overlap = overlap_pos_z;
        z_collision_direction = BOX_COLLISION_POS_Z;
    }

    if (x_collision_overlap <= y_collision_overlap && x_collision_overlap <= z_collision_overlap)
    {
        if (min_translation)
        {
            *min_translation = gfc_vector3d(x_collision_overlap, 0, 0);
            if (x_collision_direction & BOX_COLLISION_POS_X)
            {
                min_translation->x *= -1;
            }
        }
        return x_collision_direction;
    }
    else if (y_collision_overlap <= x_collision_overlap && y_collision_overlap <= z_collision_overlap)
    {
        if (min_translation)
        {
            *min_translation = gfc_vector3d(0, y_collision_overlap, 0);
            if (y_collision_direction & BOX_COLLISION_POS_Y)
            {
                min_translation->y *= -1;
            }
        }
        return y_collision_direction;
    }
    else
    {
        if (min_translation)
        {
            *min_translation = gfc_vector3d(0, 0, z_collision_overlap);
            if (z_collision_direction & BOX_COLLISION_POS_Z)
            {
                min_translation->z *= -1;
            }
        }
        return z_collision_direction;
    }
}

Uint8 utils_invert_bitmask(Uint8 bitmask)
{
    Uint8 inverted_bitmask = 0;
    if (bitmask & BOX_COLLISION_POS_X)
    {
        inverted_bitmask |= BOX_COLLISION_NEG_X;
    }
    if (bitmask & BOX_COLLISION_NEG_X)
    {
        inverted_bitmask |= BOX_COLLISION_POS_X;
    }

    if (bitmask & BOX_COLLISION_POS_Y)
    {
        inverted_bitmask |= BOX_COLLISION_NEG_Y;
    }
    if (bitmask & BOX_COLLISION_NEG_Y)
    {
        inverted_bitmask |= BOX_COLLISION_POS_Y;
    }

    if (bitmask & BOX_COLLISION_POS_Z)
    {
        inverted_bitmask |= BOX_COLLISION_NEG_Z;
    }
    if (bitmask & BOX_COLLISION_NEG_Z)
    {
        inverted_bitmask |= BOX_COLLISION_POS_Z;
    }

    if (bitmask & BOX_COLLISION_ERROR)
    {
        inverted_bitmask |= BOX_COLLISION_ERROR;
    }

    return inverted_bitmask;
}