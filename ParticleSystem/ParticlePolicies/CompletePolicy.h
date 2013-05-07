#pragma
#ifndef __COMPLETEPOLICY_H__
#define __COMPLETEPOLICY_H__
/*
 *		CompletePolicy.h
 *
 *		
 *		Coded by Joseph A. Marrero
 *		9/26/05
 */
template <class AnyParticle, class ColorPolicy, class SizePolicy,
		  class VelocityPolicy, class LifePolicy, class PositionPolicy>
class CompletePolicy
{
  public:
	inline void PrepareAction( )
	{
		m_PositionPolicy.PrepareAction( );
		m_ColorPolicy.PrepareAction( );
		m_SizePolicy.PrepareAction( );
		m_VelocityPolicy.PrepareAction( );
		m_LifePolicy.PrepareAction( );
	}

	inline void operator()( AnyParticle &particle )
	{
		m_PositionPolicy( particle );
		m_ColorPolicy( particle );
		m_SizePolicy( particle );
		m_VelocityPolicy( particle );
		m_LifePolicy( particle );
	}
  private:
	PositionPolicy m_PositionPolicy;
	ColorPolicy m_ColorPolicy;
	SizePolicy m_SizePolicy;
	VelocityPolicy m_VelocityPolicy;
	LifePolicy m_LifePolicy;
};



#endif