#include "DXUT.h"
#include "Map.h"

V3* Map::getPositions()
{
	return this->pballs->positions->pos;
}

V3* Map::getSpeed()
{
	return this->pballs->velocities->speed;
}

float* Map::getRadios()
{
	return this->pballs->radios->rad;
}

void Map::normalizar(V3 &vector)
{
	float norm = normaVectorial(vector);
	vector.x = vector.x / norm;
	vector.y = vector.y / norm;
	vector.z = vector.z / norm;
}

float Map::normaVectorial(V3 &vector)
{
	return (sqrt(pow(vector.x, 2)+ pow(vector.y, 2)+ pow(vector.z, 2)));
}

float Map::dotProduct(V3 &vec1, V3 &vec2)
{
	return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
}

void Map::scalarProduct(V3&vec1, float scalar)
{
	vec1.x *= scalar;
	vec1.y *= scalar;
	vec1.z *= scalar;
}

void Map::restaSSE(V3 &result,V3 &vec1,V3 &vec2)
{
	__asm
	{
		mov	esi, vec1
		mov	edi, vec2

		mov eax, result

		movupd xmm0, [esi]
		movupd xmm1, [edi]

		subpd xmm0, xmm1
		
		movupd [eax], xmm0

		movlpd xmm0, [esi + 16]
		movlpd xmm1, [edi + 16]

		subsd xmm0, xmm1

		movlpd [eax + 16], xmm0
	}
}

void Map::resta(V3 &result,V3 &vec1,V3 &vec2)
{	
	result.x = (vec1.x - vec2.x);
	result.y = (vec1.y - vec2.y);
	result.z = (vec1.z - vec2.z);
}

void Map::suma(V3 &result,V3 &vec1,V3 &vec2)
{
	result.x = vec1.x + vec2.x;
	result.y = vec1.y + vec2.y;
	result.z = vec1.z + vec2.z;
}

void Map::crossProduct(V3& res, V3& vii, V3& vji)
{
	res.x = (vii.y * vji.z) - (vii.z * vji.y);
	res.y = (vii.z * vji.x) - (vii.x * vji.z); 
	res.z = (vii.x * vji.y) - (vii.y * vji.x);
}

float Map::normaCuad(V3& vec)
{
	return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}
