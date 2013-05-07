#ifndef _SUBONOCEANFLOOR_H_
#define _SUBONOCEANFLOOR_H_
/*
 *	SubOnOceanFloor.h
 *
 *	An opengl scene of a submarine on the ocean floor. Its pretty cool. :)
 *
 *	Coded by Joseph A. Marrero
 *	http://www.l33tprogrammer.com/
 *	12/15/2006
 */

#include <string>
#include <vector>
#include <MathLib.h>
#include <Vector3.h>
#include <ImageIO.h>
#include <OBJLoader/OBJLoader.h>
#include "ParticleSystem/PArticle.h"
#include "ParticleSystem/ParticlePolicies/InitializePolicy.h"
#include "ParticleSystem/ParticlePolicies/ActionPolicy.h"
#include "ParticleSystem/ParticleGroup.h"

using namespace std;
#define ESC							27
#define MILLIS_PER_FRAME			25
#define MILLIS_TO_UPDATE_CAUSTICS	50
void initialize( );
void deinitialize( );

void render( );
void resize( int width, int height );
void keyboard_specialkeypress( int key, int x, int y );
void keyboard_keypress( unsigned char key, int x, int y );
void writeText( void *font, std::string &text, int x, int y );
void idle( );
void onTimer( int value );
void onTimedRender( int value );

/*
 *	Helpers
 */
#define BUFFER_OFFSET(bytes)	((GLubyte *) NULL + (bytes))
GLfloat fogColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };	

struct tagCamera {
	float x, y, z;
	float yAngle;
} Camera;

typedef enum tagRenderPass {
	RP_NORMAL,
	RP_CAUSTICS
} RenderPass;

float light0Position[ ] = { -10.5f, 35.0f, -10.0f, 0.0f };
float light0Diffuse[ ] = { 0.8f, 0.8f, 0.9f, 1.0f };//{ 0.8f, 0.8f, 0.8f, 1.0f };
float light0Direction[ ] = { -1.5f, -1.0f, -0.5f, 0.0f };
float light1Position[ ] = { 0.0f, 5.0f, 4.0f, 1.0f };
float light1Floodlight[ ] = { 1.0f, 1.0f, 1.0f, 1.0f };
float light1Direction[ ] = { 0.0f, -1.8, -3.0f, 0.0f };

float worldAmbient[ ] = { 0.08f, 0.09f, 0.1f, 0.0f };

#define	NUMBER_OF_CAUSTIC_IMAGES	32

ImageIO::Image causticTargaImages[ NUMBER_OF_CAUSTIC_IMAGES ];
unsigned int causticsTextures[ NUMBER_OF_CAUSTIC_IMAGES ];
unsigned int currentCaustic;
void loadCausticTextures( );
void renderPass( RenderPass rp );

/*
 *	Ocean Floor Stuff
 */
ImageIO::Image oceanFloorTargaImage;
unsigned int oceanFloorTexture;
unsigned int oceanFloorBufferObj;
float oceanFloorMaterial[] = { 0.8f, 0.8f, 0.82f, 1.0f };
float oceanFloorEmissionMaterial[] = { 0.02f, 0.02f, 0.022f, 1.0f };
std::vector<OBJLoader::MeshVertex> oceanFloor;

void loadOceanFloor( );
void renderOceanFloor( RenderPass rp );

/*
 *	Submarine stuff
 */
struct tagSubmarineModel {
	std::vector<OBJLoader::MeshVertex> hullMesh;
	std::vector<OBJLoader::MeshVertex> leftProp;
	std::vector<OBJLoader::MeshVertex> rightProp;
	std::vector<OBJLoader::MeshVertex> backDivePlane;
	float propAngle;
	float currentXangle;
	float currentYangle;
	float x, y, z;

	
} Submarine;

#define SUB_PATH_RADIUS		60.0f

class CSubmarineCavitationInitializePolicy : public InitializePolicy<Particle>
{
  public:
	CSubmarineCavitationInitializePolicy(){}

	void operator()( Particle &part )
	{
		part.m_Color.m_Red = part.m_Color.m_Green = part.m_Color.m_Blue = 1.0f;
		part.m_Color.m_Alpha = (rand( ) % 100) / 100.0f;
		part.m_nLifeSpan = 1;
		part.m_Size = (rand() % 30) / 100.0f;
		part.m_vVelocity.X = 0;
		part.m_vVelocity.Y = 0;
		part.m_vVelocity.Z = 0;
		part.m_ptPosition.X = SUB_PATH_RADIUS * cos(Submarine.currentYangle - 0.28f) - ((rand() % 600)-300) / 100.0f;
		part.m_ptPosition.Y = Submarine.y - 1.8f + ((rand() % 400)-200) / 100.0f;
		part.m_ptPosition.Z = SUB_PATH_RADIUS * sin(Submarine.currentYangle - 0.28f)  - ((rand() % 600)-300) / 100.0f;
	}
};

class CSubmarineCavitationActionPolicy : public ActionPolicy<Particle>
{
public:
	CSubmarineCavitationActionPolicy(){}

