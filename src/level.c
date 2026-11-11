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

int level_bounds_check(Level* level, GFC_Box aabb)
{
	//in the future, return a bitmask so you can determine what directions you collide in
	// use uint8: +x, +y, +z, -x, -y, -z, unused, unused bitmask
	// aabb -> axis aligned bounding box
	if (!level)
	{
		return 0;
	}

	if (aabb.x < level->bounds.x)
	{
		return 1;
	}
	if (aabb.y < level->bounds.y)
	{
		return 1;
	}
	if (aabb.z < level->bounds.z)
	{
		return 1;
	}
	if (aabb.x + aabb.w > level->bounds.x + level->bounds.w)
	{
		return 1;
	}
	if (aabb.y + aabb.h > level->bounds.y + level->bounds.h)
	{
		return 1;
	}
	if (aabb.z + aabb.d > level->bounds.z + level->bounds.d)
	{
		return 1;
	}

	return 0;
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