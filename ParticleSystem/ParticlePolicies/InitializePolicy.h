#pragma once
#ifndef __INITIALIZEPOLICY_H__
#define __INITIALIZEPOLICY_H__
/*
 *		InitializePolicy.h
 *
 *		An interface for describing how to initialize particles.
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */
template <class AnyParticle = Particle>
class InitializePolicy
{
  public:
	inline virtual void operator()( AnyParticle &part ) = 0;
	inline virtual ~InitializePolicy( void ){}
};

#endif