#ifndef __TRACKBALL_H__
#define __TRACKBALL_H__
#include "cgmath.h"

#define NOT_TRACKING 0
#define ROTATION_TRACKING_MOD 1
#define PANNING_TRACKING_MOD 2
#define ZOOMING_TRACKING_MOD 3

#define SENSITIVITY_MODE_SUPER_ACCURATE 0
#define SENSITIVITY_MODE_ACCURATE 1
#define SENSITIVITY_MODE_FAST 2
#define SENSITIVITY_MODE_SUPER_FAST 3

#define CONTROLLER_ROT 0
#define CONTROLLER_PAN 1
#define CONTROLLER_ZOOM 2
#define CONTROLLER_SCR_ZOOM 3

struct trackball
{
	int		b_tracking_mod = NOT_TRACKING;							// button tracking mode
	mat4	view_matrix0;											// initial view matrix
	vec3	eye0;													// initial eye vector
	vec2	m0;														// the last mouse position
	float	rot_sensitivity = 1.5f;									// sensitivity of rotation
	int		rot_sens_mod = SENSITIVITY_MODE_FAST;					// sensitivity mod of rotation
	float	pan_sensitivity = 7.5f;									// sensitivity of panning
	int		pan_sens_mod = SENSITIVITY_MODE_FAST;					// sensitivity mod of panning
	float	zoom_sensitivity = 7.5f;								// sensitivity of zooming
	int		zoom_sens_mod = SENSITIVITY_MODE_FAST;					// sensitivity mod of zooming
	float	sc_zoom_sensitivity = 2.0f;								// sensitivity of scroll zooming
	int		sc_zoom_sens_mod = SENSITIVITY_MODE_FAST;				// sensitivity mod of scroll zooming
	mat4	total_rotation0 = mat4::identity();						// saved total rotation matrix
	mat4	total_rotation = mat4::identity();						// total rotation matrix

	void setView(int view) {// 0: top view, 1: front view
		if (view == 0) {
			total_rotation = mat4::identity();
			return;
		}
		else if (view == 1) {
			total_rotation = mat4::rotate(vec3(1.0f, 0.0f, 0.0f), -PI / 2.0f);
			return;
		}
	}

	trackball(){}
	int get_tracking_mod() const { return b_tracking_mod; }
	void begin( const mat4& view_matrix, float x, float y, const vec3& eye, int tracking_mod)
	{
		b_tracking_mod = tracking_mod;			// enable trackball tracking
		m0 = vec2(x,y)*2.0f-1.0f;	// convert (x,y) in [0,1] to [-1,1]
		view_matrix0 = view_matrix;	// save current view matrix
		eye0 = eye;					// save current eye matrix
		switch (b_tracking_mod) {
		case NOT_TRACKING:
			return;
		case ROTATION_TRACKING_MOD:
			total_rotation0 = total_rotation;
			break;
		case PANNING_TRACKING_MOD:
			break;
		case ZOOMING_TRACKING_MOD:
			break;
		}
	}
	void end() { 
		switch (b_tracking_mod) {
		case NOT_TRACKING:
			return;
		case ROTATION_TRACKING_MOD:
			break;
		case PANNING_TRACKING_MOD:
			break;
		case ZOOMING_TRACKING_MOD:
			break;
		}
		b_tracking_mod = NOT_TRACKING;
	}

	void toggle_sensitivity(int controller_type) {
		int mod_of_sensitivity=-1;
		switch (controller_type) {
		case CONTROLLER_ROT:
			rot_sens_mod = (rot_sens_mod + 1) % 4;
			rot_sensitivity = (rot_sens_mod == 0) ? rot_sensitivity/ 8.f : rot_sensitivity * 2.f;
			mod_of_sensitivity = rot_sens_mod;
			break;
		case CONTROLLER_PAN:
			pan_sens_mod = (pan_sens_mod + 1) % 4;
			pan_sensitivity = (pan_sens_mod == 0) ? pan_sensitivity/8.f : pan_sensitivity * 2.f;
			mod_of_sensitivity = pan_sens_mod;
			break;
		case CONTROLLER_ZOOM:
			zoom_sens_mod = (zoom_sens_mod + 1) % 4;
			zoom_sensitivity = (zoom_sens_mod == 0) ? zoom_sensitivity/8.f : zoom_sensitivity * 2.f;
			mod_of_sensitivity = zoom_sens_mod;
			break;
		case CONTROLLER_SCR_ZOOM:
			sc_zoom_sens_mod = (sc_zoom_sens_mod + 1) % 4;
			sc_zoom_sensitivity = (sc_zoom_sens_mod == 0) ? sc_zoom_sensitivity/8.f : sc_zoom_sensitivity * 2.f;
			mod_of_sensitivity = sc_zoom_sens_mod;
			break;
		}
		switch (mod_of_sensitivity) {
		case SENSITIVITY_MODE_SUPER_ACCURATE:
			printf("super accurate mode\n");
			break;
		case SENSITIVITY_MODE_ACCURATE:
			printf("accurate mode\n");
			break;
		case SENSITIVITY_MODE_FAST:
			printf("fast mode\n");
			break;
		case SENSITIVITY_MODE_SUPER_FAST:
			printf("super fast mode\n");
			break;
		}
	}

	mat4 get_total_rotation() {
		return total_rotation;
	}

	void update_rot( float x, float y )
	{
		// retrive the current mouse position
		vec2 m = vec2(x,y)*2.0f - 1.0f; // normalize xy
		vec3 p1 = vec3(m.x - m0.x, m0.y - m.y, 0);	// displacement with vertical swap

		// project a 2D mouse position to a unit sphere
		static const vec3 p0 = vec3(0, 0, 1.0f);	// reference position on sphere
		if (length(p1) < 0.0001f) return;			// ignore subtle movement
		p1 *= rot_sensitivity;														// apply rotation scale
		p1 = vec3(p1.x, p1.y, sqrtf(max(0, 1.0f - length2(p1)))).normalize();	// back-project z=0 onto the unit sphere

		// find rotation axis and angle
		// - right-mult of mat3 = inverse view rotation to world
		// - i.e., rotation is done in the world coordinates
		vec3 n = p0.cross(p1) * mat3(view_matrix0);
		float angle = asin(min(n.length(), 1.0f));
		total_rotation = total_rotation0 * mat4::rotate(n.normalize(), angle);
	}
	void update_pan( float x, float y, vec3 &eye, vec3 &at) {
		// retrive the current mouse position
		vec2 m = vec2(x, y) * 2.0f - 1.0f; // normalize xy
		vec2 delta = vec2(m0.x - m.x, m.y - m0.y);	// displacement to delta
		delta *= pan_sensitivity;
		at.x = eye0.x + delta.x;
		at.y = eye0.y + delta.y;
		eye.x = eye0.x + delta.x;
		eye.y = eye0.y + delta.y;
	}
	void update_zoom( float x, float y, vec3 &eye ) {
		// retrive the current mouse position
		vec2 m = vec2(x, y) * 2.0f - 1.0f; // normalize xy
		float zoom_amt = (m0.y - m.y)*zoom_sensitivity;	// displacement to delta
		
		eye.z = max(0.0f, eye0.z + zoom_amt);
	}
	void scroll_zoom(float delta_Y, vec3& eye) {// scroll zoom handler
		float zoom_amt = - (delta_Y) * sc_zoom_sensitivity;	// displacement to delta
		eye.z = max(0.0f, eye.z + zoom_amt);
	}
};

#endif // __TRACKBALL_H__
