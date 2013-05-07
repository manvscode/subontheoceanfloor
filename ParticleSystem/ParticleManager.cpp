/*
 *		ParticleManager.cpp
 *
 *		Particle manager for various systems of particles.
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */
#include "ParticleManager.h"

const ParticleManager *ParticleManager::getInstance( )
{
	if( !m_Instance )
		m_Instance = new ParticleManager( );

	return m_Instance;
}

ParticleManager::ParticleManager( )
//: m_ParticleGroups( 8 )
{}

ParticleManager *ParticleManager::m_Instance = 0;
size_t ParticleManager::m_NumberOfParticleGroups = 0;


ParticleManager::~ParticleManager( )
{
	//delete [] m_Particles;
}

//template <size_t MaxSize, class AnyInitializePolicy, class AnyActionPolicy, class AnyParticle>
//void ParticleManager::addParticleGroup( const typename ParticleGroup<MaxSize, AnyInitializePolicy, AnyActionPolicy, AnyParticle> &grp )
//{
//	m_ParticleGroups.push_back( &grp );
//}