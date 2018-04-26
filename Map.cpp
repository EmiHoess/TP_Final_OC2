#include "DXUT.h" // no sé xq me lo pedía para que compile
#include "Map.h"



//--------------------------------------------------------------------------------------
// Funcion para generar numeros aleatorios
//  
//
//--------------------------------------------------------------------------------------
void generarVectorAleatorio(int n, int p, V3* v, int a)
{   
    srand((unsigned)time(0)*p);   
    int i; 
	int j;
	float pos_real;
    for(i = 0; i < n; i++)
	{
        for (int h = 0; h < n; h++)
		{
			j += rand() * 200 / (float)RAND_MAX + 1.0;
			srand((unsigned)time(0)*p);
		}
		srand((unsigned)time(0)*p);
		j /= n;
		switch (a)
		{
		case 0:
			v[i].x = j;
			break;
		case 1:
			v[i].y = j;
			break;
		case 2:
			v[i].z = j;
			break;
		}
		
    } 
}

void generarVectorAleatorioVel(int n, int p, V3* v, int a)
{   
    srand((unsigned)time(0)*p);   
    int i; 
	int j;
	float pos_real;
    for(i = 0; i < n; i++)
	{
        //j = static_cast<int>((float)rand()*(float)(vec.size()) / ((float)RAND_MAX+1.0));
		j = rand() * 2 / (float)RAND_MAX + 1.0;
		srand((unsigned)time(0)*p);

		switch (a)
		{
		case 0:
			v[i].x = j;
			break;
		case 1:
			v[i].y = j;
			break;
		case 2:
			v[i].z = j;
			break;
		}

		if ( abs(j) < 0.01 ) *((float*)(v + i*12 + a*4)) += 0.1;
    }  
}

void rangoY(V3* vec)
{
	for (int i = 0; i < MAXBALLS; i++)
	{
		vec[i].y = vec[i].y + 300.0;
	}
}

void rangoY_SSE(V3* vec)
{
	float* trescientos = new float;
	*trescientos = 300.0;
	__asm
	{
		MOV EBX, trescientos
		MOV ECX, MAXBALLS
		SHR ECX, 2
		// Divido ECX por 4 para usarlo como contador (proceso de a 4 elementos)
		MOV ESI, vec

		MOVSS XMM0, [EBX]
		PSHUFD XMM0, XMM0, 00000000b
		// Replico 300 en todo el registro
		
seguirSumando:
		MOVUPS XMM1, XMM0
		ADDPS XMM1, [ESI]
		MOVUPS [ESI], XMM1
		DEC ECX
		JNZ seguirSumando
		
	}

	// A estos códigos se les podría agregar el uso de Caché
}


//--------------------------------------------------------------------------------------
// Constructor por defecto
//--------------------------------------------------------------------------------------
Map::Map()
{}

//--------------------------------------------------------------------------------------
// Constructor a partir de una cantidad de bolas inicial
//  
//
//--------------------------------------------------------------------------------------
Map::Map(int size)
{
	this->pballs = new PhysicsBalls(size);
	generatePositions();
	generateRadios();
	generateVelocities();
	everybodyMove();

	applyCant = 0;
}


bool Map::resize(int n)
{
	this->pballs->ballInstances = n;
	generatePositions();
	generateRadios();
	generateVelocities();
	everybodyMove();

	return true;
}

void Map::everybodyMove()
{
	for (int i = 0; i < pballs->ballInstances; i++)
	{
		pballs->standing[i] = false;
		pballs->visible[i] = true;
	}
}
/*
void Map::everybodyMove_SSE()
{
	unsigned int cantBolas = pballs->ballInstances;
	bool* standingBalls = pballs->standing;
	bool* visibleBalls = pballs->visible;
	__asm
	{
		XOR EDX, EDX
		MOV ECX, cantBolas
		MOV ESI, standingBalls
		MOV EDI, visibleBalls

	}
}*/

void Map::generatePositions()
{
	srand(7);
	//generarVectorAleatorio(MAXBALLS, 73, pballs->positions->pos, 0);
	generarVectorAleatorio(MAXBALLS, 9501, pballs->positions->pos, 1);
	rangoY(pballs->positions->pos);
	//generarVectorAleatorio(MAXBALLS, 981543, pballs->positions->pos, 2);


	

	int count = 1;
	for (int z = 0; z < 5; z++)
	{
		pballs->positions->pos[z].x = -70.0f;
		pballs->positions->pos[z].z = -70.0f + z*30;

		for (int x = 0; x < 5; x++)
		{	
			if (count == pballs->ballInstances) return;
			pballs->positions->pos[count].x = pballs->positions->pos[count-1].x + 30;
			pballs->positions->pos[count].z = pballs->positions->pos[count-1].z;
			//pballs->positions->pos[count].y = 100.0f;
			count++;
		}
	}

	/*pballs->positions->posX[0] = -50.0f;
	pballs->positions->posY[0] = 50.0f;
	pballs->positions->posZ[0] = -50.0f;

	pballs->positions->posX[1] = 50.0f;
	pballs->positions->posY[1] = 50.0f;
	pballs->positions->posZ[1] = -50.0f;

	pballs->positions->posX[2] = -50.0f;
	pballs->positions->posY[2] = 50.0f;
	pballs->positions->posZ[2] = 50.0f;

	pballs->positions->posX[3] = 50.0f;
	pballs->positions->posY[3] = 50.0f;
	pballs->positions->posZ[3] = 50.0f;*/
}

void Map::generateRadios()
{
	srand(53);
	unsigned int anterior = 1;
	for (int i = 0; i < pballs->ballInstances; i++)
	{
		/*srand(53*i*anterior);
		anterior = rand();
		pballs->radios->rad[i] = (float)((anterior % 15)+1);*/
		pballs->radios->rad[i] = 15.0f;
	}

	//pballs->radios->rad[1] = 15.0f;
	//pballs->radios->rad[0] = 15.0f;
	/*pballs->radios->rad[1] = 1.0f;
	pballs->radios->rad[3] = 1.0f;
	pballs->radios->rad[2] = 10.0f;*/
}

void Map::generateDensities()
{
	for (int i = 0; i < pballs->ballInstances; i++)
	{
		pballs->densities->den[i] = 10.0f;
	}
}
/*
void Map::generateDensitiesSSE()
{
	float diez = 10.0f;
	float* densidades = pballs->densities->den;
	unsigned int cantidad = pballs->ballInstances;
	__asm
	{
		MOVD XMM0, diez
		PSHUFD XMM0, XMM0, 00000000b

		MOV ESI, densidades;
		MOV ECX, cantidad
		MOV EDX, 0
inicializar:
		MOVUPD [ESI + EDX*8], XMM0
		INC EDX
		INC EDX
		CMP EDX, ECX
		JL inicializar

	}
}*/

