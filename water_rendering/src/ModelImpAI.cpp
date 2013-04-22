#include "ModelImpAI.h"

#include <assimp/Importer.hpp>   
#include <assimp/scene.h> 
#include <assimp/postprocess.h>  

#include <assert.h>

ModelImpAI::ModelImpAI()
{

}

ModelImpAI::~ModelImpAI()
{
	for(TriangleVector::iterator it = triangles_.begin(); it != triangles_.end(); it++)
		delete *it;
}

const Triangle& ModelImpAI::GetData(int index) const
{
	return *triangles_[index];
}

int ModelImpAI::GetTriangleCount() const
{
	return triangles_.size();
}

int ModelImpAI::ReadFile(const std::string& file_path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( file_path, 
		aiProcess_CalcTangentSpace       | 
		aiProcess_Triangulate            |
		aiProcess_JoinIdenticalVertices
		);

	assert(scene->mNumMeshes == 1);			//only deals with one mesh per file(scene) for now
	aiMesh* mesh = scene->mMeshes[0];
	assert(mesh->HasNormals());						//assert for normals
	assert(mesh->HasTextureCoords(0));				//assert for texture coordinates
	for(int i=0; i<mesh->mNumFaces; i++)
	{
		const aiFace* face = &mesh->mFaces[i];
		assert(face->mNumIndices == 3);
		Triangle* t = new Triangle;
		for(int j=0; j<face->mNumIndices; j++)
		{
			int index = face->mIndices[j];
			
			t->vertices[j][0] =  mesh->mVertices[index].x;
			t->vertices[j][1] =  mesh->mVertices[index].y;
			t->vertices[j][2] =  mesh->mVertices[index].z;

			t->normals[j][0] =  mesh->mNormals[index].x;
			t->normals[j][1] =  mesh->mNormals[index].y;
			t->normals[j][2] =  mesh->mNormals[index].z;

			t->uvs[j][0] = mesh->mTextureCoords[0][index].x;
			t->uvs[j][1] = mesh->mTextureCoords[0][index].y;
		}

		triangles_.push_back(t);
	}

	return 1;
}