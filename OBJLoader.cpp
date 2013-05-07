/*
 *	OBJLoader.cpp
 *
 *	An OBJ model loader.
 *
 *	Coded by Joseph A. Marrero. 12/22/06
 *	http://www.l33tprogramer.com/
 */
#include <iostream>
#include <sstream>
#include <cassert>
#include "OBJLoader.h"

COBJLoader::COBJLoader( char *filename, bool verbose )
: m_File(), m_CurrentElement(ELM_OTHER), m_bVerboseOutput(verbose)
{
	m_File.open( filename, ios::in );


}

COBJLoader::COBJLoader( string &filename, bool verbose )
: m_File(), m_CurrentElement(ELM_OTHER), m_bVerboseOutput(verbose)
{
	m_File.open( filename.c_str(), ios::in );

}

COBJLoader::~COBJLoader( )
{
	m_File.close( );
}

bool COBJLoader::load( )
{
	string line("");

	if( m_File.fail( ) ) 
		return false;

	// read in the file...
	while( !m_File.eof( ) )
	{
		getline( m_File, line );
		if( line.empty( ) ) continue; // avoid empty lines...

		char *token = strtok( const_cast<char *>( line.c_str() ), " " );
		setElement( token );


		switch( m_CurrentElement )
		{
			case ELM_VERTEX:
				addVertex( );
				break;
			case ELM_NORMAL:
				addNormal( );
				break;
			case ELM_TEXTURECOORD:
				addTextureCoord( );
				break;
			case ELM_FACE:
				addFace( );
				break;
			case ELM_GROUP:
				addGroup( );
				break;
			case ELM_OTHER:
			default:
				if( m_bVerboseOutput )
					cout << "Unknown element; Skipping line...\n\t" << line << endl;
				continue;
				break;
		}
	}

	map<string, vector<CompleteVertex> *>::iterator itr;

	for( itr = m_GroupMap.begin( ); itr != m_GroupMap.end( ); ++itr )
	{
		OBJGroup g;

		g.groupName = itr->first;
		g.mesh = *(itr->second);

		delete itr->second;
		m_Groups.push_back( g );
	}

	return true;
}

void COBJLoader::clear( )
{
	m_Groups.clear( );
}

void COBJLoader::setElement( char *token )
{

	if( strncmp( token, "vt", 2 ) == 0 )
		m_CurrentElement = ELM_TEXTURECOORD;
	else if( strncmp( token, "vn", 2 ) == 0 )
		m_CurrentElement = ELM_NORMAL;
	else if( strncmp( token, "v", 1 ) == 0 )
		m_CurrentElement = ELM_VERTEX;
	else if( strncmp( token, "f", 1 ) == 0 )
		m_CurrentElement = ELM_FACE;
	else if( strncmp( token, "g", 1 ) == 0 )
		m_CurrentElement = ELM_GROUP;
	else
		m_CurrentElement = ELM_OTHER;
}

/*
 *	Vertices come first so
 */
void COBJLoader::addVertex( )
{
	OBJVertex v;

	char *token = strtok( NULL, " " );
	v.x = atof( token );

	token = strtok( NULL, " " );
	v.y = atof( token );

	token = strtok( NULL, " " );
	v.z = atof( token );

	m_Vertices.push_back( v );
}

void COBJLoader::addTextureCoord( )
{
	OBJTextureCoord vt;

	char *token = strtok( NULL, " " );
	vt.u = atof( token );

	token = strtok( NULL, " " );
	vt.v = atof( token );

	m_TextureCoords.push_back( vt );
}

void COBJLoader::addNormal( )
{
	OBJNormal n;

	char *token = strtok( NULL, " " );
	n.nx = atof( token );

	token = strtok( NULL, " " );
	n.ny = atof( token );

	token = strtok( NULL, " " );
	n.nz = atof( token );

	m_Normals.push_back( n );
}

void COBJLoader::addFace( )
{
	char *token = strtok( NULL, " " );
	unsigned int vIndex = 0;
	unsigned int tIndex = 0;
	unsigned int nIndex = 0;

	if( m_CurrentGroup.compare("") == 0 ) // check if there are no groups;
		addDefaultGroup( ); // if none create a default one...

	vector<CompleteVertex> *mesh = m_GroupMap[ m_CurrentGroup ];

	while( token != NULL )
	{
		char garbage;
		string tok( token );
		istringstream iss( tok );

		iss >> vIndex  >> garbage >> tIndex >> garbage >> nIndex;

		CompleteVertex v;
		v.x = m_Vertices[ vIndex - 1 ].x;
		v.y = m_Vertices[ vIndex - 1 ].y;
		v.z = m_Vertices[ vIndex - 1 ].z;

		v.u = m_TextureCoords[ tIndex - 1 ].u;
		v.v = m_TextureCoords[ tIndex - 1 ].v;

		v.nx = m_Normals[ nIndex - 1 ].nx;
		v.ny = m_Normals[ nIndex - 1 ].ny;
		v.nz = m_Normals[ nIndex - 1 ].nz;

		// set color to gray...
		v.r = 0.5f;
		v.g = 0.5f;
		v.b = 0.5f;
		v.a = 0.0f;

		mesh->push_back( v );

		token = strtok( NULL, " " );
	}
}

void COBJLoader::addGroup( )
{
	string groupName;
	char *token = strtok( NULL, " " );


	if( strncmp( token, "default", 7 ) == 0 ) // i hate this group
		return;

	// concatenate all tokens to form group name.
	while( token != NULL )
	{
		groupName += token;
		groupName += " ";
		token = strtok( NULL, " " );
	}

	groupName = groupName.substr( 0, groupName.length() - 1 );


	m_GroupMap[ groupName ] = new vector<CompleteVertex>( );
	m_CurrentGroup = groupName;
}

void COBJLoader::addDefaultGroup( )
{
	string groupName( "default" );
	m_GroupMap[ groupName ] = new vector<CompleteVertex>( );
	m_CurrentGroup = groupName;
}