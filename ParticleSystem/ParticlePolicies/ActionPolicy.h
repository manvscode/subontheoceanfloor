#pragma once
#ifndef __ACTIONPOLICY_H__
#define __ACTIONPOLICY_H__
/*
 *		ActionPolicy.h
 *
 *		An interface for describing how to do certain actions
 *		on particles.
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */

template <class AnyParticle = Particle>
class ActionPolicy
{
  public:
	inline virtual void operator()( AnyParticle &part ) = 0;
	virtual ~ActionPolicy( void ){}

	inline virtual void PrepareAction( ) = 0;
};

#endif