#include "cgmath.h"			// slee's simple math library
#pragma once
#ifndef __HERO_H__
#define __HERO_H__


struct hero_t
{
	vec2	center = vec2(0);		// 2D position for translation
	vec2	destination = vec2(0);	// 2D position of destination of skill
	bool	hasDestination = false;	// to check if it has destination now
	bool	isSkillPlay = false;	// to check if it's skill is now used
	bool	isSkillOn = true;		// to check if it's skill is now available
	bool	isAttackOn = true;		// to check if it's attack is now available
	float	radius = 1.0f;			// radius of hit circle, it affects it's size
	float 	movingSpeed = 1.0f;		// control the moving speed
	float	skillCoolTime = 2.0f;	// skill cool time
	float	restSkillCool = 2.0f;	// rest skill cool time 
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	update(float dT, float X, float Y);
};

inline hero_t create_hero()
{
	hero_t hero = { vec2(0,0)// center
		, vec2(0,0)// destination
		, false// hasDestination
		, false// isSkillPlay
		, false// isSkillOn
		, false// isAttackOn
		, 1.0f// radius
		, 1.0f// movingSpeed
		, 2.0f// skillCoolTime
		, 2.0f// restSkillCool
		,vec4(1.f)// Color
		,mat4::identity()// modeling transformation
	};

	return hero;
}

inline void hero_t::update(float dT, float X, float Y)
{
	center.x += dT * 15.0f * X;
	center.y += dT * 15.0f * Y;

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix = mat4::scale(vec3(radius, radius, 1));

	mat4 rotation_matrix = mat4::identity();
	// mat4::rotate(vec3(0.0f, 0.0f, 1.0f), 0.0f);

	mat4 translate_matrix = mat4::translate(vec3(center, 0));

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
