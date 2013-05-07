/*
 *	SubOnOceanFloor.cc
 * 
 *	An opengl scene of a submarine on the ocean floor. Its pretty cool. :)
 *
 *	Coded by Joseph A. Marrero
 *	http://www.l33tprogrammer.com/
 *	12/15/2006
 */

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>
#include <MathLib.h>
#include <OBJLoader/OBJLoader.h>
#include "SubOnOceanFloor.h"
#include "glutility.h"



using namespace std;
using namespace Math;

int main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE );


#ifdef _DEBUG
	glutInitWindowSize( 640, 480 );
	glutCreateWindow( "Sub on the ocean floor - Debug Build" );
#else
	glutGameModeString( "1024x768:32@60" );

	if( glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) )
		glutEnterGameMode( );
	else
	{
		glutGameModeString( "640x480@60" );
		if( glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) )
			glutEnterGameMode( );
		else
		{
			cerr << "The requested mode is not available!" << endl;
			return -1;	
		}
	}
#endif


	glutDisplayFunc( render );
	glutReshapeFunc( resize );
	glutKeyboardFunc( keyboard_keypress );
	//glutSpecialFunc( keyboard_specialkeypress );
	glutIdleFunc( idle );
	glutTimerFunc( MILLIS_TO_UPDATE_CAUSTICS, onTimer, 0 );
	//glutTimerFunc( MILLIS_PER_FRAME, onTimedRender, 0 );
	glutSetCursor( GLUT_CURSOR_NONE );
	

	GLenum err = glewInit( );
	if( GLEW_OK != err )
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		cerr << "Error: " << glewGetErrorString( err ) << endl;
		return -1;
	}

	initialize( );
	glutMainLoop( );
	deinitialize( );

	return 0;
}


void initialize( )
{
	Math::seed( );
	
	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_LINE_SMOOTH );
	//glEnable( GL_POLYGON_SMOOTH );
	glEnable( GL_FOG );
#ifdef _DEBUG
	glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_FASTEST );
#else
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
#endif
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	

	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glShadeModel( GL_SMOOTH );
	glClearDepth( 1.0f );						
	glDepthFunc( GL_LEQUAL );
	glEnable( GL_DEPTH_TEST );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	glLightfv( GL_LIGHT0, GL_POSITION, light0Position );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light0Diffuse );
	glLightfv( GL_LIGHT0, GL_SPOT_DIRECTION, light0Direction );
	glLightf( GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f );
	glLightf( GL_LIGHT0, GL_SPOT_EXPONENT, 10.0f );


	glEnable( GL_LIGHT1 );
	glLightfv( GL_LIGHT1, GL_POSITION, light1Position );
	glLightfv( GL_LIGHT1, GL_DIFFUSE, light1Floodlight );
	glLightfv( GL_LIGHT1, GL_SPOT_DIRECTION, light1Direction );
	glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f );
	glLightf( GL_LIGHT1, GL_SPOT_CUTOFF, 20.0f );
	glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.20f );
	glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02f );

	glLightModelf( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
	glLightModelf( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, worldAmbient );

	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );


	// fog stuff
	glEnable( GL_FOG );

	glFogi( GL_FOG_MODE, GL_LINEAR );		// Fog Mode
	glFogfv( GL_FOG_COLOR, fogColor );			// Set Fog Color
	glFogf( GL_FOG_DENSITY, 95.0f );				// How Dense Will The Fog Be
	glHint( GL_FOG_HINT, GL_NICEST );			// Fog Hint Value
	glFogf( GL_FOG_START, 1.0f );				// Fog Start Depth
	glFogf( GL_FOG_END, 200.0f );	


	//glEnable( GL_NORMALIZE );

	glEnable( GL_TEXTURE_2D );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	loadCausticTextures( );
	currentCaustic = 0;

	ImageIO::Result r = ImageIO::loadImage( &cavitationTargaImage, "assets/cavitation.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );

	glGenTextures( 1, &cavitationTexture );
	glBindTexture( GL_TEXTURE_2D, cavitationTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, (cavitationTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   cavitationTargaImage.width, cavitationTargaImage.height,
					   GL_RGBA, GL_UNSIGNED_BYTE,
					   cavitationTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	ImageIO::destroyImage( &cavitationTargaImage );

	loadFishTextures( );
	seedFish( );

	// load models...
	loadOceanFloor( );
	loadSubmarine( );
	loadUnderwaterBase( );
	loadLightstand( );

	//create buffer objects...
	glGenBuffers( 1, &oceanFloorBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, oceanFloorBufferObj );
	glBufferData( GL_ARRAY_BUFFER, oceanFloor.size( ) * sizeof(OBJLoader::MeshVertex), &oceanFloor[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &submarineHullBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, submarineHullBufferObj );
	glBufferData( GL_ARRAY_BUFFER, Submarine.hullMesh.size( ) * sizeof(OBJLoader::MeshVertex), &Submarine.hullMesh[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &submarineBackDivePlaneBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, submarineBackDivePlaneBufferObj );
	glBufferData( GL_ARRAY_BUFFER, Submarine.backDivePlane.size( ) * sizeof(OBJLoader::MeshVertex), &Submarine.backDivePlane[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &submarineLeftPropBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, submarineLeftPropBufferObj );
	glBufferData( GL_ARRAY_BUFFER, Submarine.leftProp.size( ) * sizeof(OBJLoader::MeshVertex), &Submarine.leftProp[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &submarineRightPropBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, submarineRightPropBufferObj );
	glBufferData( GL_ARRAY_BUFFER, Submarine.rightProp.size( ) * sizeof(OBJLoader::MeshVertex), &Submarine.rightProp[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &underwaterBaseBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, underwaterBaseBufferObj );
	glBufferData( GL_ARRAY_BUFFER, UnderwaterBase.mesh.size( ) * sizeof(OBJLoader::MeshVertex), &UnderwaterBase.mesh[ 0 ], GL_STATIC_DRAW );

	glGenBuffers( 1, &underwaterBaseGlassBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, underwaterBaseGlassBufferObj );
	glBufferData( GL_ARRAY_BUFFER, UnderwaterBase.glass_mesh.size( ) * sizeof(OBJLoader::MeshVertex), &UnderwaterBase.glass_mesh[ 0 ], GL_STATIC_DRAW );


	glGenBuffers( 1, &lightstandBufferObj );
	glBindBuffer( GL_ARRAY_BUFFER, lightstandBufferObj );
	glBufferData( GL_ARRAY_BUFFER, Lightstand.mesh.size( ) * sizeof(OBJLoader::MeshVertex), &Lightstand.mesh[ 0 ], GL_STATIC_DRAW );


	Camera.y = 15.0f;
	
	// check for any errors...
	assert( glGetError( ) == GL_NO_ERROR );
}

void loadCausticTextures( )
{
	GLfloat envColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glGenTextures( NUMBER_OF_CAUSTIC_IMAGES, &causticsTextures[ 0 ] );

	for( int i = 0; i < NUMBER_OF_CAUSTIC_IMAGES; i++ )
	{
		ostringstream oss;
		oss << "assets/caustics/caustics_";
		oss.setf( ios::fixed );
		oss.width( 3 );
		oss.fill( '0' );
		oss << i + 1 << ".tga";

		cout << "Loading " << oss.str() << "..." << endl;

		ImageIO::Result r = ImageIO::loadImage( &causticTargaImages[ i ], oss.str( ).c_str( ), ImageIO::TARGA );
		assert( r == ImageIO::SUCCESS );

		glBindTexture( GL_TEXTURE_2D, causticsTextures[ i ] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE,
						   causticTargaImages[ i ].width, causticTargaImages[ i ].height,
						   GL_LUMINANCE, GL_UNSIGNED_BYTE, causticTargaImages[ i ].pixelData );
	}

	// free image data...
	for( int i = 0; i < NUMBER_OF_CAUSTIC_IMAGES; i++ )
		ImageIO::destroyImage( &causticTargaImages[ i ] );
}

void deinitialize( )
{
	//delete buffer objects...
	glDeleteBuffers( 1, &oceanFloorBufferObj );
	glDeleteBuffers( 1, &submarineHullBufferObj );
	glDeleteBuffers( 1, &submarineBackDivePlaneBufferObj );
	glDeleteBuffers( 1, &submarineLeftPropBufferObj );
	glDeleteBuffers( 1, &submarineRightPropBufferObj );
	glDeleteBuffers( 1, &underwaterBaseBufferObj );
	glDeleteBuffers( 1, &underwaterBaseGlassBufferObj );
	glDeleteBuffers( 1, &lightstandBufferObj );
	glutLeaveGameMode( );
}


void render( )
{
#ifdef _DEBUG
	static float fps = 0.0f;
	static unsigned int nFrame = 0;
	unsigned int stime = glutGet( GLUT_ELAPSED_TIME );
	unsigned int etime = 0;
	static unsigned int timeDelta = 0;
#endif

	glClearColor( 0.0f, 0.0f, 0.0f, 0.1f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity( );

	// for debugging...
	//glPolygonMode( GL_FRONT, GL_LINE );

	// move the camera...
	Camera.x = 85.0f * cos(Camera.yAngle);
	Camera.z = 85.0f * sin(Camera.yAngle);
	gluLookAt( Camera.x, Camera.y, Camera.z, 0, 0, 0, 0, 1, 0 );

	if( Camera.yAngle <= 0.0f )
		Camera.yAngle = 360.0f;
	Camera.yAngle -= 0.003f;

	glLightfv( GL_LIGHT0, GL_POSITION, light0Position );
	glLightfv( GL_LIGHT0, GL_SPOT_DIRECTION, light0Direction );

	// The background...
	glDisable( GL_FOG );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glPushMatrix( );
		glLoadIdentity( );
		glTranslatef( 0.0f, 0.0f, -200.0f );
		glScalef( 200.0f, 95.0f, 0.0f );
		
		glBegin( GL_QUADS );
			glColor3f( 0.0f, 0.0f, 0.0f );
			glNormal3i( 0, 0, 1 ); glVertex3i( -1, 0, 0 );
			glColor3f( 0.0f, 0.0f, 0.0f );
			glNormal3i( 0, 0, 1 ); glVertex3i( 1, 0, 0 );
			glColor3f( 0.12f, 0.281f, 0.28f );
			glNormal3i( 0, 0, 1 ); glVertex3i( 1, 1, 0 );
			glColor3f( 0.12f, 0.281f, 0.28f );
			glNormal3i( 0, 0, 1 ); glVertex3i( -1, 1, 0 );
		glEnd( );
	glPopMatrix( );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_LIGHTING );
	glEnable( GL_FOG );

	
	renderEscapingBubbles( );
	renderPass( RP_NORMAL );
	


	glDepthMask( GL_FALSE );
	glDepthFunc( GL_EQUAL );


	glBlendFunc( GL_ZERO, GL_SRC_COLOR );
	glEnable( GL_BLEND );


	renderPass( RP_CAUSTICS );

	/* Restore fragment operations to normal. */
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
	glDisable( GL_BLEND );




	
	renderSubmarineCavitation( );
	renderFish( );




	/* Write text */
	glColor3f( 1.0f, 1.0f, 1.0f );
	int width = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int height = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

	writeText( GLUT_BITMAP_HELVETICA_18, std::string("A Sub on the Ocean Floor"), 2, 22 );
	
	
#ifdef _DEBUG
	etime = glutGet( GLUT_ELAPSED_TIME );
	nFrame++;
	timeDelta = etime - stime;
	if(  timeDelta > 1000 )
	{
		fps = ( nFrame * 10.0f) / timeDelta;
	}
	ostringstream oss;
	oss.precision( 2 );
	oss.setf( ios::fixed );
	oss << "Press <ESC> to quit. (DEBUG VERSION, " << timeDelta << " fps" << ")";
		

	writeText( GLUT_BITMAP_9_BY_15, oss.str(), 2, 5 );

	// check for any errors...
	assert( !GL_UTILITY::HasErrorOccured( ) );
#else
	writeText( GLUT_BITMAP_9_BY_15, std::string("Press <ESC> to quit."), 2, 5 );
#endif
	glutSwapBuffers( );
}

void resize( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	

	if( height == 0 )
		height = 1;
		
	double aspect = (double) height / (double) width;
	
	gluPerspective( 40.0f, (double) width / (double) height, 1.0, 500.0 );
	glMatrixMode( GL_MODELVIEW );

}

void keyboard_specialkeypress( int key, int x, int y )
{
	//switch( key )
	//{
	//	case GLUT_KEY_F1:
	//		{
	//			glutGameModeString( "800x600:16" );
	//			if( glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) )
	//				glutEnterGameMode( );
	//			else
	//			{
	//				cerr << "The requested mode is not available!" << endl;
	//				deinitialize( );
	//				exit(-1);	
	//			}
	//		break;
	//		}
	//	default:
	//		break;
	//}
}

void keyboard_keypress( unsigned char key, int x, int y )
{
	switch( key )
	{
		case 'A':
		case 'a':
			Camera.yAngle += -0.02f;
			break;
		case 'D':
		case 'd':
			Camera.yAngle += 0.02f;
			break;
#ifdef _DEBUG
		case 'W':
		case 'w':
			Camera.y += -0.1f;
			break;
		case 'S':
		case 's':
			Camera.y += 0.1f;
			break;
#endif
		case ESC:
		case GLUT_KEY_END:
			deinitialize( );
			exit( 0 );
			break;
		default:
			break;
	}

}

void writeText( void *font, std::string &text, int x, int y )
{
	int width = glutGet( (GLenum)GLUT_WINDOW_WIDTH );
	int height = glutGet( (GLenum)GLUT_WINDOW_HEIGHT );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix( );
		glLoadIdentity( );	
		glOrtho( 0, width, 0, height, 1.0, 10.0 );
			
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix( );
			glLoadIdentity( );
			glColor3f( 1.0f, 1.0f, 1.0f );
			glTranslatef( 0.0f, 0.0f, -1.0f );
			glRasterPos2i( x, y );

			for( unsigned int i = 0; i < text.size( ); i++ )
				glutBitmapCharacter( font, text[ i ] );
			
		glPopMatrix( );
		glMatrixMode( GL_PROJECTION );
	glPopMatrix( );
	glMatrixMode( GL_MODELVIEW );

	glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_TEXTURE_2D);

}

void idle( )
{ glutPostRedisplay( ); }

void loadOceanFloor( )
{
	GLfloat envColor[] = { 0.9f, 0.92f, 0.99f, 1.0f };
	cout << "Loading Ocean";
	OBJLoader loader;

	if( !loader.load( "assets/oceanfloor.obj" ) )
	{
		cerr << endl << "ERROR: Loading failed! :(" << endl;
		exit( -1 );
	}

	OBJLoader::GroupCollection &objOcean = loader.mesh( );

	for( unsigned int i = 0; i < objOcean.size( ); i++ )
		for( unsigned int j = 0; j < objOcean[ i ].mesh.size( ); j++ )
		{
			if( j % 100 == 0 ) cout << ".";						
			oceanFloor.push_back( objOcean[ i ].mesh[ j ] );
		}

	cout << " Done!" << endl << endl;

	// load texture data...
	ImageIO::Result r = ImageIO::loadImage( &oceanFloorTargaImage, "assets/oceanfloor.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );

	
	glGenTextures( 1, &oceanFloorTexture );
	glBindTexture( GL_TEXTURE_2D, oceanFloorTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, (oceanFloorTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   oceanFloorTargaImage.width, oceanFloorTargaImage.height,
					   GL_RGBA, GL_UNSIGNED_BYTE,
					   oceanFloorTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR/*GL_LINEAR*/ );

	glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor );
	

	glTexImage2D( GL_TEXTURE_2D,
				  0,
				  (oceanFloorTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
				  oceanFloorTargaImage.width,
				  oceanFloorTargaImage.height,
				  0, //border
				  GL_RGBA,
				  GL_UNSIGNED_BYTE,
				  oceanFloorTargaImage.pixelData );

	// free image data...
	ImageIO::destroyImage( &oceanFloorTargaImage );
}

void loadSubmarine( )
{
	GLfloat envColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	cout << "Loading Submarine";
	OBJLoader loader;

	if( !loader.load( "assets/sub.obj" ) )
	{
		cerr << endl << "ERROR: Loading failed! :(" << endl;
		exit( -1 );
	}

	OBJLoader::GroupCollection &objSubmarine = loader.mesh( );

	for( unsigned int i = 0; i < objSubmarine.size( ); i++ )
	{
		if( objSubmarine[ i ].groupName == "BackDivePlane" )
		{
			for( unsigned int j = 0; j < objSubmarine[ i ].mesh.size(); j++ )
			{
				if( j % 100 == 0 ) cout << ".";
				Submarine.backDivePlane.push_back( objSubmarine[ i ].mesh[ j ] );
			}
		}
		else if( objSubmarine[ i ].groupName == "LeftProp" )
		{
			for( unsigned int j = 0; j < objSubmarine[ i ].mesh.size(); j++ )
			{
				if( j % 100 == 0 ) cout << ".";
				Submarine.leftProp.push_back( objSubmarine[ i ].mesh[ j ] );
			}
		}
		else if( objSubmarine[ i ].groupName == "RightProp" )
		{
			for( unsigned int j = 0; j < objSubmarine[ i ].mesh.size(); j++ )
			{
				if( j % 100 == 0 ) cout << ".";
				Submarine.rightProp.push_back( objSubmarine[ i ].mesh[ j ] );
			}
		}
		else {
			for( unsigned int j = 0; j < objSubmarine[ i ].mesh.size(); j++ )
			{
				if( j % 100 == 0 ) cout << ".";
				Submarine.hullMesh.push_back( objSubmarine[ i ].mesh[ j ] );
			}
		}
	}

	cout << " Done!" << endl << endl;

	// load texture data...
	ImageIO::Result r = ImageIO::loadImage( &submarineTargaImage, "assets/hull.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );


	glGenTextures( 1, &submarineTexture );
	glBindTexture( GL_TEXTURE_2D, submarineTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, (submarineTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   submarineTargaImage.width, submarineTargaImage.height,
					   GL_RGBA, GL_UNSIGNED_BYTE,
					   submarineTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor );

	// free image data...
	ImageIO::destroyImage( &submarineTargaImage );
}

void loadUnderwaterBase( )
{
	cout << "Loading Underwater Base";
	OBJLoader loader;

	if( !loader.load( "assets/underwater_base.obj" ) )
	{
		cerr << endl << "ERROR: Loading failed! :(" << endl;
		exit( -1 );
	}

	OBJLoader::GroupCollection &objUnderwaterBase = loader.mesh( );

	for( unsigned int i = 0; i < objUnderwaterBase.size( ); i++ )
		for( unsigned int j = 0; j < objUnderwaterBase[ i ].mesh.size( ); j++ )
		{
			if( j % 100 == 0 ) cout << ".";

			if( objUnderwaterBase[ i ].groupName.compare("glass") == 0 )
			{
				UnderwaterBase.glass_mesh.push_back( objUnderwaterBase[ i ].mesh[ j ] );
			}
			else
				UnderwaterBase.mesh.push_back( objUnderwaterBase[ i ].mesh[ j ] );
		}

	cout << " Done!" << endl << endl;
}

void loadLightstand( )
{
	unsigned int r = 0;
	cout << "Loading Lightstand";
	OBJLoader loader;

	if( !loader.load( "assets/lightstand.obj" ) )
	{
		cerr << endl << "ERROR: Loading failed! :(" << endl;
		exit( -1 );
	}

	OBJLoader::GroupCollection &objLightstand = loader.mesh( );

	for( unsigned int i = 0; i < objLightstand.size( ); i++ )
		for( unsigned int j = 0; j < objLightstand[ i ].mesh.size( ); j++ )
		{
			if( j % 100 == 0 ) cout << ".";
			Lightstand.mesh.push_back( objLightstand[ i ].mesh[ j ] );
		}

	cout << " Done!" << endl << endl;

	// Load up the textures...
	r = ImageIO::loadImage( &lightstandTargaImage, "assets/lightstand.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );

	glGenTextures( 1, &lightstandTexture );
	glBindTexture( GL_TEXTURE_2D, lightstandTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, (lightstandTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   lightstandTargaImage.width, lightstandTargaImage.height,
					   GL_RGBA, GL_UNSIGNED_BYTE,
					   lightstandTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );


	r = ImageIO::loadImage( &lightstandGodraysTargaImage, "assets/lightstand_godrays.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );

	glGenTextures( 1, &lightstandGodraysTexture );
	glBindTexture( GL_TEXTURE_2D, lightstandGodraysTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, /*GL_LUMINANCE*/(lightstandGodraysTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   lightstandGodraysTargaImage.width, lightstandGodraysTargaImage.height,
					   /*GL_LUMINANCE*/ GL_RGBA, GL_UNSIGNED_BYTE,
					   lightstandGodraysTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	r = ImageIO::loadImage( &lensFlareTargaImage, "assets/lens_flare.tga", ImageIO::TARGA );
	assert( r == ImageIO::SUCCESS );

	glGenTextures( 1, &lensFlareTexture );
	glBindTexture( GL_TEXTURE_2D, lensFlareTexture );
	gluBuild2DMipmaps( GL_TEXTURE_2D, (lensFlareTargaImage.bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
					   lensFlareTargaImage.width, lensFlareTargaImage.height,
					   GL_RGBA, GL_UNSIGNED_BYTE,
					   lensFlareTargaImage.pixelData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );


	// free image data...
	ImageIO::destroyImage( &lightstandTargaImage );
	ImageIO::destroyImage( &lightstandGodraysTargaImage );
	ImageIO::destroyImage( &lensFlareTargaImage );
}

void renderOceanFloor( RenderPass rp )
{
	glPushMatrix( );
		glPushAttrib( GL_CURRENT_BIT );
			if( rp == RP_NORMAL )
				glBindTexture( GL_TEXTURE_2D, oceanFloorTexture );
			glMaterialfv( GL_FRONT, GL_DIFFUSE, oceanFloorMaterial );
			glBindBuffer( GL_ARRAY_BUFFER, oceanFloorBufferObj );
			glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET( 0 ) );
			glDrawArrays( GL_TRIANGLES, 0, oceanFloor.size( ) );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glPopAttrib( );
	glPopMatrix( );
}

void renderSubmarine( RenderPass rp )
{
	static float matrix[ 16 ];
	

	glPushMatrix( );
		glColor4f( 0.5f, 1.0f, 0.5f, 1.0f );


		// Position the sub...
		glTranslatef( 0, 2.0f, -35.0f );

		Submarine.x = SUB_PATH_RADIUS * cos( Submarine.currentYangle );
		Submarine.y = 25.0f + 0.15f * cos( Submarine.currentXangle / 4.0f );
		Submarine.z = SUB_PATH_RADIUS * sin( Submarine.currentYangle );

		glRotatef( 20.0f * cos(Submarine.currentXangle / 4.0f), 1.0f, 0, 0 );
		glTranslatef( Submarine.x, Submarine.y, Submarine.z );
		glScalef( 0.7f, 0.7f, 0.7f );
		
		Math::Vector3 right( Submarine.x, 0.0f, Submarine.z ); // From the orgin
		right.normalize( );
		Math::Vector3 &forward = crossProduct( right, Math::Vector3::Y_UNIT );
		forward.normalize( );
		

		matrix[  0 ] = right.X;
		matrix[  1 ] = right.Y;
		matrix[  2 ] = right.Z;
		matrix[  3 ] = 0.0f;

		matrix[  4 ] = 0.0f;
		matrix[  5 ] = 1.0f;
		matrix[  6 ] = 0.0f;
		matrix[  7 ] = 0.0f;

		matrix[  8 ] = forward.X;
		matrix[  9 ] = forward.Y;
		matrix[ 10 ] = forward.Z;
		matrix[ 11 ] = 0.0f;

		matrix[ 12 ] = 0.0f;
		matrix[ 13 ] = 0.0f;
		matrix[ 14 ] = 0.0f;
		matrix[ 15 ] = 1.0f;
		
		
		glMultMatrixf( matrix ); // Orient the sub along its path...
		
		GLenum err1 = glGetError( );
		assert( err1 == GL_NO_ERROR );

		if( rp == RP_NORMAL )
			glBindTexture( GL_TEXTURE_2D, submarineTexture );
		glMaterialfv( GL_FRONT, GL_DIFFUSE, submarineMaterial );
		glMaterialfv( GL_FRONT, GL_SPECULAR, submarineSpecularMaterial );

		// Hull
		glBindBuffer( GL_ARRAY_BUFFER, submarineHullBufferObj );
		glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
		glDrawArrays( GL_TRIANGLES, 0, Submarine.hullMesh.size( ) );


		glPushMatrix( ); // Back Dive Plane
			glTranslatef( 0, -1.8f*cos(Submarine.currentXangle / 4.0f), 0 );
			glRotatef( 10.0f * cos(Submarine.currentXangle / 4.0f), 1.0f, 0, 0 );	
			glBindBuffer( GL_ARRAY_BUFFER, submarineBackDivePlaneBufferObj );
			glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
			glDrawArrays( GL_TRIANGLES, 0, Submarine.backDivePlane.size( ) );
		glPopMatrix( );

		glPushAttrib( GL_CURRENT_BIT );
			//glMaterialf( GL_FRONT, GL_SHININESS, 128 );

			glPushMatrix( ); // Left Prop
				glTranslatef( 2.35f, -2.0f, 3.0f );
				glRotatef( Submarine.propAngle, 0, 0, 1 );
				glTranslatef( -2.35f, 2.0f, -3.0f );
				glBindBuffer( GL_ARRAY_BUFFER, submarineLeftPropBufferObj );
				glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
				glDrawArrays( GL_TRIANGLES, 0, Submarine.leftProp.size( ) );
			glPopMatrix( );

			glPushMatrix( ); // Right Prop
				glTranslatef( -2.35f, -2.0f, -3.0f );
				glRotatef( 180.0f + Submarine.propAngle, 0.0f, 0.0f, 1.0f );
				glTranslatef( 2.35f, 2.0f, 3.0f );
				glBindBuffer( GL_ARRAY_BUFFER, submarineRightPropBufferObj );
				glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
				glDrawArrays( GL_TRIANGLES, 0, Submarine.rightProp.size( ) );
			glPopMatrix( );
		glPopAttrib( );
	glPopMatrix( );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	Submarine.propAngle += 8.0f;
	if( Submarine.propAngle >= 360.0f ) Submarine.propAngle = 0.0f;

	Submarine.currentXangle += 0.002f;
	if( Submarine.currentXangle >= 8 * M_PI ) Submarine.currentXangle = 0.0f;
	Submarine.currentYangle += 0.0045f;
	if( Submarine.currentYangle >= 2 * M_PI ) Submarine.currentYangle = 0.0f;

}

void renderPass( RenderPass rp )
{
	static GLfloat sPlane[ 4 ] = { 1.0, 0.1, 0.0, 0.0 };
	static GLfloat tPlane[ 4 ] = { 0.0, 0.1, 1.0, 0.0 };

	sPlane[0] = 0.5 / 4.0f;
	sPlane[1] = 0.1 / 10.0f;

	tPlane[1] = 0.1 / 10.0f;
	tPlane[2] = 0.5 / 4.0f;


	if( rp == RP_CAUSTICS )
	{
		glColor3f( 1.0f, 1.0f, 1.0f );
		glDisable( GL_LIGHTING );

		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGenfv( GL_S, GL_OBJECT_PLANE, sPlane );
		glTexGenfv( GL_T, GL_OBJECT_PLANE, tPlane );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );

		glBindTexture( GL_TEXTURE_2D, causticsTextures[ currentCaustic ] );

		glMatrixMode( GL_TEXTURE );
		glPushMatrix( );
			glScalef( 0.5f, 0.5f, 0.5f );
		glMatrixMode( GL_MODELVIEW );
	}

	renderOceanFloor( rp ); // Ocean Floor..
	renderLightstand( rp );
	renderUnderwaterBase( rp );
	renderSubmarine( rp ); // Submarine...

	if( rp == RP_CAUSTICS )
	{
		glEnable( GL_LIGHTING );
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );

		glMatrixMode( GL_TEXTURE );
			glPopMatrix( );
		glMatrixMode( GL_MODELVIEW );
	}

	assert( glGetError( ) == GL_NO_ERROR );
}

void onTimer( int value )
{
	currentCaustic = ( currentCaustic + 1 ) % NUMBER_OF_CAUSTIC_IMAGES;
	glutTimerFunc( MILLIS_TO_UPDATE_CAUSTICS, onTimer, 0 );
}

void renderSubmarineCavitation( )
{

	glDepthMask( GL_FALSE );
	glDisable( GL_LIGHTING );
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

	glPushMatrix( );
		cavitationParticleGroup.Emits( 4, Math::Vertex3(Submarine.x, Submarine.y, Submarine.z) );
		bubbleParticleGroup.Emits( 4, Math::Vertex3(0, 0, 0) );
		const Particle *particles = cavitationParticleGroup.getParticles( );
		unsigned int numberOfParticles = cavitationParticleGroup.getParticlesCount/*getMaxParticles*/( );
		float x, y, z, fSize;

		glTranslatef( 0, 0.0f, -35.0f );
		glRotatef( 20.0f * cos(Submarine.currentXangle / 4.0f), 1.0f, 0.0f, 0.0f );


		//Render
		for( unsigned int i = 0; i < numberOfParticles; i++ )
		{
			glBindTexture( GL_TEXTURE_2D, cavitationTexture );
			glBegin( GL_TRIANGLE_STRIP );
			glColor4f( 0.3f, 0.3f, 0.3f, particles[ i ].m_Color.m_Alpha );
				x = particles[ i ].m_ptPosition.X;
				y = particles[ i ].m_ptPosition.Y;
				z = particles[ i ].m_ptPosition.Z;
				fSize = particles[ i ].m_Size;
		
				
				glTexCoord2d( 1, 1 ); glVertex3f( x + fSize, y + fSize, z );		// Top Right
				glTexCoord2d( 0, 1 ); glVertex3f( x - fSize, y + fSize, z );		// Top Left
				glTexCoord2d( 1, 0 ); glVertex3f( x + fSize, y - fSize, z );	// Bottom Right
				glTexCoord2d( 0, 0 ); glVertex3f( x - fSize, y - fSize, z );	// Bottom Left
			glEnd( );
		}

		cavitationParticleGroup.Update( );
	glPopMatrix( );

	glEnable( GL_LIGHTING );
	glEnable( GL_CULL_FACE );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );

	assert( glGetError( ) == GL_NO_ERROR );
}

void renderEscapingBubbles( )
{
	glDisable( GL_LIGHTING );
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

	glPushMatrix( );
		glLoadIdentity( );
		bubbleParticleGroup.Emits( 1, Math::Vertex3(0, 30, 0) );
		const Particle *particles = bubbleParticleGroup.getParticles( );
		unsigned int numberOfParticles = bubbleParticleGroup.getParticlesCount( );
		float x, y, z, fSize;
		
		//Render
		for( unsigned int i = 0; i < numberOfParticles; i++ )
		{
			glBindTexture( GL_TEXTURE_2D, cavitationTexture );
			glBegin( GL_TRIANGLE_STRIP );
			glColor4f( 0.3f, 0.3f, 0.3f, particles[ i ].m_Color.m_Alpha );
				x = particles[ i ].m_ptPosition.X;
				y = particles[ i ].m_ptPosition.Y;
				z = particles[ i ].m_ptPosition.Z;
				fSize = particles[ i ].m_Size;
		
				
				glTexCoord2d( 1, 1 ); glVertex3f( x + fSize, y + fSize, z );		// Top Right
				glTexCoord2d( 0, 1 ); glVertex3f( x - fSize, y + fSize, z );		// Top Left
				glTexCoord2d( 1, 0 ); glVertex3f( x + fSize, y - fSize, z );	// Bottom Right
				glTexCoord2d( 0, 0 ); glVertex3f( x - fSize, y - fSize, z );	// Bottom Left
			glEnd( );
		}

		bubbleParticleGroup.Update( );
	glPopMatrix( );

	glEnable( GL_CULL_FACE );
	glEnable( GL_LIGHTING );
	glDisable( GL_BLEND );
}

void renderUnderwaterBase( RenderPass rp )
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glPushMatrix( );
		glRotatef( -90.0f, 0.0f, 1.0f, 0.0f );
		glTranslatef( 0.0f, -8.0f, -16.0f );
		glScalef( 2.0f, 2.0f, 2.0f );
		

		glPushAttrib( GL_CURRENT_BIT );
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			if( rp == RP_NORMAL )
				glBindTexture( GL_TEXTURE_2D, submarineTexture );

			// render the structure
			glMaterialfv( GL_FRONT, GL_DIFFUSE, underwaterBaseMaterial );
			glMaterialfv( GL_FRONT, GL_SPECULAR, underwaterBaseSpecularMaterial );
			glBindBuffer( GL_ARRAY_BUFFER, underwaterBaseBufferObj );
			glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
			glDrawArrays( GL_TRIANGLES, 0, UnderwaterBase.mesh.size( ) );

			// render the glass
			glEnable( GL_BLEND );
			glDepthMask( GL_FALSE );
			if( rp == RP_NORMAL )
				glBindTexture( GL_TEXTURE_2D, 0 );
			glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, underwaterBaseGlassMaterial );
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, underwaterBaseGlassSpecularMaterial );
			glBindBuffer( GL_ARRAY_BUFFER, underwaterBaseGlassBufferObj );
			glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET(0) );
			glDrawArrays( GL_TRIANGLES, 0, UnderwaterBase.glass_mesh.size( ) );
			glDisable( GL_BLEND );
			glDepthMask( GL_TRUE );
			
		glPopAttrib( );
	glPopMatrix( );
	
	//glEnable( GL_CULL_FACE );
}

void renderLightstand( RenderPass rp )
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glPushMatrix( );
		
		glTranslatef( -35.0f, 2.1f, -2.0f );
		glScalef( 2.0f, 2.0f, 2.0f );
		glRotatef( -45.0f, 0.0f, 1.0f, 0.0f );

		if( rp == RP_NORMAL )
		{
			glDisable( GL_CULL_FACE );
			glDisable( GL_LIGHTING );
			glEnable( GL_BLEND );
				

			glPushMatrix( );
				glPushAttrib( GL_CURRENT_BIT );					
					glTranslatef( light1Position[ 0 ], light1Position[ 1 ] + 0.2f, light1Position[ 2 ] );

					glPushMatrix( );
						glTranslatef( -0.4f, 0, 0 );
						renderLensFlare( );
					glPopMatrix( );
					glPushMatrix( );
						glTranslatef( 0.4f, 0, 0 );
						renderLensFlare( );
					glPopMatrix( );

					glDepthMask( GL_FALSE );
					glBlendFunc( GL_ONE, GL_ONE );
					glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

					glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
					glBindTexture( GL_TEXTURE_2D, lightstandGodraysTexture );
					glRotatef( 90.0f, 0.0f, 1.0f, 0.0f );
					glTranslatef( 6.3f, -2.0f, 0.0f );
					glRotatef( -25.0f, 0.0f, 0.0f, 1.0f );

					glBegin( GL_TRIANGLE_STRIP );
						glTexCoord2i( -1, -1 ); glVertex2i( -2.5, -1.8 );		// Top Right
						glTexCoord2i( 0, -1 ); glVertex2i( 0, -1.8 );		// Top Left
						glTexCoord2i( -1, 0 ); glVertex2i( -2.5, 0 );	// Bottom Right
						glTexCoord2i( 0, 0 ); glVertex2i( 0, 0 );	// Bottom Left
					glEnd( );
					
					glBindTexture( GL_TEXTURE_2D,  lightstandGodraysTexture );
		
				glPopAttrib( );
			glPopMatrix( );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glDisable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


			glDepthMask( GL_TRUE );
			glEnable( GL_LIGHTING );
			glEnable( GL_CULL_FACE );
		}

		glLightfv( GL_LIGHT1, GL_POSITION, light1Position );
		glLightfv( GL_LIGHT1, GL_SPOT_DIRECTION, light1Direction );


		glPushAttrib( GL_CURRENT_BIT );
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			if( rp == RP_NORMAL )
				glBindTexture( GL_TEXTURE_2D, lightstandTexture );

			glMaterialfv( GL_FRONT, GL_DIFFUSE, lightstandDiffuseMaterial );
			glMaterialfv( GL_FRONT, GL_AMBIENT, lightstandAmbientMaterial );
			glMaterialfv( GL_FRONT, GL_SPECULAR, lightstandSpecularMaterial );

			glBindBuffer( GL_ARRAY_BUFFER, lightstandBufferObj );
			glInterleavedArrays( GL_T2F_N3F_V3F, 0, BUFFER_OFFSET( 0 ) );
			glDrawArrays( GL_TRIANGLES, 0, Lightstand.mesh.size( ) );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glPopAttrib( );
	glPopMatrix( );
}

void renderLensFlare( )
{
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW );
	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_GREATER, 0.50f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ZERO );
	glMatrixMode( GL_TEXTURE );
		glPushMatrix( );
		glLoadIdentity( );
	glMatrixMode( GL_MODELVIEW );

	glPushMatrix( );
		glTranslatef( 0.0f, -1.8f, -4.55f );
		glRotatef( -10.0f, 1.0f, 0.0f, 0.0f );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lightMaterial );
		glMaterialfv( GL_FRONT, GL_SPECULAR, lightMaterial );
		glPushAttrib( GL_CURRENT_BIT );
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			glBindTexture( GL_TEXTURE_2D, lensFlareTexture );
			glBegin( GL_TRIANGLE_STRIP );
				glTexCoord2i( 0, 0 ); glVertex3f( -0.5f, -0.5f, 0.0f );
				glTexCoord2i( 1, 0 ); glVertex3f( 0.5f, -0.5f, 0.0f );	
				glTexCoord2i( 0, 1 ); glVertex3f( -0.5f, 0.5f, 0.0f );	
				glTexCoord2i( 1, 1 ); glVertex3f( 0.5f, 0.5f, 0.0f );	
			glEnd( );
		glPopAttrib( );
	glPopMatrix( );

	glMatrixMode( GL_TEXTURE );
		glPopMatrix( );
	glMatrixMode( GL_MODELVIEW );

	glDisable( GL_ALPHA_TEST );
	glFrontFace( GL_CCW );
	glDisable( GL_CULL_FACE );
}

void loadFishTextures( )
{
	glGenTextures( NUMBER_OF_FISH_TYPES, &fishTexture[ 0 ] );

	for( int i = 0; i < NUMBER_OF_FISH_TYPES; i++ )
	{
		ostringstream oss;
		oss << "assets/fish/fish";
		oss.setf( ios::fixed );
		oss.width( 2 );
		oss.fill( '0' );
		oss << i + 1 << ".tga";

		cout << "Loading " << oss.str() << "..." << endl;

		int r = ImageIO::loadImage(  &fishesTargaImage[ i ], oss.str( ).c_str( ), ImageIO::TARGA );
		assert( r == ImageIO::SUCCESS );

		glBindTexture( GL_TEXTURE_2D, fishTexture[ i ] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		gluBuild2DMipmaps( GL_TEXTURE_2D, (fishesTargaImage[ i ].bitsPerPixel >> 3 == GL_RGB ? 3 : 4 ),
						   fishesTargaImage[ i ].width, fishesTargaImage[ i ].height,
						   GL_RGBA, GL_UNSIGNED_BYTE, fishesTargaImage[ i ].pixelData );
	}


	// free image data...
	for( int i = 0; i < NUMBER_OF_FISH_TYPES; i++ )
		ImageIO::destroyImage( &fishesTargaImage[ i ] );
}

void renderFish( )
{
	static float matrix[ 16 ];
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	glPushMatrix( );

	//glPolygonMode( GL_FRONT, GL_FILL );

	for( int i = 0; i < NUMBER_OF_FISH; i++ )
	{
		float &fSize = Fish[ i ].size;
		

		glPushMatrix( );
			glLoadIdentity( );
			glTranslatef( -Fish[ i ].x, -Fish[ i ].y, -Fish[ i ].z );

			Math::Vector3 forward( Fish[ i ].x - Camera.x, Fish[ i ].y - Camera.y, Fish[ i ].z - Camera.z );
			forward.normalize( );
			//forward.negate( );
			Math::Vector3 &left = crossProduct( forward, Math::Vector3::Y_UNIT );
			left.normalize( );
			Math::Vector3 &up = crossProduct( forward, left );

			matrix[  0 ] = left.X;
			matrix[  1 ] = left.Y;
			matrix[  2 ] = left.Z;
			matrix[  3 ] = 0;

			matrix[  4 ] = up.X;
			matrix[  5 ] = up.Y;
			matrix[  6 ] = up.Z;
			matrix[  7 ] = 0;

			matrix[  8 ] = forward.X;
			matrix[  9 ] = forward.Y;
			matrix[ 10 ] = forward.Z;
			matrix[ 11 ] = 0;

			matrix[ 12 ] = -Fish[ i ].x;
			matrix[ 13 ] = -Fish[ i ].y;
			matrix[ 14 ] = -Fish[ i ].z;
			matrix[ 15 ] = 1;

			glMultMatrixf( matrix );

			glPushAttrib( GL_CURRENT_BIT );
				glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
				glBindTexture( GL_TEXTURE_2D, fishTexture[  Fish[ i ].type ] );

				glBegin( GL_TRIANGLE_STRIP );
					glTexCoord2i( -1, -1 ); glVertex3f( Fish[ i ].x + 2 * fSize, Fish[ i ].y + fSize, Fish[ i ].z );		// Top Right
					glTexCoord2i( 0, -1 ); glVertex3f( Fish[ i ].x - 2 * fSize, Fish[ i ].y + fSize, Fish[ i ].z );		// Top Left
					glTexCoord2i( -1, 0 ); glVertex3f( Fish[ i ].x + 2 * fSize, Fish[ i ].y - fSize, Fish[ i ].z );	// Bottom Right
					glTexCoord2i( 0, 0 ); glVertex3f( Fish[ i ].x - 2 * fSize, Fish[ i ].y - fSize, Fish[ i ].z );	// Bottom Left
				glEnd( );
			glPopAttrib( );
		glPopMatrix( );
	}

	glPopMatrix( );

	glEnable( GL_BLEND );
	glDisable( GL_CULL_FACE );
}

void seedFish( )
{
	for( int i = 0; i < NUMBER_OF_FISH; i++ )
	{
		Fish[ i ].size = 25.0f; //Math::uniformBetween( 1.0f, 2.0f );
		Fish[ i ].type = rand( ) % NUMBER_OF_FISH_TYPES;
		Fish[ i ].y = Math::uniformBetween( 40.0f, 60.0f );


		Fish[ i ].x = Math::uniformBetween( -20.0f, 20.0f );
		Fish[ i ].z = Math::uniformBetween( -20.0f, 20.0f );
	}
}

void onTimedRender( int value )
{
	//render( );
	glutPostRedisplay( );
	glutTimerFunc( MILLIS_PER_FRAME, onTimedRender, 0 );
}
