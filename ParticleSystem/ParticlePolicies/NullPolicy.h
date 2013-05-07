#pragma
#ifndef __NULLPOLICY_H__
#define __NULLPOLICY_H__
/*
 *		NullPolicy.h
 *
 */
template <class AnyParticle = Particle>
class NullPolicy
{
  public:
	inline void PrepareAction( )
	{}

	inline void operator()( AnyParticle &particle )
	{}

	//inline virtual ~InitializePolicy( void ){}
};

#endif