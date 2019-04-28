#include "main.h"

typedef struct
{
	D3DXVECTOR3 vtx;
	D3DXVECTOR2 tex;
}ParticleVertex;

typedef struct
{
	D3DXVECTOR2 tex;
}ParticleTex;

typedef struct
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 rot;
	D3DXVECTOR3 scl;
}Transform;

void InitParticle(void);
void UninitParticle(void);
void UpdateParticle(void);
void DrawParticle(void);