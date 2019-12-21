#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__


struct explosion_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	source = vec2(0);		// 2D position of source
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	timeOfLife = 0.3f;		// time of life
	float	rotSpeed = 50.0f;		// rotation speed
	float	radius = 0.2f;			// radius of circle
	float	angle = 0.0f;			// to preserve the angle
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float dT);
};

inline float randf();// [0,1) random number


inline float randf(float m, float M);// [m, M) random number


inline std::vector<explosion_t> create_explosion_particles(int numOfParticles)
{
	std::vector<explosion_t> particles;
	explosion_t p;
	for (int i = 0; i < numOfParticles; i++) {
		p = { vec2(0.0f)// center
			, vec2(0.0f)// sourceP
			, false// nowAlive
			, 0.5f// timeOfLife
			, 100.0f// rotation speed
			, 0.2f// radius
			, randf(0, 2 * PI)// angle
			,vec4(1.0f,0.5f,0.5f,1.0f)// Color
		};
		particles.emplace_back(p);
	}
	return particles;
}

inline void setParticle(explosion_t& p, float targetX, float targetY, float range, vec4 color)
{
	p.center = vec2(randf(targetX - range, targetX + range), randf(targetY - range, targetY + range));// center
	p.source = vec2(targetX, targetY);// source pos
	p.angle = randf(0, 2 * PI);// angle
	p.nowAlive = true;// nowAlive
	p.timeOfLife = 0.5f;
	p.color = color;
	p.color.w *= 0.4f;
}

inline void explosion_t::update(float dT)
{
	center += (center - source) * 10.0f *dT;
	angle += dT * rotSpeed;
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, radius, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif