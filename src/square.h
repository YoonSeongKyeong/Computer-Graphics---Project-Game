#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __SQUARE_H__
#define __SQUARE_H__


struct square_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	destination = vec2(0);	// 2D position of destination
	vec2	delta = vec2(0);		// 2D position of delta
	bool	hasDestination = true;	// to check if it has destination now
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	timeToRevive = 10.0f;	// time to revive
	float	phase = 10.0f;			// time to control attack phase, [-10 - alpha, 10 + alpha]
	float	radius = 1.0f;			// radius of hit circle, it affects it's size
	float	angle = 0.0f;			// to preserve the angle
	float	phaseSpeed = 1.0f;		// speed of phase
	float	length = 1.0f;			// length of square
	float	response_time = 0.1f;	// response time
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float dT);
};

inline float randf();// [0,1) random number

inline float randf(float m, float M);// [m, M) random number

inline std::vector<square_t> create_squares(float minX, float maxX, float minY, float maxY)
{
	std::vector<square_t> squares;
	square_t s;
	for (int i = 0; i < 30; i++) {
		s = { vec2(randf(minX, maxX),randf(minY, maxY))// center
			, vec2(1,0)// destination
			, vec2(1.0f,0)// delta
			, true// hasDestination
			, false// nowAlive
			, float(i/(3.0f+i/15)) // timeToRevive
			, 10.0f// phase
			, 1.0f// radius
			, 0.0f// angle
			, 10.0f// phaseSpeed
			, 1.0f// length
			, 0.1f// response time
			,vec4(0.6f,0.3f,0.9f,1.0f)// Color
		};
		squares.emplace_back(s);
	}
	return squares;
}

inline void revive_square(square_t &s, float minX, float maxX, float minY, float maxY)
{
	s.center = vec2(randf(minX, maxX), randf(minY, maxY));// center
	s.hasDestination = true;// hasDestination
	s.nowAlive = true;// nowAlive
	s.phase = 10.0f;// reset phase
	s.length = 1.0f;// reset length
	s.response_time = 0.1f;// reset response_time
}

inline void square_t::update(float dT)
{
	phase -= dT*phaseSpeed;
	length = max(radius, -phase * 30.0f);
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, length, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
