#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __BULLET_H__
#define __BULLET_H__


struct bullet_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	delta = vec2(0);		// 2D position of delta
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	radius = 1.0f;			// radius of circle
	float	angle = 0.0f;			// to preserve the angle
	float	speed = 1.0f;					// speed of bullet
	float	life = 1.0f;					// lifetime of bullet
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix; 			// modeling transformation

	// public functions
	void	update(float dT);
};


inline std::vector<bullet_t> create_bullets()
{
	std::vector<bullet_t> bullets;
	bullet_t b;
	for (int i = 0; i < 8; i++) {// it defines the number of total bullets 
		b = { vec2(0)// center
			, vec2(0)// delta
			, false// nowAlive
			, 1.0f// radius
			, 0.0f// angle
			, 300.0f// speed
			, 0.2f// lifetime
			,vec4(1.0f,1.0f,0.0f,1.0f)// Color
		};
		bullets.emplace_back(b);
	}
	return bullets;
}

inline void revive_bullet(bullet_t &b, float x, float y)
{
	b.center = vec2(x,y);// center
	b.nowAlive = true;// nowAlive
	b.life = 0.20f;
}

inline void bullet_t::update(float dT)
{
	center += dT * delta * speed;
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, radius, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif