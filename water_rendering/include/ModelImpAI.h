#ifndef MODEL_IMP_AI_H_
#define  MODEL_IMP_AI_H_

#include "Model.h"

#include <string>

class ModelImpAI  : public Model
{
public:
	typedef std::vector<Triangle* > TriangleVector;

public:
	ModelImpAI();
	virtual ~ModelImpAI();

	virtual const Triangle& GetData(int index) const;
	virtual int GetTriangleCount() const;

	int ReadFile(const std::string& file_path);

private:
	TriangleVector triangles_;
};

#endif				//MODEL_IMP_AI_H_