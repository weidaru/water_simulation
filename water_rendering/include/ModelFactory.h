#include <string>

#ifndef MODEL_FACTORY_H_
#define  MODEL_FACTORY_H_

class Model;

class ModelFactory
{
public:
	static Model* CreateModel(const std::string& file_path, const std::string& ext);
};

#endif			//MODEL_FACTORY_H_