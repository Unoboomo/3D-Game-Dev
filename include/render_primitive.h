#ifndef __RENDER_PRIMITIVE_H__
#define __RENDER_PRIMITIVE_H__

#include "gfc_primitives.h"

#include "model.h"

void render_primitive_init();


void render_primitive_cube(GFC_Box cube, GFC_Color colorMod);

//void render_primitive_cylinder(GFC_Cylinder cylinder, GFC_Color colorMod);

void render_primitive_sphere(GFC_Sphere sphere, GFC_Color colorMod);


#endif

