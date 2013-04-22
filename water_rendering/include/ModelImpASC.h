#include "Model.h"

#include <vector>

#ifndef MODEL_IMP_ASC_H_
#define  MODEL_IMP_ASC_H_

class ModelImpASC : public Model
{
private:
	typedef std::vector<Triangle*> TriangleVector;

public:
	ModelImpASC();
	virtual ~ModelImpASC();

	virtual const Triangle& GetData(int index) const;
	virtual int GetTriangleCount() const;
	
	int ReadMesh(const std::string& file_path);

private:
	TriangleVector triangles_;
};

#endif			//MODEL_IMP_ASC_H_