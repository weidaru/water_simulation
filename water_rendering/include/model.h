#include "Gz.h"

#include <vector>

#ifndef MODEL_H_
#define  MODEL_H_

struct Triangle
{
	GzCoord vertices[3];			/* vertex position coordinates */ 
	GzCoord normals[3];		/* vertex normals */ 
	GzTextureIndex uvs[3];		/* vertex texture map indices */ 
};

/*
* Class to store model data
*/
class Model
{
public:
	typedef std::vector<Triangle* > TriangleVector;

public:
	Model();
	~Model();

	int ReadMesh(const std::string& file_path);
	const TriangleVector& GetData();

private:
	TriangleVector triangles_;
};

#endif		//MODEL_H_