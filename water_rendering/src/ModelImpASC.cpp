#include "ModelImpASC.h"

#include <stdio.h>

ModelImpASC::ModelImpASC()
{

}

ModelImpASC::~ModelImpASC()
{
	for(TriangleVector::iterator it = triangles_.begin(); it != triangles_.end(); it++)
		delete *it;
}

const Triangle& ModelImpASC::GetData(int index) const
{
	return *triangles_[index];
}

int ModelImpASC::GetTriangleCount() const
{
	return (int)triangles_.size();
}

int ModelImpASC::ReadMesh(const std::string& file_path)
{
	FILE *infile;
	char dummy[128];
	if( (infile  = fopen( file_path.c_str() , "r" )) == NULL )
	{
		printf("Error open file.\n");
		return 0;
	}

	while( fscanf(infile, "%s", dummy) == 1) 
	{
		Triangle *t = new Triangle;

		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(t->vertices[0][0]), &(t->vertices[0][1]),  
			&(t->vertices[0][2]), 
			&(t->normals[0][0]), &(t->normals[0][1]), 	
			&(t->normals[0][2]), 
			&(t->uvs[0][0]), &(t->uvs[0][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(t->vertices[1][0]), &(t->vertices[1][1]), 	
			&(t->vertices[1][2]), 
			&(t->normals[1][0]), &(t->normals[1][1]), 	
			&(t->normals[1][2]), 
			&(t->uvs[1][0]), &(t->uvs[1][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(t->vertices[2][0]), &(t->vertices[2][1]), 	
			&(t->vertices[2][2]), 
			&(t->normals[2][0]), &(t->normals[2][1]), 	
			&(t->normals[2][2]), 
			&(t->uvs[2][0]), &(t->uvs[2][1]) ); 
		triangles_.push_back(t);
	}
	return 1;
}

