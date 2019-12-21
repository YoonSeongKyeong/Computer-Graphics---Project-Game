#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __SQUARE_H__
#define __SQUARE_H__


struct square_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	destination = vec2(0);	// 2D position of destination
	bool	hasDestination = false;	// to check if it has destination now
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	angle = 0.0f;			// to preserve the angle
	float	radius = 1.0f;			// radius of hit circle, it affects it's size
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float dT, float angle);
	bool	hasDestination() { return hasDestination; }
	bool	nowAlive() { return nowAlive; }
};

inline std::vector<square_t> create_squares()
{
	std::vector<square_t> squares;
	square_t c;
	c = { vec2(0,0)// center
		, vec2(1,0)// destination
		, false// hasDestination
		, false// nowAlive
		, 0.0f// angle
		, 1.0f// radius
		,vec4(1.0f,0.5f,0.5f,1.0f)// Color
	};
	squares.emplace_back(c);
}

inline void square_t::update(float dT, float angle)
{

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, radius, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
