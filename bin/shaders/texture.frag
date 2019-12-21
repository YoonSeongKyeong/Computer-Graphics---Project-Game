#version 330

// input from vertex shader
in vec4 epos;
in vec3 norm;
in vec2 tc;

// the only output variable
out vec4 fragColor;

uniform mat4	view_matrix;
uniform vec4	light_position, Ia, Id, Is;	// light
uniform vec4	Ka, Kd, Ks;					// material properties
uniform float	shininess;
uniform vec4	solid_color;
uniform int		color_mode;

// texture sampler
uniform sampler2D TEX0;
uniform sampler2D TEX1;
uniform bool	  isTex0;

vec4 phong( vec3 l, vec3 n, vec3 h, vec4 Kd )
{
	vec4 Ira = Ka*Ia;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is,0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	// light position in the eye space
	vec4 lpos = view_matrix*light_position;

	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz-(lpos.a==0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l+v);	// the halfway vector

	vec4 Kd = isTex0?texture2D( TEX0, tc ) : texture2D( TEX1, tc );
	fragColor = (color_mode == 0 ? (solid_color*0.10f +  phong( l, n, h, Kd )*1.30f) : solid_color);	// apply original color
	fragColor.w = solid_color.w;
	fragColor = (color_mode != 2 ? fragColor : vec4(1.f));
}
