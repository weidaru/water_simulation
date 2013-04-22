#include "Gz.h"

#include <vector>

#ifndef MODEL_H_
#define  MODEL_H_

struct Triangle
{
	GzCoord vertices[3];			// vertex position coordinates
	GzCoord normals[3];			// vertex normals
	GzTextureIndex uvs[3];		// vertex texture coordinates
};

/*
* Class to store model data
*/
class Model
{
public:
	virtual ~Model() { }

	virtual const Triangle& GetData(int index) const = 0;
	virtual int GetTriangleCount() const = 0;

};

#endif		//MODEL_H_