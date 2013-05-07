#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "glutility.h"



namespace GL_UTILITY {

bool HasErrorOccured( )
{
	GLenum error = glGetError( );

	return error != GL_NO_ERROR;
}

void renderLightSource( float position[], float direction[], float color[] )
{
	//glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );

	glPushMatrix( );
		//glLoadIdentity( );
		glTranslatef( position[ 0 ], position[ 1 ], position[ 3 ] );


		glPushAttrib( GL_CURRENT_BIT );
			glColor4f( color[ 0 ], color[ 1 ], color[ 2 ], 1.0f );
			glLineWidth( 3.0f );

			glBegin( GL_LINES );
				glVertex3f( direction[ 0 ], direction[ 1 ], direction[ 2 ] );
				glVertex3f( direction[ 0 ] * 100.0f, direction[ 1 ] * 100.0f, direction[ 2 ] * 100.0f );
			glEnd( );
		glPopAttrib( );

	glPopMatrix( );

	//glEnable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );
}

} // end of namespace