void Map::generateVelocities()
{
	srand(17);
	generarVectorAleatorioVel(MAXBALLS, 101, pballs->velocities->speed, 0);
	generarVectorAleatorioVel(MAXBALLS, 501, pballs->velocities->speed, 1);
	generarVectorAleatorioVel(MAXBALLS, 1543, pballs->velocities->speed, 2);
}

//--------------------------------------------------------------------------------------
// Función principal -- ver de renombrar xq hace bastante más
//  
//
//--------------------------------------------------------------------------------------
bool Map::checkCollisions()
{
	//unsigned int iniciosse = GetPentiumTimer();
	applyCollision_SSE();
	//applyCollision();
	//unsigned int finsse = GetPentiumTimer();

	//unsigned int tiemposse = finsse - iniciosse;

	
	return true;

}
//--------------------------------------------------------------------------------------
// Toma 2 indices y si están en contacto recupera sus posiciones
// y calcula las nuevas velocidades (no aplica física del mundo)
//
//--------------------------------------------------------------------------------------
bool Map::checkBallsCollision(int i, int j)
{
	// Deprecated
	float radioi = getRadios()[i];
	float radioj = getRadios()[j];

	if (getPositions()[i].x+radioi >= getPositions()[j].x-radioj &&
		getPositions()[i].y+radioi >= getPositions()[j].y-radioj &&
		getPositions()[i].z+radioi >= getPositions()[j].z-radioj)
	{
		return (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j]);
	}

	if (getPositions()[i].x-radioi <= getPositions()[j].x+radioj &&
		getPositions()[i].y-radioi <= getPositions()[j].y+radioj &&
		getPositions()[i].z-radioi <= getPositions()[j].z+radioj)
	{
		return (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j]);
	}

	return false;
}

//--------------------------------------------------------------------------------------
// Función para chequear que todas las pelotitas esten dentro de los limites del cuadrado
//  
//
//--------------------------------------------------------------------------------------
void Map::getInBounds()
{
	V3* pos = getPositions();

	V3 actualPos;
	float actualRadio;

	for (int i = 0; i < pballs->ballInstances; i++)
	{
		actualPos = pos[i];
		actualRadio = getRadios()[i];

		if (actualPos.x > 100 - actualRadio)
		{
			actualPos.x = 100 - actualRadio;
			if(getSpeed()[i].x>0){
				getSpeed()[i].x = -getSpeed()[i].x;
			}
		}
		if (actualPos.x < (-100) + actualRadio)
		{
			actualPos.x = -100 + actualRadio;
			if(getSpeed()[i].x<0){
				getSpeed()[i].x = -getSpeed()[i].x;
			}
		}
		
		if (actualPos.z > 100 - actualRadio)
		{
			actualPos.z = 100 - actualRadio;
			if(getSpeed()[i].z>0){
				getSpeed()[i].z = -getSpeed()[i].z;
			}
		}
		if (actualPos.z < (-100) + actualRadio)
		{
			actualPos.z = -100 + actualRadio;
			if(getSpeed()[i].z<0){
				getSpeed()[i].z = -getSpeed()[i].z;
			}
		}

		if (actualPos.y > 100 - actualRadio) 
		{
			actualPos.y = 100 - actualRadio;
			if(getSpeed()[i].y>0){
				getSpeed()[i].y = -getSpeed()[i].y;
			}
		}
		
		if (actualPos.y < actualRadio )
		{
			actualPos.y = actualRadio;
			getSpeed()[i].y = -getSpeed()[i].y;
			if (fabs(getSpeed()[i].y) < 0.01)
			{
				pballs->standing[i] = true;
				getSpeed()[i].y = 0.0f;
			}
		}

		pos[i].x = actualPos.x;
		pos[i].y = actualPos.y;
		pos[i].z = actualPos.z;
	}
}


void Map::getInBounds_SSE()
{
	V3* velocidades, *posiciones;
	velocidades = pballs->velocities->speed;
	posiciones = pballs->positions->pos;

	unsigned int cantidad = pballs->ballInstances;
	
	float *radios;
	radios = pballs->radios->rad;

	float *cien = new float, *menosCien = new float;
	*cien = 100.0f;
	*menosCien = -100.0f;


	__asm
	{
		MOV		ESI, posiciones
		MOV		EDI, velocidades
		MOV		EAX, radios

		XOR		ECX, ECX
		MOV		EDX, cantidad
		
seguir:
		MOVAPS	XMM0, [ESI + ECX*8] // Cargo posiciones
		//prefetchnta	[esi+0x30]
		MOVAPS	XMM1, [EDI + ECX*8] // Cargo velocidades
		//prefetchnta	[edi+0x30]
		
		MOVSS	XMM2, [EAX + ECX*2] // Cargo radios
		//prefetchnta	[eax+0x30]

		PSHUFD	XMM2, XMM2, 00000000b
		
		PUSH	EDX
		MOV		EDX, cien
		MOVSS	XMM3, [EDX]
		POP		EDX
		PSHUFD	XMM3, XMM3, 00000000b
		
		MOVUPS	XMM4, XMM3
		SUBPS	XMM4, XMM2 // 100 - actualRadio
		
		MOVUPS	XMM5, XMM0
		//MOVUPS	XMM6, XMM0
		CMPNLEPS XMM0, XMM4
		

		MOVUPS	XMM7, XMM0 // Guardo una copia

		ANDPS	XMM4, XMM0
		ANDNPS	XMM0, XMM5
		
		ORPS	XMM0, XMM4 // Este es el valor de actualPos
		
		XORPS	XMM4, XMM4
		XORPS	XMM5, XMM5
		SUBPS	XMM5, XMM1	// En XMM5 tengo -velocidades
		
		CMPLTPS	XMM4, XMM1	// Veo si las velocidades son mayores a cero
		
		ANDPS	XMM4, XMM7	// Hago el if (actualPos > 100-actualRadio && velocidad > 0)
		
		ANDPS	XMM5, XMM4
		ANDNPS	XMM4, XMM1
		
		ORPS	XMM5, XMM4	// En XMM5 tengo las velocidades finales
		MOVUPS	XMM1, XMM5


		// Hasta aca viene bien
		
		XORPS	XMM4, XMM4
		SUBPS	XMM4, XMM3	// En XMM4 tengo -100.0
		
		PSLLDQ	XMM4, 4
		PSRLDQ	XMM4, 4
		PSHUFD	XMM4, XMM4, 11101100b
		ADDPS	XMM4, XMM2		//Cargo -100.0 + radios o radio solo para el caso de la coord 'y'
		
		MOVUPS	XMM5, XMM0
		CMPLTPS	XMM5, XMM4
		MOVUPS	XMM7, XMM5
		
		ANDPS	XMM4, XMM5		// Si cumple el if entonces guardo -100.0+radio
		ANDNPS	XMM5, XMM0
		
		ORPS	XMM4, XMM5		// Tengo el valor de actualPos
		MOVUPS	XMM0, XMM4
		
		XORPS	XMM6, XMM6
		CMPNLEPS XMM6, XMM1
		
		XORPS	XMM5, XMM5
		SUBPS	XMM5, XMM1			// Tengo -velocidades
		
		ANDPS	XMM6, XMM7			// Veo que cumpla los dos if's
		
		ANDPS	XMM5, XMM6
		ANDNPS	XMM6, XMM1
		

		ORPS	XMM5, XMM6			// En XMM1 tengo actualizadas las velocidades
		MOVUPS	XMM1, XMM5
		
		MOVAPS	[ESI + ECX*8], XMM0 // Cargo posiciones
		MOVAPS	[EDI + ECX*8], XMM1

		INC ECX
		INC ECX
		DEC EDX
		JNZ	seguir
	}
}

