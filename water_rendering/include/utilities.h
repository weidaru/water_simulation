#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "Gz.h"

void SetIdentity(GzMatrix mat);
void VectorSubtract(const GzCoord& v0, const GzCoord& v1, GzCoord& result);
void VectorAdd(const GzCoord& v0, const GzCoord& v1, GzCoord& result);
float Length(const GzCoord& v);
void Scale(const GzCoord& v, float scale, GzCoord result);
float Dot(const GzCoord& v0, const GzCoord& v1);
void Cross(const GzCoord& v0, const GzCoord& v1, GzCoord& result);
void MatrixCopy(const GzMatrix from, GzMatrix to);
void MatrixMultiply(const GzMatrix m0, const GzMatrix m1, GzMatrix result);
void MatrixMultiplyVector(const GzMatrix mat, const GzCoord& vec, GzCoord& result, bool is_direction = false);
void Normalize(GzCoord& vec);
bool MatrixInverse(const GzMatrix mat, GzMatrix result);
void MatrixTranspose(const GzMatrix mat, GzMatrix result);
float Clamp(float v, float v_floor, float v_ceiling);
short	ctoi(float color);

#endif		//UTILITIES_H_