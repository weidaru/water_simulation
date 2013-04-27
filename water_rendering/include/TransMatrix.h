#ifndef TRANS_MATRIX_H_
#define  TRANS_MATRIX_H_

class CTransMatrix  
{
public:
	CTransMatrix(void);
	virtual ~CTransMatrix(void) { }

	CTransMatrix(const CTransMatrix & rhs);
	CTransMatrix& operator=(CTransMatrix& rhs);
	CTransMatrix& operator*=(CTransMatrix& rhs);
	void Inverse();
	float * operator [](int i) { return (matrixData[i]); }

	void SetElementValue(int i, int j, float value) 
	{ 
		matrixData[i][j] = value; 
	};
	void GetTransformedVertex(float InVertex[3], float OutVertex[3]);
	void GetTransformedHVertex(float InVertex[4], float OutVertex[4]);
	void GetTransformedVector(float InVector[3], float OutVector[3]);
	void SetToIdentity();

public:
	float matrixData[4][4];
	float scale[3];
};


#endif					//TRANS_MATRIX_H_