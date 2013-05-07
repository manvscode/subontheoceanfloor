#ifndef _OBJLOADER_H_
#define _OBJLOADER_H_
/*
 *	OBJLoader.h
 *
 *	An OBJ model loader.
 *
 *	Coded by Joseph A. Marrero. 12/22/06
 *	http://www.l33tprogramer.com/
 */
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "Math/Vertex.h"
using namespace std;
using namespace MATH;




class COBJLoader
{
  public:
	typedef struct tagOBJVertex {
		float x; float y; float z;
	} OBJVertex;

	typedef struct tagOBJTextureCoord {
		float u; float v;
	} OBJTextureCoord;

	typedef struct tagOBJNormal {
		float nx; float ny; float nz;
	} OBJNormal;

	typedef struct tagOBJGroup {
		string groupName;
		vector<CompleteVertex> mesh;
	} OBJGroup;

  protected:
	typedef enum tagElement {
		ELM_VERTEX,
		ELM_TEXTURECOORD,
		ELM_NORMAL,
		ELM_FACE,
		ELM_GROUP,
		ELM_OTHER
	} Element;

	void setElement( char *token );
	void addVertex( );
	void addTextureCoord( );
	void addNormal( );
	void addFace( );
	void addGroup( );
	
  public:
	explicit COBJLoader( char *filename, bool verbose = false );
	explicit COBJLoader( string &filename, bool verbose = false  );
	~COBJLoader( );

	
	bool load( );
	void clear( );
	vector<OBJGroup> &getMesh( );
	vector<OBJGroup> getMesh( ) const;

  protected:
	void addDefaultGroup( );
	ifstream m_File;

	vector<OBJVertex> m_Vertices;
	vector<OBJTextureCoord> m_TextureCoords;
	vector<OBJNormal> m_Normals;
	map<string, vector<CompleteVertex> *> m_GroupMap;
	vector<OBJGroup> m_Groups;

	Element m_CurrentElement;
	string m_CurrentGroup;
	bool m_bVerboseOutput;

};

inline vector<COBJLoader::OBJGroup> &COBJLoader::getMesh( )
{ return m_Groups; }

inline vector<COBJLoader::OBJGroup> COBJLoader::getMesh( ) const
{ return m_Groups; }

#endif