#include "TransMatrix.h"

CTransMatrix::CTransMatrix(void)
{
	for (int i= 0; i < 4; i++)
	{
		for (int j= 0; j < 4; j++)
		{
			if (i == j)
				matrixData[i][j] = 1.0f;
			else
				matrixData[i][j] = 0.0f;
		}
	}
	scale[0] = 1.0f;
	scale[1] = 1.0f;
	scale[2] = 1.0f;
}

CTransMatrix::CTransMatrix(const CTransMatrix & src)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			matrixData[i][j] = src.matrixData[i][j];
		}
	}
	scale[0] = src.scale[0];
	scale[1] = src.scale[1];
	scale[2] = src.scale[2];
}

void CTransMatrix::SetToIdentity()
{
	for (int i= 0; i < 4; i++)
	{
		for (int j= 0; j < 4; j++)
		{
			if (i == j)
				matrixData[i][j] = 1.0f;
			else
				matrixData[i][j] = 0.0f;
		}
	}
	scale[0] = 1.0f;
	scale[1] = 1.0f;
	scale[2] = 1.0f;
}

CTransMatrix& CTransMatrix::operator=(CTransMatrix& rhs)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			matrixData[i][j] = rhs.matrixData[i][j];
		}
	}
	scale[0] = rhs.scale[0];
	scale[1] = rhs.scale[1];
	scale[2] = rhs.scale[2];
	return *this;
}

CTransMatrix& CTransMatrix::operator*=(CTransMatrix& rhs)
{
	int i, j, k;

	float tmp;
	float tmpMatrix[4][4];
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			tmpMatrix[i][j] = matrixData[i][j];
		}

	}

	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			tmp = 0.0;
			for (k=0; k<4; k++)
			{
				tmp += tmpMatrix[i][k] * rhs.matrixData[k][j];
			}
			matrixData[i][j] = tmp;	
		}
	}
	scale[0] = rhs.scale[0];
	scale[1] = rhs.scale[1];
	scale[2] = rhs.scale[2];
	return *this;
}

void CTransMatrix::GetTransformedVertex(float InVertex[3], float OutVertex[3])
{
	OutVertex[0] = (float) (matrixData[0][0] * InVertex[0] +
							matrixData[0][1] * InVertex[1] +
							matrixData[0][2] * InVertex[2] +
							matrixData[0][3] * 1.0);
	
	OutVertex[1] = (float) (matrixData[1][0] * InVertex[0] +
							matrixData[1][1] * InVertex[1] +
							matrixData[1][2] * InVertex[2] +
							matrixData[1][3] * 1.0);
	
	OutVertex[2] = (float) (matrixData[2][0] * InVertex[0] +
							matrixData[2][1] * InVertex[1] +
							matrixData[2][2] * InVertex[2] +
							matrixData[2][3] * 1.0);
}
	

void CTransMatrix::GetTransformedHVertex(float InVertex[4], float OutVertex[4])
{
	OutVertex[0] = (float) (matrixData[0][0] * InVertex[0] +
							matrixData[0][1] * InVertex[1] +
							matrixData[0][2] * InVertex[2] +
							matrixData[0][3] * InVertex[3]);
	OutVertex[1] = (float) (matrixData[1][0] * InVertex[0] +
							matrixData[1][1] * InVertex[1] +
							matrixData[1][2] * InVertex[2] +
							matrixData[1][3] * InVertex[3]);
	OutVertex[2] = (float) (matrixData[2][0] * InVertex[0] +
							matrixData[2][1] * InVertex[1] +
							matrixData[2][2] * InVertex[2] +
							matrixData[2][3] * InVertex[3]);
	OutVertex[3] = (float) (matrixData[3][0] * InVertex[0] +
							matrixData[3][1] * InVertex[1] +
							matrixData[3][2] * InVertex[2] +
							matrixData[3][3] * InVertex[3]);
}
	
void CTransMatrix::GetTransformedVector(float InVector[3], float OutVector[3])
{
	OutVector[0] = (float) (matrixData[0][0] * InVector[0] +
							matrixData[0][1] * InVector[1] +
							matrixData[0][2] * InVector[2] );
	
	OutVector[1] = (float) (matrixData[1][0] * InVector[0] +
							matrixData[1][1] * InVector[1] +
							matrixData[1][2] * InVector[2] );
	
	OutVector[2] = (float) (matrixData[2][0] * InVector[0] +
							matrixData[2][1] * InVector[1] +
							matrixData[2][2] * InVector[2] );
}

void CTransMatrix::Inverse()
{
	int i, j;

	float invMatrix[4][4];

    invMatrix[3][0] = 0.0;
	invMatrix[3][1] = 0.0;
    invMatrix[3][2] = 0.0;
    invMatrix[3][3] = 1.0;

	// the rotation part
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			invMatrix[i][j] = matrixData[j][i];
		}
	}

	// the translation part
	for (i = 0; i < 3; i++)
	{
		invMatrix[i][3] = -( invMatrix[i][0]*matrixData[0][3] 
							+ invMatrix[i][1]*matrixData[1][3] 
							+ invMatrix[i][2]*matrixData[2][3]);
	}

	// get the inverse
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			matrixData[i][j] = invMatrix[i][j];
		}
	}
}
