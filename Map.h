#include <vector>
#include <time.h>
#include <xmmintrin.h>
#include "time.h"



#ifndef MAP_H
#define MAP_H

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------
#define VISCOSITY 0.2
#define GRAVITY 9.81
#define PI 3.1415
#define MAXBALLS 511

//--------------------------------------------------------------------------------------
// Struct V3 -- 3 coordenadas 
//--------------------------------------------------------------------------------------
__declspec(align(16)) struct V3
{
	float x;
	float y;
	float z;
};
//--------------------------------------------------------------------------------------
// Struct Velocidad -- Velocidades de las Bolas
//--------------------------------------------------------------------------------------
struct Speed
{
	V3* speed;

	Speed(unsigned int n)
	{
		speed = (V3*)_aligned_malloc(sizeof(V3)*n, 16);
		//int j = sizeof(V3);
	}
};
//--------------------------------------------------------------------------------------
// Struct Impact -- Impacto bola
//--------------------------------------------------------------------------------------
struct Impact
{
	V3 position;
	V3 direction;
	int ballIndex;

	Impact()
	{
		position.x = 0;
		position.y = 0;
		position.z = 0;

		direction.x = 0;
		direction.y = 0;
		direction.z = 0;
	}
};


//--------------------------------------------------------------------------------------
// Struct Position -- Posiciones de las Bolas
//--------------------------------------------------------------------------------------
struct Position
{
	V3* pos;

	Position(unsigned int n)
	{
		pos = (V3*)_aligned_malloc(sizeof(V3)*n, 16);
	}
};


//--------------------------------------------------------------------------------------
// Struct Radio -- Radios de las Bolas
//--------------------------------------------------------------------------------------
struct Radio
{
	float* rad;

	Radio(unsigned int n)
	{
		rad = (float*)_aligned_malloc(sizeof(float)*n, 16);
	}
};

//--------------------------------------------------------------------------------------
// Struct Density -- Densidad de las Bolas
//--------------------------------------------------------------------------------------
struct Density
{
	float* den;

	Density(unsigned int n)
	{
		den = new float [n];
	}
};

//--------------------------------------------------------------------------------------
// Struct Physics -- 
//--------------------------------------------------------------------------------------
struct PhysicsBalls
{
	unsigned int ballInstances;
	
	Speed* velocities;
	Position* positions;
	Radio* radios;
	Density* densities;
	bool* standing;
	bool* visible;
	unsigned int colision[MAXBALLS][MAXBALLS];

	PhysicsBalls(unsigned int n)
	{
		ballInstances = n;
		standing = new bool[MAXBALLS];
		visible = new bool[MAXBALLS];
		velocities = new Speed(MAXBALLS);
		positions = new Position(MAXBALLS);
		radios = new Radio(MAXBALLS);
		densities = new Density(MAXBALLS);
	}
};



//--------------------------------------------------------------------------------------
// Map La clase principal para el modelo físico
// el método Simulate debería avanzar el modelo
// más adelante habría que pensar si es necesario separar en más clases
// esta clase no va a tener la responsabilidad de dibujar 
// sino que va a ser utilizada como modelo de datos y nada más
//--------------------------------------------------------------------------------------

class Map
{

public:

	// Para los test

	unsigned int apply_sse[1000];
	int applyCant;
	
	Map();
	Map(int size);

	bool resize(int n);
	
	bool addBall(V3 &position, V3 &velocity, float radio);
	bool simulatePhysicsBalls(float time, int effect, bool explosion);
	bool simulatePhysicsBalls(float time, int effect, bool explosion,V3 &postion, V3 &direction);

	V3* getPositions();
	
	V3* getSpeed();

	float* getRadios();
	
	PhysicsBalls* pballs;

	void shooted(int i, V3 &posImpact, V3 &dirImpact);

	float masaEsfera(int i);

//private:

	bool explosion;
	
	void generatePositions();
	void generateRadios();
	
	// Funcion para inicializar las densidades de cada bola
	void generateDensities();
	//void generateDensitiesSSE();

	void generateVelocities();
	void transformation();
	void getInBounds();
	void getInBounds_SSE();

	void everybodyMove();
	//void everybodyMove_SSE();

	float distancia (int i, int j);
	bool checkStatus();
	
	bool applyCollision();
	bool applyCollision_SSE();

	bool applyWorldPhysics();
	bool checkCollisions();
	bool checkBallsCollision(int i, int j); // ver si se va a utilizar o hay que hacer una optimización por búsqueda x espacio
	
	
	bool moveBalls();
	bool moveBalls_SSE();
	
	Impact* getBallShooted(V3 &position, V3 &direction);
	Impact* getBallShooted_SSE(V3 &position, V3 &direction);


	bool impactBall(Impact &impacto);

	bool colisiona(int i, float v);
	bool colisiona_SSE(int i, float v);
	void armarColsiones();

	float dotProduct(V3 &vii, V3 &vji);
	void scalarProduct(V3&vec1, float scalar);
	void resta(V3 &normal, V3&vii, V3&vji);
	void restaSSE(V3 &normal, V3&vii, V3&vji);
	void suma(V3 &normal, V3&vii, V3&vji);
	void crossProduct(V3 &normal, V3&vii, V3&vji);
	float normaVectorial(V3 &vector);
	void normalizar(V3 &vector);
	float normaCuad(V3& vec);
	int raySphere(V3 &p1,V3 &p2,V3 &sc, float r, float *mu1, float *mu2);
	int raySphereSSE(V3 &p1,V3 &dp,V3 &sc, float r, float *mu1, float *mu2);

};



#endif // MAP_H