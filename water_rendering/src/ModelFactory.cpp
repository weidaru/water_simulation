#include "ModelFactory.h"

#include "ModelImpASC.h"
#include "ModelImpAI.h"


Model* ModelFactory::CreateModel(const std::string& file_path, const std::string& ext)
{
	if(ext.compare("ASC") == 0 || ext.compare("asc") == 0)
	{
		ModelImpASC* model = new ModelImpASC;
		model->ReadMesh(file_path);
		return model;
	}
	else
	{
		ModelImpAI* model = new ModelImpAI;
		model->ReadFile(file_path);
		return model;
	}
}


