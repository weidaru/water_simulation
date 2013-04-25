#include <assert.h>
#include <math.h>

#include "Gz.h"
#include "utilities.h"

void SetIdentity(GzMatrix mat)
{
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			if(i == j)
				mat[i][j] = 1.0f;
			else
				mat[i][j] = 0.0f;
		}
	}
}

void VectorSubtract(const GzCoord& v0, const GzCoord& v1, GzCoord& result)
{
	result[0] = v0[0] - v1[0];
	result[1] = v0[1] - v1[1];
	result[2] = v0[2] - v1[2];
}

void VectorAdd(const GzCoord& v0, const GzCoord& v1, GzCoord& result)
{
	result[0] = v0[0] + v1[0];
	result[1] = v0[1] + v1[1];
	result[2] = v0[2] + v1[2];
}

float Length(const GzCoord& v)
{
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

float Dot(const GzCoord& v0, const GzCoord& v1)
{
	return (v0[0]*v1[0]+v0[1]*v1[1]+v0[2]*v1[2]);
}

void Scale(const GzCoord& v, float scale, GzCoord result)
{
	for(int i=0; i<3; i++)
		result[i] = v[i] * scale;
}

void Cross(const GzCoord& v0, const GzCoord& v1, GzCoord& result)
{
	result[0] = v0[1]*v1[2] - v0[2]*v1[1];
	result[1] = v0[2]*v1[0] - v0[0]*v1[2];
	result[2] = v0[0]*v1[1] - v0[1]*v1[0];
}

void MatrixCopy(const GzMatrix from, GzMatrix to)
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			to[i][j] = from[i][j];
}

void MatrixMultiply(const GzMatrix m0, const GzMatrix m1, GzMatrix result)
{
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			result[i][j] = 0.0f;
			for(int k=0; k<4; k++)
				result[i][j] += m0[i][k]*m1[k][j];
		}
	}
}

void MatrixMultiplyVector(const GzMatrix mat, const GzCoord& vec, GzCoord& result, bool is_direction)
{
	float extended_vec[4] = {vec[0], vec[1], vec[2], 1.0f};
	if(is_direction)
		extended_vec[3] = 0.0f;
	float extended_result[4] = {0.0f};
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			extended_result[i] += mat[i][j] * extended_vec[j];
	for(int i=0; i<3; i++)
	{
		if(extended_result[3] != 0.0f)
			result[i] = extended_result[i]/extended_result[3];
		else
			result[i] = extended_result[i];
	}


}

void Normalize(GzCoord& vec)
{
	float length = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
	if(length != 0)
	{
		for(int i=0; i<3; i++)
			vec[i] = vec[i]/length;
	}
}

inline void SwapLine(float mat[4][8], int l1, int l2)
{
	if(l1 == l2)
		return;
	for(int i=0; i<8; i++)
	{
		float temp = mat[l1][i];
		mat[l1][i] = mat[l2][i];
		mat[l2][i] = temp;
	}
}

inline void ScaleLine(float mat[4][8], int line, float scale)
{
	for(int i=0; i<8; i++)
		mat[line][i] *= scale;
}

inline void AddLine(float mat[4][8], int des, int source ,float scale)
{
	for(int i=0; i<8; i++)
		mat[des][i]  += mat[source][i]*scale;
}

bool MatrixInverse(const GzMatrix mat, GzMatrix result)
{
	//use Gauss Jordan elimination
	float augment[4][8];
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			augment[i][j] = mat[i][j];
			augment[i][j+4] = (i==j ? 1 : 0);
		}
	}

	for(int i=0; i< 4; i++)
	{	
		//Scale
		int pivot=i;
		while(augment[pivot][i] == 0 && pivot <4)
		{
			pivot++;
		}
		if(pivot == 4)
			return false;
		SwapLine(augment, i, pivot);
		ScaleLine(augment, i, 1.0f/augment[i][i]);

		assert(abs(augment[i][i] - 1.0f) < 1e-6);

		//Eliminate
		for(int j=0; j<4; j++)
		{
			if(j == i)
				continue;
			AddLine(augment, j, i, -augment[j][i]);
		}
	}
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			result[i][j] = augment[i][j+4];

	return true;
}

void MatrixTranspose(const GzMatrix mat, GzMatrix result)
{
	GzMatrix temp;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			temp[j][i] = mat[i][j];
	MatrixCopy(temp, result);
}

float Clamp(float v, float v_floor, float v_ceiling)
{
	assert(v_ceiling >= v_floor);
	float result = v > v_ceiling ? v_ceiling : (v<v_floor ? v_floor : v);
	return result;
}

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
	return(short)((int)(color * ((1 << 12) - 1)));
}

