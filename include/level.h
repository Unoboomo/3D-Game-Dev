#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gfc_audio.h"

#include "model.h"

typedef struct
{
	Model* sky;
	GFC_TextLine skyFile;
	GFC_TextLine terrainFile;
	Model* terrain;
	GFC_Matrix4 terrainMatrix;
	GFC_Box bounds; //don't let anything leave this
	Mix_Music* backgroundMusic;
	GFC_TextLine musicFile;
	SJson* json; //keep the json around in case we need to reference it

}Level;

/**
* @brief allocates a new empty level
* @return NULL on error, or a blank level
*/
Level* level_new();

/**
* @brief free a previously created level after it is no longer needed
* @param *level a pointer to the entity to free
*/
void level_free(Level* level);

/**
* @brief load an entity based on provided filename that contains a json
* @param filename the filename of the json providing level information
* @return returns the level that was created
*/
Level* level_load_from_file(const char *filename);

/**
* @brief queue render an entity
* @param *level a pointer to the level to queue render
*/
void level_queue_render(Level* level);

/**
* @brief gets the current level
* @return the current level
*/
Level* level_get_the();

/**
* @brief check if the bounding box provided is clipping or outside the level bounds
* @param level the level to check the bounding box against
* @param aabb the axis aligned bounding box we check against the level
* @return 1 if the bounding box is outside the level bounds, 0 otherwise or on error
*/
int level_bounds_check(Level* level, GFC_Box aabb);

Uint8 level_line_test(Level* level, GFC_Edge3D edge, GFC_Vector3D* contact_point);

#endif 