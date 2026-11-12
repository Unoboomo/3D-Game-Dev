#include "simple_logger.h"

#include "gfc_primitives.h"

#include "gf3d_obj_load.h"

#include "level.h"

static Level* the_level;

Level* level_new()
{
	Level* level;
	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level)
	{
		slog("failed to allocate level data");
		return NULL;
	}
	//boiler plate init stuff
	gfc_matrix4_identity(level->terrainMatrix);
	return level;
}

void level_free(Level* level)
{
	if (!level)
	{
		return;
	}
	if (level->sky)
	{
		model_free(level->sky);
	}
	if (level->terrain)
	{
		model_free(level->terrain);
	}
	if (level->backgroundMusic)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(level->backgroundMusic);
	}
	if (level->json)
	{
		sj_free(level->json);
	}
	free(level);
	the_level = NULL;
}

Level* level_load_from_file(const char* filename)
{
	SJson* json;
	SJson* data;
	const char* str;
	Level* level;
	json = sj_load(filename);
	if (!json)
	{
		slog("failed to open level file %s", filename);
		return NULL;
	}
	data = sj_object_get_value(json, "level");
	if (!data)
	{
		sj_free(json);
		slog("file %s does not contain level data", filename);
		return NULL;
	}
	level = level_new();
	if (!level)
	{
		sj_free(json);
		slog("could not create new level obeject for %s",filename);
		return NULL;
	}
	str = sj_object_get_string(data, "sky");
	if (str)
	{
		level->sky = model_load(str);
		if (level->sky)
		{
			gfc_line_cpy(level->skyFile, str);
		}
	}
	str = sj_object_get_string(data, "terrain");
	if (str)
	{
		level->terrain = model_load(str);
		if (level->terrain)
		{
			gfc_line_cpy(level->terrainFile, str);
		}
	}
	level->bounds = gfc_box_from_config(sj_object_get_value(data,"bounds"));
	str = sj_object_get_string(data, "music");
	if (str)
	{
		level->backgroundMusic = gfc_sound_load_music(str);
		if (level->backgroundMusic)
		{
			gfc_line_cpy(level->musicFile, str);
			Mix_PlayMusic(level->backgroundMusic, -1);
			
		}
	}
	level->json = json;
	if (the_level)
	{
		level_free(the_level);
	}
	the_level = level;
	return level;
}

Level* level_get_the()
{
	return the_level;
}


void level_queue_render(Level* level)
{
	GFC_Matrix4 id;
	if (!level)
	{
		return;
	}
	gfc_matrix4_identity(id);
	if (level->sky)
	{
		model_queue_render_sky(level->sky, id, GFC_COLOR_WHITE);
	}
	if (level->terrain)
	{
		model_queue_render(level->terrain, id, GFC_COLOR_WHITE);

	}
}

Uint8 level_bounds_check(Level* level, GFC_Box aabb, GFC_Vector3D* min_translation_vec)
{
	//in the future, return a bitmask so you can determine what directions you collide in
	// use uint8: +x, +y, +z, -x, -y, -z, unused, unused bitmask
	// aabb -> axis aligned bounding box
	

	//	LEVEL_BOUNDS_NONE = 0,
	//	LEVEL_BOUNDS_POS_X = 1 << 0,
	//	LEVEL_BOUNDS_POS_Y = 1 << 1,
	//	LEVEL_BOUNDS_POS_Z = 1 << 2,
	//	LEVEL_BOUNDS_NEG_X = 1 << 3,
	//	LEVEL_BOUNDS_NEG_Y = 1 << 4,
	//	LEVEL_BOUNDS_NEG_Z = 1 << 5,
	//	LEVEL_BOUNDS_ERROR = 1 << 6,

	Uint8 bitmask = 0;
	float level_max;
	float box_max;


	if (min_translation_vec)
	{
		*min_translation_vec = gfc_vector3d(0, 0, 0);
	}

	if (!level || level->bounds.w <= 0 || level->bounds.h <= 0 || level->bounds.d <= 0 || aabb.w <= 0 || aabb.h <= 0 || 
		aabb.d <= 0 || aabb.w > level->bounds.w || aabb.h > level->bounds.h || aabb.d > level->bounds.d)
	{
		return LEVEL_BOUNDS_ERROR;
	}

	if (aabb.x < level->bounds.x)
	{
		bitmask |= LEVEL_BOUNDS_NEG_X;
		if (min_translation_vec)
		{
			min_translation_vec->x = level->bounds.x - aabb.x;
		}
	}
	else 
	{
		box_max = aabb.x + aabb.w;
		level_max = level->bounds.x + level->bounds.w;
		if (box_max > level_max)
		{
			bitmask |= LEVEL_BOUNDS_POS_X;
			if (min_translation_vec)
			{
				min_translation_vec->x = level_max - box_max;
			}
		}
	}
	if (aabb.y < level->bounds.y)
	{
		bitmask |= LEVEL_BOUNDS_NEG_Y;
		if (min_translation_vec)
		{
			min_translation_vec->y = level->bounds.y - aabb.y;
		}
	}
	else
	{
		box_max = aabb.y + aabb.h;
		level_max = level->bounds.y + level->bounds.h;
		if (box_max > level_max)
		{
			bitmask |= LEVEL_BOUNDS_POS_Y;
			if (min_translation_vec)
			{
				min_translation_vec->y = level_max - box_max;
			}
		}
	}
	if (aabb.z < level->bounds.z)
	{
		bitmask |= LEVEL_BOUNDS_NEG_Z;
		if (min_translation_vec)
		{
			min_translation_vec->z = level->bounds.z - aabb.z;
		}
	}
	else
	{
		box_max = aabb.z + aabb.d;
		level_max = level->bounds.z + level->bounds.d;
		if (box_max > level_max)
		{
			bitmask |= LEVEL_BOUNDS_POS_Z;
			if (min_translation_vec)
			{
				min_translation_vec->z = level_max - box_max;
			}
		}
	}

	return bitmask;
}

Uint8 level_line_test(Level* level, GFC_Edge3D edge, GFC_Vector3D* contact)
{
	MeshPrimitive* prim;
	GFC_Triangle3D t;
	int i, j, c;
	GFC_Vector3D out = { -1 };
	if (!level || !level->terrain || !level->terrain->mesh)
	{
		slog("no level data");
		return 0;
	}
	c = gfc_list_count(level->terrain->mesh->primitives);
	for (i = 0; i < c; i++)
	{
		prim = gfc_list_nth(level->terrain->mesh->primitives, i);
		if (!prim)
		{
			slog("primitive is NULL");
			continue;
		}
		if (!prim->objData)
		{
			slog("prim obj data is null");
			continue;
		}

		for (j = 0; j < prim->objData->face_count; j++)
		{
			//ts is hella inefficient
			t.a = prim->objData->faceVertices[prim->objData->outFace[j].verts[0]].vertex;
			t.b = prim->objData->faceVertices[prim->objData->outFace[j].verts[1]].vertex;
			t.c = prim->objData->faceVertices[prim->objData->outFace[j].verts[2]].vertex;
			if (gfc_triangle_edge_test(edge, t, &out))
			{
				//outputs the first face found, not necessarily the closest one, may snap through floor if 
				//multiple floors exist, and first face is on lower floor

				if (contact)
				{
					*contact = out;
				}
				return 1;
			}
		}
	}
	return 0;

}