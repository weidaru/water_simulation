//////////////////////////////////////////////////////////////////////
// Author:		Yong CHEN
// Date:        12/05
// MeshTestbed - used for USC-ISE510
//////////////////////////////////////////////////////////////////////

#pragma once

class CTransMatrix  
{
public:
	CTransMatrix(void);
	virtual ~CTransMatrix(void) { }

	CTransMatrix(const CTransMatrix & rhs);
	CTransMatrix& operator=(CTransMatrix& rhs);
	CTransMatrix& operator*=(CTransMatrix& rhs);
	void Inverse();
	double * operator [](int i) { return (matrixData[i]); }
	void SetElementValue(int i, int j, double value) 
	{ 
		matrixData[i][j] = value; 
	};
	void GetTransformedVertex(float InVertex[3], float OutVertex[3]);
	void GetTransformedHVertex(float InVertex[4], float OutVertex[4]);
	void GetTransformedVector(float InVector[3], float OutVector[3]);
	void SetToIdentity();

protected:
	double matrixData[4][4];
	double scale[3];
};