//--------------------------------------------------------------------------------------
// Se asegura que la pelota i esté en una posición válida
//  llama a applyWorldPhysics
//
//--------------------------------------------------------------------------------------
bool Map::checkStatus()
{
	//applyWorldPhysics();
	return true;
}

//--------------------------------------------------------------------------------------
// Se encarga de aplicar viscosidad/rozamiento y gravedad
//  
//
//--------------------------------------------------------------------------------------
bool Map::applyWorldPhysics()
{
	for (int i = 0; i < pballs->ballInstances; i++)
	{
		if (pballs->visible[i])
		{
			//version simple para aplicar viscosity debería aplicarse al módulo de la velocidad en realidad
			getSpeed()[i].z *= (100 - VISCOSITY)/100;
			getSpeed()[i].y *= (100 - VISCOSITY)/100;
			getSpeed()[i].z *= (100 - VISCOSITY)/100;
			//estos valores hay que reajustarlos una vez definido el nro de frames que vamos a permitir
			//y la velocidad que le vamos a dejar a las pelotitas
			//if (getSpeedX()[i] < 0.05) getSpeedX()[i] = 0;
			
			//if (getSpeed()[i].y < 0.05 && getPositions()[i].y == getRadios()[i] )
			{
			//	getSpeed()[i].y = 0;
			}
			//else	if (!colisiona_SSE(i, (float)GRAVITY/5000) && !pballs->standing[i]) getSpeed()[i].y -= GRAVITY/5000;
		}
	}
	
	return true;
}


bool Map::colisiona_SSE(int i, float v)
{
	float* radios = pballs->radios->rad;
	V3* posiciones = pballs->positions->pos;

	unsigned int cantidad = pballs->ballInstances;

	__asm
	{
		MOV ESI, posiciones
		MOV EDI, radios

		MOV EDX, i
		SHL	EDX, 1

		MOV EAX, v
		XORPS	XMM7, XMM7
		MOVSS	XMM7, EAX
		PSLLDQ	XMM7, 4

		MOVUPS	XMM0, [ESI + EDX*8]	// Cargo la bola contra la que voy a probar en XMM0
		MOVUPS	XMM1, [EDI + ECX*2]		// Cargo el radio de la bola i-ésima
		PSHUFD	XMM1, XMM2, 00000000b
		
		XOR	ECX, ECX
		MOV EDX, cantidad

seguir:
		MOVUPS	XMM2, [ESI + ECX*8]		// Cargo la bola jesima
		MOVUPS	XMM3, [EDI + ECX*2]		// Cargo el radio de la bola j-ésima
		PSHUFD	XMM3, XMM3, 00000000b
		

		MOVUPS	XMM4, XMM0
		SUBPS	XMM4, XMM2
		MULPS	XMM4, XMM4
		PSLLDQ	XMM4, 4
		PSRLDQ	XMM4, 4
		HADDPS	XMM4, XMM4
		HADDPS	XMM4, XMM4			// Calculo distancia(i,j)
		
		MOVUPS	XMM5, XMM1
		ADDPS	XMM5, XMM3


		MOVUPS	XMM6, XMM0
		CMPNLEPS XMM6, XMM2		// Comparo a ver si Yi > Yj

		CMPLEPS	XMM4, XMM5		// Comparo si la distancia es menor o igual que los radios

		ANDPS	XMM4, XMM6		// Las dos cosas anteriores
		
		MOVUPS	XMM6, XMM4

		PSLLDQ	XMM4, 4
		PSRLDQ	XMM4, 4
		
		

		PSHUFD	XMM4, XMM4, 11110111;	 // Dejo todo en cero salvo la y
	}

	for (int j = 0; j < this->pballs->ballInstances; j++)
	{
		if (j != i)
		{
			if (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j] && getPositions()[i].y > getPositions()[j].y)
			{
				getPositions()[i].y -= v;
				if (distancia(i,j) < this->pballs->radios->rad[i] + this->pballs->radios->rad[j])
				{
					getPositions()[i].y += v;
					return true;
				}
				getPositions()[i].y += v;
			}
		}
	}
	return false;
}

bool Map::colisiona(int i, float v)
{
	for (int j = 0; j < this->pballs->ballInstances; j++)
	{
		if (j != i)
		{
			if (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j] && getPositions()[i].y > getPositions()[j].y)
			{
				getPositions()[i].y -= v;
				if (distancia(i,j) < this->pballs->radios->rad[i] + this->pballs->radios->rad[j])
				{
					getPositions()[i].y += v;
					return true;
				}
				getPositions()[i].y += v;
			}
		}
	}
	return false;
}


//--------------------------------------------------------------------------------------
// Calcula la distancia entre 2 elementos
//  
//
//--------------------------------------------------------------------------------------
/*float Map::distancia(int i, int j)
{
	float ix = getPositions()[i].x, iy = getPositions()[i].y, iz = getPositions()[i].z;
	float jx = getPositions()[j].x, jy = getPositions()[j].y, jz = getPositions()[j].z;
	float xf = (ix-jx)*(ix-jx);
	float yf = (iy-jy)*(iy-jy);
	float zf = (iz-jz)*(iz-jz);
	float distancia = sqrt(xf+yf+zf);
	
	return distancia;
}*/

float Map::distancia(int i, int j)
{
	return 
		sqrt(pow(this->getPositions()[i].x - this->getPositions()[j].x,2) + 
		pow(this->getPositions()[i].y - this->getPositions()[j].y,2) + 
		pow(this->getPositions()[i].z - this->getPositions()[j].z,2)); 
}

float Map::masaEsfera(int i)
{
	return (4.0f/3.0f * PI * getRadios()[i] * 1.0f);
}

void Map::armarColsiones()
{
	int cantidad = 0, indiceFila = 0;
	for (int i = 0; i < this->pballs->ballInstances; i++)
	{
		indiceFila = 1;
		cantidad = 0;
		for (int j = i + 1; j < this->pballs->ballInstances; j++)
		{
			if (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j])
			{
				pballs->colision[i][indiceFila] = j;
				cantidad++;
			}
		}
		pballs->colision[i][0] = cantidad;
	}
}

