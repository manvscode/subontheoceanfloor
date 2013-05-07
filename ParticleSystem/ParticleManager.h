#pragma once
#ifndef __PARTICLEMANAGER_H__
#define __PARTICLEMANAGER_H__
/*
 *		ParticleManager.h
 *
 *		Particle manager for various systems of particles.
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */
#include <vector>
#include "ParticleGroup.h"
#include "Particle.h"


class ParticleManager
{
  public:
	//typedef std::vector< ParticleGroup *> ParticleGroupCollection;

	inline const ParticleManager *getInstance( );
	virtual ~ParticleManager( );

	//inline const size_t getNumberOfGroups( ) const { return m_ParticleGroups.size( ); }
	//template <size_t MaxSize, class AnyInitializePolicy, class AnyActionPolicy, class AnyParticle>
	//void addParticleGroup( const typename ParticleGroup<MaxSize, AnyInitializePolicy, AnyActionPolicy, AnyParticle>  &grp );
	
	//virtual void removeParticleGroup( );
  private:
	ParticleManager( );
	static ParticleManager *m_Instance;
	static unsigned int m_NumberOfParticleGroups;
	//ParticleGroupCollection m_ParticleGroups;
};

#endif