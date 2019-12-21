#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__


struct circle_t
{
	vec2	center = vec2(0);		// 2D position for translation
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	timeToRevive = 10.0f;	// time to revive
	float	rotSpeed = 1.0f;		// rotation speed
	float	radius = 1.0f;			// radius of circle
	float	angle = 0.0f;			// to preserve the angle
	float	scaleSpeed = 10.0f;		// speed of sizing up
	float	response_time = 0.1f;	// response time
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float dT);
};

inline float randf()// [0,1) random number
{
	return rand() / float(RAND_MAX);
}

inline float randf(float m, float M)// [m, M) random number
{
	return m + (M - m) * randf();
}

inline std::vector<circle_t> create_circles(float minX, float maxX, float minY, float maxY)
{
	std::vector<circle_t> circles;
	circle_t c;
	for (int i = 0; i < 30; i++) {
		c = { vec2(randf(minX, maxX),randf(minY, maxY))// center
			, false// nowAlive
			, float(i/(3.0f+ i/15))// timeToRevive
			, 10.0f// rotation speed
			, 1.0f// radius
			, randf(0, 2 * PI)// angle
			, 10.0f// scaleSpeed
			, 0.1f// response_time
			,vec4(1.0f,0.2f,0.1f,1.0f)// Color
		};
		circles.emplace_back(c);
	}
	return circles;
}

inline void revive_circle(circle_t &c, float minX, float maxX, float minY, float maxY, float rad)
{
	c.center = vec2(randf(minX, maxX), randf(minY, maxY));// center
	c.nowAlive = true;// nowAlive
	c.radius = rad;// radius
	c.response_time = 0.1f;// response_time
}

inline void circle_t::update(float dT)
{
	angle += dT * rotSpeed;
	radius += scaleSpeed*dT;
	rotSpeed += 0.01f * dT;
	scaleSpeed += radius * 2.0f * dT;
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, radius, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
