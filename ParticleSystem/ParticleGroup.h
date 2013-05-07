#pragma once
#ifndef __PARTICLEGROUP_H__
#define __PARTICLEGROUP_H__
/*
 *		ParticleGroup.h
 *
 *		Particle group that groups systems of particles. In order
 *		to keep systems seperate.
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */
#include <ctime>
#include <MathLib.h>
#include "ParticlePolicies/InitializePolicy.h"
#include "ParticlePolicies/ActionPolicy.h"
#include "Particle.h"

using namespace Math;

template <size_t MaxSize = 128,
		  class AnyInitializePolicy = InitializePolicy,
		  class AnyActionPolicy = ActionPolicy,
		  class AnyParticle = Particle>
class ParticleGroup
{
  public:
	ParticleGroup( )
	{
		m_Particles = new AnyParticle[ MaxSize ];
		m_NumberOfParticles = 0;
	}
	virtual ~ParticleGroup( ){ delete [] m_Particles; }

	
	void Emits( const size_t &amount, const Math::Vertex3 &position )
	{
		size_t nAmount = amount;
		if( ( getParticlesCount( ) + nAmount ) > getMaxParticles( ) ) 
			nAmount = getMaxParticles( ) - getParticlesCount( );
		
		if( nAmount > 0 )
		{
			size_t cnt = m_NumberOfParticles;
			m_NumberOfParticles += nAmount;

			for( ; cnt < m_NumberOfParticles; ++cnt )
			{
				m_Particles[ cnt ].m_ptPosition = position;
				m_InitializePolicy( m_Particles[ cnt ] );
			}
		}
	}

	/*
	void Emits( const size_t &amount, const C3DLine<float> &line )
	{
		size_t nAmount = amount;
		if( ( getParticlesCount( ) + nAmount ) > getMaxParticles( ) ) 
			nAmount = getMaxParticles( ) - getParticlesCount( );
		
		srand( (unsigned)time( NULL ) );

		if( nAmount > 0 )
		{
			size_t cnt = m_NumberOfParticles;
			m_NumberOfParticles += nAmount;
			srand( (unsigned) time( NULL ) );
			for( ; cnt < m_NumberOfParticles; ++cnt )
			{
				
				int r = (int) 10 * rand()/RAND_MAX;
				m_Particles[ cnt ].m_ptPosition = line.getPoint( r );
				m_InitializePolicy( m_Particles[ cnt ] );
			}
		}
	}
	*/

	//void Emits( const size_t &amount, const C3DPlane<float> &position )
	
	/*
	void Emits( const size_t &amount, const C3DBox<float> &position )
	{
		size_t nAmount = amount;
		if( ( getParticlesCount( ) + nAmount ) > getMaxParticles( ) ) 
			nAmount = getMaxParticles( ) - getParticlesCount( );

		if( nAmount > 0 )
		{
			size_t cnt = m_NumberOfParticles;
			m_NumberOfParticles += nAmount;

			for( ; cnt < m_NumberOfParticles; ++cnt )
			{
				m_Particles[ cnt ].m_ptPosition = position;
				m_InitializePolicy( m_Particles[ cnt ] );
			}
		}
	}
	*/

	//void Emits( const size_t &amount, const C3DSphere<float> &position )

	inline void Clear( ) { m_NumberOfParticles = 0; }
	inline const size_t getParticlesCount( ) const { return m_NumberOfParticles; }
	inline const size_t getMaxParticles( ) const { return MaxSize; }

	inline const AnyParticle *getParticles( ) const
	{
		if( getParticlesCount( ) == 0 )
			return 0;

		return m_Particles;
	}

	void Update( )
	{
		m_ActionPolicy.PrepareAction( );

		// kill off dead particles
		for( size_t cnt = 0; cnt < m_NumberOfParticles; )
		{
			m_ActionPolicy( m_Particles[ cnt ] );

			if( m_Particles[ cnt ].m_nLifeSpan <= 0 )
			{
				m_Particles[ cnt ] = m_Particles[ m_NumberOfParticles - 1 ];
				#if _DEBUG
					memset( &m_Particles[ m_NumberOfParticles - 1 ], 0, sizeof(Particle) );
				#endif
				m_NumberOfParticles--;
			}
			else 
				cnt++;
		}
	}

	void UpdateAndRespawn( )
	{
		m_ActionPolicy.PrepareAction( );

		// kill off dead particles
		for( size_t cnt = 0; cnt < m_NumberOfParticles; )
		{
			m_ActionPolicy( m_Particles[ cnt ] );

			if( m_Particles[ cnt ].m_nLifeSpan <= 0 )
			{
				m_InitializePolicy( m_Particles[ cnt ] );
			}
			else 
				cnt++;
		}
	}


  private:
		AnyInitializePolicy m_InitializePolicy;
		AnyActionPolicy m_ActionPolicy;
		size_t m_NumberOfParticles;
		AnyParticle *m_Particles;
};
#endif