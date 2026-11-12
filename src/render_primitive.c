#include "simple_logger.h"

#include "render_primitive.h"

typedef struct 
{
	Model* cube;
	Model* sphere;
	Model* cylinder;
} render_primitive_data;

static render_primitive_data unit_primitives = { 0 };

void render_primitive_close();

void render_primitive_init()
{
	unit_primitives.cube = model_load("models/primitives/cube.model");
	unit_primitives.sphere = model_load("models/primitives/sphere.model");
	//unit_primitives.cylinder = model_load("models/primitives/cylinder.model");
	atexit(render_primitive_close);
}

void render_primitive_close()
{
	if (unit_primitives.cube)
	{
		model_free(unit_primitives.cube);
	}
	if (unit_primitives.sphere)
	{
		model_free(unit_primitives.sphere);
	}

	if (unit_primitives.cylinder)
	{
		model_free(unit_primitives.cylinder);
	}
	
}

void render_primitive_cube(GFC_Box cube, GFC_Color colorMod)
{
	if (!unit_primitives.cube)
	{
		slog("cannot render a unit cube whose model doesn't exist");
	}

	GFC_Matrix4 mat;
	gfc_matrix4_from_vectors(
		mat,
		gfc_vector3d((cube.x + cube.w * 0.5), (cube.y + cube.h * 0.5), (cube.z + cube.d * 0.5)),
		gfc_vector3d(0, 0, 0),
		gfc_vector3d(cube.w, cube.h, cube.d)
	);
	model_queue_render(unit_primitives.cube, mat, colorMod);
}

void render_primitive_sphere(GFC_Sphere sphere, GFC_Color colorMod);
