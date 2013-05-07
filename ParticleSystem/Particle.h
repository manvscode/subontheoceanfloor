#pragma once
#ifndef __PARTICLE_H__
#define __PARTICLE_H__
/*
 *		Particle.h
 *
 */
#include <MathLib.h>
#include <Vector3.h>

typedef struct tagColor {
	float m_Red,
		  m_Green,
		  m_Blue,
		  m_Alpha;
} Color;

typedef struct tagSize3D {
	float radius;
} Size3D;

typedef struct tagParticle {
	Math::Vertex3 m_ptPosition;
	Math::Vector3 m_vVelocity;
	Color m_Color;
	float m_Size;
	int m_nLifeSpan;
} Particle;

#endif