//--------------------------------------------------------------------------------------
// Función que define la posición del par en el instante
//  sabiendo que colisionaron y que tienen que haberse desplazado
//  una cantidad "perdida" por el retroceso del choque
//--------------------------------------------------------------------------------------
bool Map::applyCollision()
{
	for (int i = 0; i < this->pballs->ballInstances; i++)
	{
		if (pballs->visible[i]) // En SSE no va a ser necesario chequear si estan o no visibles para hacer los calculos
		{
			for (int j = i + 1; j < this->pballs->ballInstances; j++)
			{
				if (pballs->visible[j])
				{
					if (true)
					{
						V3 vii, vji;

						V3 normal;

						V3 resultado;
						
						// Velocidades iniciales de 
						vii.x = getSpeed()[i].x;
						vii.y = getSpeed()[i].y;
						vii.z = getSpeed()[i].z;

						vji.x = getSpeed()[j].x;
						vji.y = getSpeed()[j].y;
						vji.z = getSpeed()[j].z;
						

						// Inutil llamada a la funcion restaSSE, solo para ver si anda bien...
						//restaSSE(resultado, vii, vji);
						
						float mi = masaEsfera(i);
						float mj = masaEsfera(j);

						float masas = (1.0f / mj + 1.0f / mi);

						normal.x = getPositions()[i].x - getPositions()[j].x;
						normal.y = getPositions()[i].y - getPositions()[j].y;
						normal.z = getPositions()[i].z - getPositions()[j].z;


						normalizar(normal);


						float op = (2.0f * (dotProduct(vii, normal) - dotProduct(vji, normal))) / (mj + mi);

						float choque = dotProduct(normal, vji) - dotProduct(normal, vii);


						float promvii = (vii.x + vii.y + vii.z)/3.0f;
						float promvij = (vji.x + vji.y + vji.z)/3.0f;


						if (choque > 0.0f)
						{
							getSpeed()[i].x = vii.x - op * mj * normal.x;
							getSpeed()[i].y = vii.y - op * mj * normal.y;
							getSpeed()[i].z = vii.z - op * mj * normal.z;

							getSpeed()[j].x = vji.x + op * mi * normal.x;
							getSpeed()[j].y = vji.y + op * mi * normal.y;
							getSpeed()[j].z = vji.z + op * mi * normal.z;
						}

						/*if (fabs(op) < 0.01 && promvii < 0.001 && promvij < 0.001 && ( pballs->standing[i] || pballs->standing[j] ) )
						{
							getSpeed()[i].x = 0.0f;
							getSpeed()[i].y = 0.0f;
							getSpeed()[i].z = 0.0f;

							getSpeed()[j].x = 0.0f;
							getSpeed()[j].y = 0.0f;
							getSpeed()[j].z = 0.0f;

							pballs->standing[i] = true;
							pballs->standing[j] = true;
						}else
						{
							pballs->standing[i] = false;
							pballs->standing[j] = false;
						}*/
					}
				}
			}
		}
	}
	
	return true;
}

bool Map::applyCollision_SSE()
{
	V3 *velocidades = pballs->velocities->speed;
	V3 *posiciones = pballs->positions->pos;
	float *radios = pballs->radios->rad;

	float* pi = new float, *cuatro = new float, *cota = new float;

	*cuatro = 1.3333333333333333333f;
	*pi = 3.14159265358979323846f;
	*cota = 0.0001;

	unsigned int *ebxtemp;

	int cantidad = this->pballs->ballInstances;

	__m128 test5; 

	__asm
	{
		MOV ebxtemp, EBX

		MOV	ESI, velocidades
		MOV	EDI, posiciones
		MOV	EAX, radios

		MOV	ECX, 0
		MOV	EDX, 0

		
seguirAun:
		MOVAPS	XMM0, [ESI + ECX*8]
		prefetchnta	[esi+0x30]
		MOV		EDX, ECX
		
seguir:
		INC		EDX
		INC		EDX
		
		MOVAPS	XMM2, [EDI + ECX*8]
		prefetchnta	[edi+0x30]
		
		MOVAPS	XMM1, [ESI + EDX*8]					//	Cargo las velocidades Vii, Vij
		
		MOVAPS	XMM3, [EDI + EDX*8]					//	Cargo las posiciones para calcular la normal

		SUBPS	XMM2, XMM3					
		MOVAPS	XMM3, XMM2							// Calculo en XMM3 la norma, para normalizar el vector

		MULPS	XMM3, XMM3
		PSLLDQ	XMM3, 4
		PSRLDQ	XMM3, 4
		HADDPS	XMM3, XMM3
		HADDPS	XMM3, XMM3

		MOVAPS	XMM7, XMM3
		

		MOVSS	XMM5, [EAX + ECX*2]
		prefetchnta	[eax+0x30]
		
		PSHUFD	XMM5, XMM5, 00000000b
			
			
		MOVSS	XMM6, [EAX + EDX*2]
		PSHUFD	XMM6, XMM6, 00000000b

		// En XMM7 tengo calculada la distancia al cuadrado, voy a compararla con la suma de radios al cuadrado
		MOVAPS	XMM4, XMM5
		ADDPS	XMM4, XMM6
		MULPS	XMM4, XMM4							// En XMM4 tengo la suma de radios al cuadrado

		CMPLEPS	XMM7, XMM4							// Tengo el valor de verdad de si la distancia es menor al cuadrado de la suma de radios
		PSHUFD	XMM7, XMM7, 00000000b					

		RSQRTPS	XMM3, XMM3
		MULPS	XMM2, XMM3							// Tengo el vector norma, normalizado en XMM2
		

		
		MOV		EBX, pi
		MOVSS	XMM3, [EBX]
		
		PSHUFD	XMM3, XMM3, 00000000b
		
		MULPS	XMM5, XMM3
		MULPS	XMM6, XMM3
		
		MOV		EBX, cuatro
		MOVSS	XMM3, [EBX]
		
		PSHUFD	XMM3, XMM3, 00000000b
		
		MULPS	XMM5, XMM3
		MULPS	XMM6, XMM3						// Todo esto para calcular la masa de las pelotitas


		MOVAPS	XMM3, XMM0
		MOVAPS	XMM4, XMM1
		
		
		MULPS	XMM3, XMM2
		MULPS	XMM4, XMM2
		
		PSLLDQ	XMM3, 4
		PSRLDQ	XMM3, 4
		HADDPS	XMM3, XMM3
		HADDPS	XMM3, XMM3						// XMM3 = dotProduct(normal, vii)
		MOVUPS	test5, XMM3

		PSLLDQ	XMM4, 4
		PSRLDQ	XMM4, 4
		HADDPS	XMM4, XMM4
		HADDPS	XMM4, XMM4						// XMM4 = dotProduct(normal, vij)

		
		
		SUBPS	XMM4, XMM3						// Calculo dot2 - dot1					
		XORPS	XMM3, XMM3						// Calculo choque = dot1 - dot2
		
		CMPLTPS	XMM3, XMM4
		ANDPS	XMM3, XMM7

		XORPS	XMM7, XMM7
		SUBPS	XMM7, XMM4						// Calculo dot2 - dot1

		MOVAPS	XMM4, XMM3

		MOVAPS	XMM3, XMM6
		ADDPS	XMM3, XMM5						// Calculo (mi + mj)

		ADDPS	XMM7, XMM7						// Calculo 2 * (dot2 - dot1)
		DIVPS	XMM7, XMM3						// Calculo op
		
		MULPS	XMM5, XMM2
		MULPS	XMM6, XMM2

		MULPS	XMM5, XMM7						// Calculo op * mj * normal
		MULPS	XMM6, XMM7						// Calculo op * mi * normal
		

		
		
		MOVAPS	XMM3, XMM0
		SUBPS	XMM3, XMM5
		MOVAPS	XMM5, XMM1
		ADDPS	XMM5, XMM6						// XMM3 y XMM5 tienen los valores nuevos de las velocidades
		
		
		MOVAPS	XMM6, XMM4
		ANDPS	XMM3, XMM6						// XMM0 = if (choque > 0.0f && colisiona) {todo el resto}
		ANDPS	XMM5, XMM6
		
		MOVAPS	XMM4, XMM6
		ANDNPS	XMM6, XMM0
		ANDNPS	XMM4, XMM1
		
		MOVAPS	XMM0, XMM6
		MOVAPS	XMM1, XMM4

		ORPS	XMM0, XMM3
		ORPS	XMM1, XMM5
		
		MOVAPS	[ESI + EDX*8], XMM1
		
		
		CMP		EDX, cantidad
		JL		seguir

		MOVAPS	[ESI + ECX*8], XMM0

		INC		ECX
		INC		ECX

		CMP		ECX, cantidad
		JL		seguirAun
		
		MOV		EBX, ebxtemp
	}
	
	return true;
}

