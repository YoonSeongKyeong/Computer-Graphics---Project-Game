#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__


struct triangle_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	destination = vec2(0);	// 2D position of destination
	vec2	delta = vec2(0);		// 2D position of delta
	bool	hasDestination = false;	// to check if it has destination now
	bool	nowAlive = false;		// to preserve the constant number of object pool
	float	timeToRevive = 10.0f;	// time to revive
	float	radius = 1.0f;			// radius of hit circle, it affects it's size
	float	angle = 0.0f;			// angle of rotation
	float	speed = 10.0f;			// speed factor
	float	response_time = 0.1f;	// response time
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float delT);
};

inline float randf();// [0,1) random number

inline float randf(float m, float M);// [m, M) random number


inline std::vector<triangle_t> create_triangles(float minX, float maxX, float minY, float maxY)
{
	std::vector<triangle_t> triangles;
	triangle_t t;
	for (int i = 0; i < 30; i++) {
		t = { vec2(randf(minX, maxX),randf(minY, maxY))// center
		, vec2(1.0f,0)// destination
		, vec2(1.0f,0)// delta
		, false// hasDestination
		, false// nowAlive
		, float(i/(2.0f+  i/15))// timeToRevive
		, 1.5f// radius
		, 0.0f// angle
		, 10.0f// speed
		, 0.1f// response_time
		,vec4(0.0f,1.0f,0.8f,1.0f)// Color
		};
		triangles.emplace_back(t);
	}
	return triangles;
}


inline void revive_triangle(triangle_t &t, float minX, float maxX, float minY, float maxY)
{
	t.center = vec2(randf(minX, maxX), randf(minY, maxY));// center
	t.hasDestination = false;// hasDestination
	t.nowAlive = true;// nowAlive
	t.response_time = 0.1f;// response_time
}

inline void triangle_t::update(float delT)
{
	center += delT * delta * speed;
	speed += 0.1f;
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius*0.8f, radius * 0.8f, 1));

	mat4 rotation_matrix = mat4::rotate(vec3(0.0f, 0.0f, 1.0f), angle );

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