	void operator()( Particle &part )
	{
		
		if( sqrt( (Submarine.x - part.m_ptPosition.X) * (Submarine.x - part.m_ptPosition.X) + 
			  (Submarine.y - part.m_ptPosition.Y) * (Submarine.y - part.m_ptPosition.Y) +
			  (Submarine.z - part.m_ptPosition.Z) * (Submarine.z - part.m_ptPosition.Z) ) >= 75.0f )
				part.m_nLifeSpan = 0;
		
		part.m_ptPosition.Y = part.m_ptPosition.Y + 0.1f;

		part.m_Size -= 0.01f;
		if( part.m_Size <= 0.0f )
			part.m_Size = 0.0f;
		part.m_Color.m_Alpha -= 0.02f;
		if( part.m_Color.m_Alpha <= 0.0f )
			part.m_Color.m_Alpha = 0.0f;
	}



	void PrepareAction( )
	{
	}
};


class CBubblesInitializePolicy : public InitializePolicy<Particle>
{
  public:
	CBubblesInitializePolicy(){}

	void operator()( Particle &part )
	{
		part.m_Color.m_Red = part.m_Color.m_Green = part.m_Color.m_Blue = 1.0f;
		part.m_Color.m_Alpha = 1;//(rand( ) % 100) / 100.0f;
		part.m_nLifeSpan = 1;
		part.m_Size =  (1 + (rand() % 30)) / 100.0f;
		part.m_vVelocity.X = 0;
		part.m_vVelocity.Y = 0;
		part.m_vVelocity.Z = 0;		
		part.m_ptPosition.X = (((rand() % 3800)-1900) )/ 100.0f;
		part.m_ptPosition.Y = 30.0f;
		part.m_ptPosition.Z = ((rand() % 1000)-500) / 100.0f;
	}


};

class CBubblesActionPolicy : public ActionPolicy<Particle>
{
public:
	CBubblesActionPolicy(){}

	void operator()( Particle &part )
	{
		
		if( sqrt( part.m_ptPosition.Y * part.m_ptPosition.Y ) >= 50.0f )
				part.m_nLifeSpan = 0;
		
		part.m_ptPosition.Y = part.m_ptPosition.Y + 0.25f;

		part.m_Size -= 0.001f;
		if( part.m_Size <= 0.0f )
			part.m_Size = 0.0f;
		part.m_Color.m_Alpha -= 0.1f;
		if( part.m_Color.m_Alpha <= 0.0f )
			part.m_Color.m_Alpha = 0.0f;
	}



	void PrepareAction( )
	{
	}
};


ImageIO::Image submarineTargaImage;
unsigned int submarineTexture;
unsigned int submarineHullBufferObj;
unsigned int submarineBackDivePlaneBufferObj;
unsigned int submarineLeftPropBufferObj;
unsigned int submarineRightPropBufferObj;
float submarineAmbientMaterial[] = { 0.12f, 0.14f, 0.15f, 1.0f };
float submarineMaterial[] = { 0.15f, 0.15f, 0.15f, 1.0f };
float submarineSpecularMaterial[] = { 0.05f, 0.05f, 0.08f, 1.0f };
ParticleGroup<500, CSubmarineCavitationInitializePolicy, CSubmarineCavitationActionPolicy, Particle> cavitationParticleGroup;


void loadSubmarine( );
void renderSubmarine( RenderPass rp );
void renderSubmarineCavitation( );


unsigned int cavitationTexture;
ImageIO::Image cavitationTargaImage;

ParticleGroup<3, CBubblesInitializePolicy, CBubblesActionPolicy, Particle> bubbleParticleGroup;
void renderEscapingBubbles( );

/*
 *	Underwater Base
 */

struct tagUnderwaterBase {
	std::vector<OBJLoader::MeshVertex> mesh;
	std::vector<OBJLoader::MeshVertex> glass_mesh;
} UnderwaterBase;
unsigned int underwaterBaseBufferObj;
unsigned int underwaterBaseGlassBufferObj;
float underwaterBaseMaterial[] = { 0.8f, 0.8f, 0.8f, 1.0f };
float underwaterBaseSpecularMaterial[] = { 0.5f, 0.60f, 0.62f, 1.0f };

float underwaterBaseGlassMaterial[] = { 0.65f, 0.63f, 0.66f, 0.2f };
float underwaterBaseGlassSpecularMaterial[] = { 0.5f, 0.5f, 0.5f, 0.9f };

void loadUnderwaterBase( );
void renderUnderwaterBase( RenderPass rp );



/*
 *  Lightstand
 */
struct tagLightstand {
	std::vector<OBJLoader::MeshVertex> mesh;
} Lightstand;

unsigned int lightstandTexture;
unsigned int lightstandGodraysTexture;
unsigned int lightstandBufferObj;
ImageIO::Image lightstandTargaImage;
ImageIO::Image lightstandGodraysTargaImage;
float lightstandDiffuseMaterial[] = { 0.9f, 0.9f, 0.9f, 1.0f };
float lightstandAmbientMaterial[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float lightstandSpecularMaterial[] = { 0.9f, 0.9f, 0.9f, 1.0f };
void loadLightstand( );
void renderLightstand( RenderPass rp );


void renderLensFlare( );
unsigned int lensFlareTexture;
ImageIO::Image lensFlareTargaImage;

float lightMaterial[] = { 1.0f, 1.0f, 1.0f, 1.0f };


/*
 *	Fishes
 */
#define NUMBER_OF_FISH_TYPES	5
#define NUMBER_OF_FISH			1

struct tagFish {
	unsigned int type;
	float size;
	float x, y, z;
} Fish[ NUMBER_OF_FISH ];

unsigned int fishTexture[ NUMBER_OF_FISH_TYPES ];
ImageIO::Image fishesTargaImage[ NUMBER_OF_FISH_TYPES ];

void loadFishTextures( );
void renderFish( );
void seedFish( );

#endif