void Map::shooted(int i, V3 &posImpact, V3 &dirImpact)
{
	int j = MAXBALLS-1;

	getSpeed()[MAXBALLS-1].x = dirImpact.x * 2.0f;
	getSpeed()[MAXBALLS-1].y = dirImpact.y * 2.0f;
	getSpeed()[MAXBALLS-1].z = dirImpact.z * 2.0f;

	if (distancia(i,j) <= this->pballs->radios->rad[i] + this->pballs->radios->rad[j])
	{
		V3 vii, vji;

		V3 normal;

		V3 resultado;
		
		// Velocidades iniciales de 
		vii.x = getSpeed()[i].x;
		vii.y = getSpeed()[i].y;
		vii.z = getSpeed()[i].z;

		vji.x = getSpeed()[j].x;
		vji.y = getSpeed()[j].y;
		vji.z = getSpeed()[j].z;
		

		// Inutil llamada a la funcion restaSSE, solo para ver si anda bien...
		restaSSE(resultado, vii, vji);
		
		float mi = masaEsfera(i);
		float mj = masaEsfera(j);

		float masas = (1.0f / mj + 1.0f / mi);
		
		normal.x = getPositions()[i].x - getPositions()[j].x;
		normal.y = getPositions()[i].y - getPositions()[j].y;
		normal.z = getPositions()[i].z - getPositions()[j].z;


		normalizar(normal);


		float op = (2.0f * (dotProduct(vii, normal) - dotProduct(vji, normal))) / (mj + mi);

		float choque = dotProduct(normal, vji) - dotProduct(normal, vii);


		float promvii = (vii.x + vii.y + vii.z)/3.0f;
		float promvij = (vji.x + vji.y + vji.z)/3.0f;


		if (choque > 0.0f)
		{
			getSpeed()[i].x = vii.x - op * mj * normal.x;
			getSpeed()[i].y = vii.y - op * mj * normal.y;
			getSpeed()[i].z = vii.z - op * mj * normal.z;

			getSpeed()[j].x = vji.x + op * mi * normal.x;
			getSpeed()[j].y = vji.y + op * mi * normal.y;
			getSpeed()[j].z = vji.z + op * mi * normal.z;
		}

		if (fabs(op) < 0.01 && promvii < 0.001 && promvij < 0.001 && ( pballs->standing[i] || pballs->standing[j] ) )
		{
			getSpeed()[i].x = 0.0f;
			getSpeed()[i].y = 0.0f;
			getSpeed()[i].z = 0.0f;

			getSpeed()[j].x = 0.0f;
			getSpeed()[j].y = 0.0f;
			getSpeed()[j].z = 0.0f;

			pballs->standing[i] = true;
			pballs->standing[j] = true;
		}else
		{
			pballs->standing[i] = false;
			pballs->standing[j] = false;
		}
	}
}

//--------------------------------------------------------------------------------------
// Función que setea las nuevas posiciones de las pelotas
//  una vez modificadas las velocidades de estas
//
//--------------------------------------------------------------------------------------
bool Map::moveBalls()
{
	pballs;
	float cota = 1.5f;
	for (int i = 0; i < pballs->ballInstances; i++)
	{
		// No va a hacer falta chequear si son o no visibles en SSE.. da igual
		if (pballs->visible[i])
		{
			if (getSpeed()[i].x >= cota) getSpeed()[i].x = cota;
			if (getSpeed()[i].y >= cota) getSpeed()[i].y = cota;
			if (getSpeed()[i].z >= cota) getSpeed()[i].z = cota;

			getPositions()[i].x += getSpeed()[i].x * 1;
			getPositions()[i].y += getSpeed()[i].y * 1;
			getPositions()[i].z += getSpeed()[i].z * 1;
		}
	}
	return true;
}

bool Map::moveBalls_SSE()
{
	V3* velocidades, *posiciones;
	velocidades = pballs->velocities->speed;
	posiciones = pballs->positions->pos;

	unsigned int cantidad = pballs->ballInstances;
	
	__m128 test0, test1, test2;


	__asm
	{
		MOV		ESI, posiciones
		MOV		EDI, velocidades

		XOR		ECX, ECX
		MOV		EDX, cantidad
		//SHL		EBX, 1
seguir:
		MOVAPS	XMM0, [ESI + ECX*8] // Cargo posiciones
		//prefetchnta	[esi+0x30]
		//MOVAPS	test0, XMM0
		MOVAPS	XMM1, [EDI + ECX*8] // Cargo velocidades
		//prefetchnta	[edi+0x30]
		//MOVUPS	test1, XMM1
		
		LEA		ECX, [ECX+2]

		MOVAPS	XMM2, [ESI + ECX*8]
		MOVAPS	XMM3, [EDI + ECX*8]

		LEA		ECX, [ECX+2]

		MOVAPS	XMM4, [ESI + ECX*8]
		MOVAPS	XMM5, [EDI + ECX*8]

		LEA		ECX, [ECX+2]

		MOVAPS	XMM6, [ESI + ECX*8]
		MOVAPS	XMM7, [EDI + ECX*8]
		
		ADDPS	XMM0, XMM1 // Sumo x coord
		ADDPS	XMM2, XMM3
		ADDPS	XMM4, XMM5
		ADDPS	XMM6, XMM7

		MOVAPS	[ESI + ECX*8], XMM6 // Cargo posiciones

		LEA		ECX, [ECX-2]
		
		MOVAPS	[ESI + ECX*8], XMM4 // Cargo posiciones

		LEA		ECX, [ECX-2]
		
		MOVAPS	[ESI + ECX*8], XMM2 // Cargo posiciones

		LEA		ECX, [ECX-2]
		
		MOVAPS	[ESI + ECX*8], XMM0

		LEA		ECX, [ECX + 8]
		SUB		EDX, 4
		
		JG	seguir
		MOV EAX, 1
	}
}


bool Map::simulatePhysicsBalls(float time, int effect, bool exp)
{
	explosion = exp;
	
	moveBalls_SSE();
	
	checkCollisions();
	
	getInBounds_SSE();
	
	return true;
}


bool Map::simulatePhysicsBalls(float time, int effect, bool exp, V3 &position, V3 &direction)
{
	Impact* impacto = getBallShooted(position,direction);
	
	if (impacto) impactBall(*impacto);
	simulatePhysicsBalls(time,effect,exp);
	return impacto;
}


// Deprecated
void Map::transformation()
{
	
	//float* tmp = this->pballs->positions->pos;
	//this->pballs->positions->posY = this->pballs->positions->posZ;
	//this->pballs->positions->posZ = tmp;
}

//--------------------------------------------------------------------------------------
// Función para agregar una pelota en tiempo de ejecución
//  ver como se puede optimizar el tema del tamaño de los arrays
//
//--------------------------------------------------------------------------------------
bool Map::addBall(V3 &position,V3 &velocity, float radio)
{
	pballs->densities->den[pballs->ballInstances] = 1.0f;
	pballs->positions->pos[pballs->ballInstances] = position;
	pballs->radios->rad[pballs->ballInstances] = radio;
	pballs->velocities->speed[pballs->ballInstances] = velocity;
	pballs->standing[pballs->ballInstances] = false;
	pballs->visible[pballs->ballInstances] = true;

	pballs->ballInstances++;

	return true;
}

Impact* Map::getBallShooted_SSE(V3 &position, V3 &direction)
{
	Impact* index = 0;

	int cant = pballs->ballInstances;
	V3 *inicialPos, *actPos, *inicialDir;

	actPos = pballs->positions->pos;
	
	inicialPos = &position;
	inicialDir = &direction;

	float *radios;
	radios = pballs->radios->rad;

	float *maxDisCameraFinal = new float, *veinte = new float;
	*maxDisCameraFinal = 65536.0f;
	*veinte = 20.0f;
	
	float *result = new float;
	//*result = 0;
	
	*result = 0.0f;

	__m128 test;

	
	__asm
	{
		XOR			EDX, EDX
		XOR			EAX, EAX
		MOV			ECX, cant
		MOV			ESI, inicialPos

		MOVUPS		XMM1, [ESI]			// Movemos position a XMM1 (no se puede usar XMM0 con MOVUPS de memoria)
		
		XORPS		XMM0, XMM0
		MOV			ESI, maxDisCameraFinal
		MOVSS		XMM0, [ESI]			// distCameraFinal queda almacenado en la parte baja de XMM0
		

		MOV			ESI, inicialDir
		MOVUPS		XMM2, [ESI]			// Muevo a XMM2 direction
		MOVUPS		test, XMM2

		MOV			ESI, veinte
		
		MOVUPS		XMM3, [ESI]
		MOVUPS		test, XMM3
		PSHUFD		XMM3, XMM3, 00000000b	// Cargo la constante 20.0 para multiplicar por direction
		MOVUPS		test, XMM3
			
		MULPS		XMM2, XMM3
		ADDPS		XMM2, XMM1			// Calculo x2 como : position + 20.0 * direction

		XORPS		XMM3, XMM3

		MOV			ESI, actPos
		MOV			EDI, radios



seguirMidiendo:
			MOVUPS		XMM4, [ESI + EAX*8]		// Movemos x0 a XMM4
			MOVUPS		test, XMM4

			MOVUPS		XMM7, XMM3
			MOVUPS		XMM3, XMM4
			SUBPS		XMM3, XMM1		// Creamos la variable x0_x1
			MOVUPS		XMM5, XMM3		// Guardo la variable x0_x1
			MOVUPS		test, XMM3

			MULPS		XMM3, XMM3		// Calculo x0_x1 al cuadrado
			PSLLDQ		XMM3, 4
			PSRLDQ		XMM3, 4
			HADDPS		XMM3, XMM3
			HADDPS		XMM3, XMM3		// distCamera = normaCuad(x0_x1); (queda guardado en la parte baja)
			MOVUPS		test, XMM3

			PSHUFD		XMM3, XMM3, 00011011b
			PSHUFD		XMM7, XMM7, 00011011b
			MOVSS		XMM3, XMM7
			PSHUFD		XMM3, XMM3, 00011011b

			MOVUPS		test, XMM2
			MOVUPS		test, XMM5
			SUBPS		XMM4, XMM2		// Calculo x0_x2
			
			//Tengo que calcular el Cross Product entre x0_x1 = XMM5 y x0_x2 = XMM4
			// Calculo num, tengo que hacer crossproduct

			MOVUPS		test, XMM5
			MOVUPS		XMM6, XMM5
			PSHUFD		XMM6, XMM6, 11010010b
			MOVUPS		test, XMM6
			MULPS		XMM6, XMM4
			MOVUPS		test, XMM6
			PSHUFD		XMM6, XMM6, 11010010b
			MOVUPS		test, XMM6

			MOVUPS		test, XMM5
			MOVUPS		XMM7, XMM5
			PSHUFD		XMM7, XMM7, 11001001b
			MOVUPS		test, XMM7
			MULPS		XMM7, XMM4
			MOVUPS		test, XMM7
			PSHUFD		XMM7, XMM7, 11001001b
			MOVUPS		test, XMM7

			SUBPS		XMM6, XMM7
			MOVUPS		XMM5, XMM6

			// num = XMM5

			MOVUPS		XMM4, XMM2
			SUBPS		XMM4, XMM1		// XMM4 es x2_x1
			MOVUPS		test, XMM4

			MULPS		XMM5, XMM5
			PSLLDQ		XMM5, 4
			PSRLDQ		XMM5, 4
			HADDPS		XMM5, XMM5
			HADDPS		XMM5, XMM5		// Calculo en XMM5 = normaCuad(num)

			MULPS		XMM4, XMM4
			PSLLDQ		XMM4, 4
			PSRLDQ		XMM4, 4
			HADDPS		XMM4, XMM4
			HADDPS		XMM4, XMM4		// Calculo en XMM4 = normaCuad(x2_x1)

			
			DIVPS		XMM5, XMM4		// distCuad = normaCuad(num)/normaCuad(x2_x1);
			MOVUPS		test, XMM5

			MOVSS		XMM6, [EDI + EDX*4]
			MOVUPS		test, XMM6
			MULSS		XMM6, XMM6		// Cargo el radio actual y lo elevo al cuadrado
			MOVUPS		test, XMM6
			
			MOVUPS		XMM4, XMM6		// Guardo el radio en otro registro para una futura comparacion

			MOVUPS		XMM7, XMM0
			MOVUPS		test, XMM7

			MOVUPS		test, XMM3
			CMPNLTPS	XMM7, XMM3		// if (distCameraFinal >= distCamera)
			MOVUPS		test, XMM7
			CMPNLTPS	XMM6, XMM5		// if ( pow(radioActual, 2) >= distCuad)
			MOVUPS		test, XMM6
			ANDPS		XMM6, XMM7		// XMM6 = if (distCameraFinal >= distCamera && distCuad < pow(radioActual, 2))
			MOVUPS		test, XMM6
			PSHUFD		XMM6, XMM6, 00000000b
			MOVUPS		test, XMM6
			
			/*MOVUPS		test, XMM1
			PSHUFD		XMM1, XMM1, 00011011b
			MOVUPS		test, XMM1
			ADDSS		XMM1, XMM6
			MOVUPS		test, XMM1
			PSHUFD		XMM1, XMM1, 00011011b
			MOVUPS		test, XMM1*/
			
			MOVUPS		XMM7, XMM0
			MOVUPS		test, XMM7
			CMPLTPS		XMM7, XMM3		// if !(distCameraFinal >= distCamera)
			MOVUPS		test, XMM7
			CMPLTPS		XMM4, XMM5		// if !(distCuad < pow(radioActual, 2))
			MOVUPS		test, XMM4
			ORPS		XMM7, XMM4		// if (!distCameraFinal >= distCamera || !distCuad < pow(radioActual, 2))
			MOVUPS		test, XMM7
			PSHUFD		XMM6, XMM6, 00000000b
			MOVUPS		test, XMM7
			
			MOVUPS		test, XMM3
			PSHUFD		XMM3, XMM3, 00011011b
			DIVSS		XMM5, XMM5
			ADDSS		XMM3, XMM5

			/*PSHUFD		XMM0, XMM0, 00011011b
			MOVSS		XMM0, XMM3
			PSHUFD		XMM0, XMM0, 00011011b*/

			MOVUPS		test, XMM3
			PSHUFD		XMM3, XMM3, 00011011b
			MOVUPS		test, XMM3
			
			MOVUPS		XMM5, XMM3
			ANDPS		XMM3, XMM6		// distCamera * alfa(distCameraFinal >= distCamera && distCuad < pow(radioActual, 2))
			ANDPS		XMM0, XMM7		// distCameraFinal * alfa(!distCameraFinal >= distCamera || !distCuad < pow(radioActual, 2))

			ORPS		XMM0, XMM3		// Queda actualizado el valor de distCameraFinal (sigo con el mismo procedimiento para el resto)
			MOVUPS		test, XMM0
			MOVUPS		XMM3, XMM5
			
			INC			EDX
			LEA			EAX, [EAX + 2]
			DEC			ECX
			JNZ			seguirMidiendo

		MOV			EDI, result
		MOV			ESI, maxDisCameraFinal
		MOVSS		[ESI], XMM0
		PSHUFD		XMM0, XMM0, 00011011b
		MOVSS		[EDI], XMM0
	
	}

	if (*maxDisCameraFinal != 65536.0f)
	{ 
		index = new Impact();
		index->ballIndex = (int)*result - 1;
		index->position = position;
		index->direction = direction;
	}

	return index;
}

Impact* Map::getBallShooted(V3 &position, V3 &direction)
{
	Impact* index = 0;
	
	V3 x2;

	int indexFinal;

	x2.x = direction.x * 20.0f + position.x; 
	x2.y = direction.y * 20.0f + position.y; 
	x2.z = direction.z * 20.0f + position.z;

	float distCuad, distCamera, distCuadFinal = 100000.0f, distCameraFinal = 100000.0f, radioActual;
	V3 x0_x1, x0, x0_x2, den, num, x2_x1;



	for (int i = 0; i < pballs->ballInstances; i++)
	{
		if (pballs->visible[i])
		{
			x0 = pballs->positions->pos[i];

			resta(x0_x1, x0, position);
			distCamera = normaCuad(x0_x1);
			
			if (distCamera < distCameraFinal)
			{
				radioActual = pballs->radios->rad[i];
				
				resta(x0_x2, x0, x2);

				crossProduct(num, x0_x1, x0_x2);
				
				resta(x2_x1, x2, position);

				distCuad = normaCuad(num)/normaCuad(x2_x1);

				if (distCuad < pow(radioActual, 2))
				{
					distCameraFinal = distCamera;
					indexFinal = i;
				}
			}
		}
	}

	if (distCameraFinal != 100000.0f)
	{
		index = new Impact();
		index->ballIndex = indexFinal;
		index->position = position;
		index->direction = direction;
	}

	return index;
}

int Map::raySphere(V3 &p1,V3 &dp,V3 &sc, float r, float *mu1, float *mu2)
{
	float a,b,c;
	float bb4ac;

	a = dp.x * dp.x + dp.y * dp.y + dp.z * dp.z;
	b = 2 * (dp.x * (p1.x - sc.x) + dp.y * (p1.y - sc.y) + dp.z * (p1.z - sc.z));
	c = sc.x * sc.x + sc.y * sc.y + sc.z * sc.z;
	c += p1.x * p1.x + p1.y * p1.y + p1.z * p1.z;
	c -= 2 * (sc.x * p1.x + sc.y * p1.y + sc.z * p1.z);
	c -= r * r;
	bb4ac = b * b - 4 * a * c;
	if ( bb4ac < 0 )
	{
	  *mu1 = 0;
	  *mu2 = 0;
	  return(0);
	}

	*mu1 = (-b + sqrt(bb4ac)) / (2 * a);
	*mu2 = (-b - sqrt(bb4ac)) / (2 * a);

	return(1);
}

int Map::raySphereSSE(V3 &p1,V3 &dp,V3 &sc, float r, float *mu1, float *mu2)
{
	float a,b,c;
	float bb4ac;
	float r1 = r;
	
	float dpx = dp.x;
	float dpy = dp.y;
	float dpz = dp.z;

	float scx = sc.x;
	float scy = sc.y;
	float scz = sc.z;

	float p1x = p1.x;
	float p1y = p1.y;
	float p1z = p1.z;

	__asm{
		FLD scx
		FLD scy
		FLD scz
		FLD p1x
		FLD st(0) 
		FMUL st(0), st(4) //Estado 1
		//Calculé scx*p1x
		
		FLD p1y
		FXCH st(1)
		FLD st(1)
		FMUL st(0), st(5) //Estado 2
		FADDP st(1), st(0)
		//Calculé scx*p1x + scy*p1y
		
		FLD p1z
		FXCH st(1)
		FLD st(1)
		FMUL st(0), st(5) //Estado 3
		FADDP st(1), st(0)
		//Calculé scx*p1x + scy*p1y + scz*p1z

		FLD1
		FADD st(0), st(0)
		FMULP st(1), st(0)
		FCHS //Estado 4
		//Calculé -2*(scx*p1x + scy*p1y + scz*p1z)
		
		FLD r1
		FMUL st(0), st(0)
		FSUBP st(1), st(0) //Estado 5. st(1)=st(1)-st(0) y POP st(0)
		//Calculé -2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FLD st(3)
		FMUL st(0), st(0)
		FADDP st(1), st(0)
		//Calculé p1.x*p1.x -2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FLD st(2)
		FMUL st(0), st(0)
		FADDP st(1), st(0)
		//Calculé p1.x*p1.x + p1.y*p1.y - 2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FLD st(1)
		FMUL st(0), st(0)
		FADDP st(1), st(0)
		//Calculé p1.x*p1.x + p1.y*p1.y + p1.z*p1.z - 2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FLD st(6)
		FMUL st(0), st(0)
		FADDP st(1), st(0)
		//Calculé sc.x*sc.x + p1.x*p1.x + p1.y*p1.y + p1.z*p1.z - 2*(scx*p1x + scy*p1y + scz*p1z) - r*r
		
		FLD st(5)
		FMUL st(0), st(0)
		FADDP st(1), st(0)
		//Calculé sc.x*sc.x + sc.y*sc.y + p1.x*p1.x + p1.y*p1.y + p1.z*p1.z - 2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FLD st(4)
		FMUL st(0), st(0)
		FADDP st(1), st(0) //Estado 6
		//Calculé c = sc.x*sc.x + sc.y*sc.y + sc.z*sc.z + p1.x*p1.x + p1.y*p1.y + p1.z*p1.z - 2*(scx*p1x + scy*p1y + scz*p1z) - r*r

		FXCH st(6)
		FSUBP st(3), st(0) //Estado 7

		FXCH st(3)
		FSUBP st(3), st(0) //Estado 8

		FXCH st(3)
		FSUBP st(3), st(0) //Estado 9

		FLD dpx
		FLD st(0)
		FMULP st(2), st(0)
		FMUL st(0), st(0) //Estado 10
		//Calculé dpx*(p1x-scx) y dpx*dpx

		FLD dpy
		FLD st(0)
		FMULP st(5), st(0)
		FMUL st(0), st(0) //Estado 11
		//Calculé dpy*(p1y-scy) y dpy*dpy

		FLD dpz
		FLD st(0)
		FMULP st(5), st(0)
		FMUL st(0), st(0) //Estado 12
		//Calculé dpz*(p1z-scz) y dpz*dpz

		FADDP st(1), st(0)
		FADDP st(1), st(0) //Estado 13
		//Calculé a = dp.x * dp.x + dp.y * dp.y + dp.z * dp.z;

		FXCH st(3)
		FADDP st(1), st(0)
		FADDP st(1), st(0)
		FLD1
		FADD st(0), st(0)
		FMULP st(1), st(0) //Estado 14
		//Calculé b = 2*(dp.x*(p1.x-sc.x) + dp.y*(p1.y-sc.y) + dp.z*(p1.z-sc.z))

		FLD st(0)
		FMUL st(0), st(0) //b*b
		FLD1
		FADD st(0), st(0)
		FADD st(0), st(0)
		FMUL st(0), st(4)
		FMUL st(0), st(3) //4*a*c
		FSUBP st(1), st(0) //Estado 15
		//Calculé bb4ac = b*b - 4*a*c

		MOV ESI, mu1
		MOV EDI, mu2

		FLDZ
		FCOMP
		FSTSW ax
		SAHF
		JL raiz_negativa //if ( bb4ac < 0 )

		FSQRT
		FLD st(0)
		FSUB st(0), st(2) // -b + sqrt(bb4ac)
		FXCH st(1)
		FCHS
		FSUB st(0), st(2)//Estado 16. -b - sqrt(bb4ac)

		FLD st(3)
		FADD st(0), st(0)
		FLD st(0)
		FDIVP st(3), st(0)
		FDIVP st(1), st(0)//Estado 17
		//Calculé (-b+sqrt(bb4ac))/(2*a) y (-b-sqrt(bb4ac))/(2*a)

		MOV EAX, mu2
		FSTP [EAX]
		MOV EAX, mu1
		FSTP [EAX]
		MOV EAX, 1
		JMP fin

raiz_negativa:
		MOV [ESI], 0
		MOV [EDI], 0
		MOV EAX, 0

fin:
	}
}

bool Map::impactBall(Impact& impacto)
{
	V3 posOldBall;

	posOldBall = getPositions()[impacto.ballIndex];

	float* raiz1 = new float;
	float* raiz2 = new float;

	raySphereSSE(impacto.position, impacto.direction, posOldBall, getRadios()[impacto.ballIndex], raiz1, raiz2);

	V3 posImpact;

	if (raiz1 > raiz2) raiz1 = raiz2;

	posImpact.x = impacto.position.x + impacto.direction.x * *raiz1;
	posImpact.y = impacto.position.y + impacto.direction.y * *raiz1;
	posImpact.z = impacto.position.z + impacto.direction.z * *raiz1;

	shooted(impacto.ballIndex, posImpact, impacto.direction);

	getRadios()[impacto.ballIndex] /= 2.0f;

	V3 posNewBall, velNewBall;	

	posNewBall.x = getPositions()[impacto.ballIndex].x;
	posNewBall.y = getPositions()[impacto.ballIndex].y;
	posNewBall.z = getPositions()[impacto.ballIndex].z + getRadios()[impacto.ballIndex];

	velNewBall.x = getSpeed()[MAXBALLS-1].x;
	velNewBall.y = getSpeed()[MAXBALLS-1].y;
	velNewBall.z = getSpeed()[MAXBALLS-1].z;

	getPositions()[impacto.ballIndex].z -= getRadios()[impacto.ballIndex];

	addBall(posNewBall, velNewBall, getRadios()[impacto.ballIndex]);

	return false;
}