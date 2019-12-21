#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "trackball.h"		// virtual trackball
#include "circle.h"			// circle class definition
#include "triangle.h"		// triangle class definition
#include "square.h"			// square class definition
#include "hero.h"			// hero class definition
#include "bullet.h"			// bullet class definition
#include "explosion.h"		// explosion particle class definition
#include "irrKlang\irrKlang.h"
#pragma comment(lib, "irrKlang.lib")
//*************************************
// include stb_image with the implementation preprocessor definition
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//*******************************************************************
// global constants
static const char* window_name = "time attack";
static const char* vert_shader_path = "../bin/shaders/texture.vert";
static const char* frag_shader_path = "../bin/shaders/texture.frag";
//static const char* mesh_vertex_path = "../bin/mesh/name.vertex.bin";// for later possible 3d mesh loading
//static const char* mesh_index_path = "../bin/mesh/name.index.bin";// for later possible 3d mesh loading	
static const char* galaxy_texture_path = "../bin/mesh/galaxy.jpg";
static const char* thunder_texture_path = "../bin/mesh/thunder.jpg";
static const char* bullet_sound_path = "../bin/bullet.wav";
static const char* background_music_path = "../bin/backgroundMusic.mp3";
static const char* hit_sound_path = "../bin/hit_sound.mp3";
static const char* end_sound_path = "../bin/end.wav";
static const char* t_d_sound_path = "../bin/triangle_die.wav";
static const char* c_d_sound_path = "../bin/circle_die.wav";
static const char* s_d_sound_path = "../bin/square_die.wav";
static const char* time_pause_sound_path = "../bin/time_pause.mp3";
uint				NUM_TESS = 32;		// tessellation factor of the circle as a polygon

//*******************************************************************
// irrKlang objects
irrklang::ISoundEngine* engine;
irrklang::ISoundSource* bullet_src = nullptr;
irrklang::ISoundSource* background_music_src = nullptr;
irrklang::ISoundSource* hit_sound_src = nullptr;
irrklang::ISoundSource* end_sound_src = nullptr;
irrklang::ISoundSource* t_d_sound_src = nullptr;
irrklang::ISoundSource* c_d_sound_src = nullptr;
irrklang::ISoundSource* s_d_sound_src = nullptr;
irrklang::ISoundSource* time_pause_sound_src = nullptr;

//*******************************************************************
// common structures
struct camera
{
	vec3	eye = vec3(0, 0, 100);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 1, 0);
	mat4	view_matrix = mat4::look_at(eye, at, up);
	float	fovy = PI / 4.0f; // must be in radian
	float	aspect_ratio = 0.3f;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};

struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, -1.0f, 0.0f);   // directional light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 1000.0f;
};

//*******************************************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = ivec2(1024, 576);	// initial window size

//*******************************************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_buffer = 1;	// ID holder for vertex buffer
GLuint	index_buffer = 1;	// ID holder for index buffer
GLuint	GALAXY_TEX_SRC = 0, WOOD_TEX_SRC = 1, HORZ = 0, VERT = 0;	// texture objects
GLuint	fbo = 0;			// framebuffer objects

//*******************************************************************
// global variables
int		frame = 0;						// index of rendering frames
ivec2	image_size;
vec2	sigma = 0.5f;					// amount of blur
float	delT = 0.0f;					// delta time
float	prevT = 0.0f;					// for calcuating delT
float	t = 0.0f;						// current simulation parameter
bool	b_index_buffer = true;			// use index buffering?
bool	b_wireframe = false;			// use wireframe?
bool	isGameFinished = false;			// check if game is finished
bool	isEasyMode = false;				// check if game mode is easy
bool	isHardMode = false;				// check if game mode is hard
bool	isHelpMode = true;				// check if game mode is help				
bool	attackReq = false;				// flag representing about attack request
bool	skillReq = false;				// flag representing about skill request
bool	isAttacked = false;				// check if hero is attacked
bool	isImmortal = false;				// check if hero is immortal
bool	isTimeFreeze = false;				// check if time is freezed
float	freezeRemain = 0.0f;			// the amount of time freeze
int		numMoving = 0;					// the flag to find if hero is moving
vec2	attackPos;						// the attack position
vec2	skillPos;						// the skill position
float	skillCoolTime = 0.0f;			// the skill cool time
auto	enemy_circles = std::move(create_circles(-70.f, 70.f, -40.f, 40.f));
auto	enemy_triangles = std::move(create_triangles(-70.f, 70.f, -40.f, 40.f));
auto	enemy_squares = std::move(create_squares(-70.f, 70.f, -40.f, 40.f));
auto	hero = create_hero();
auto	bullets = create_bullets();
int		unitExplosionParticles = 32;	// number of particles in unit explosion 
int		totalNumOfParticles = 8*unitExplosionParticles;// total num of particles
auto	particles = create_explosion_particles(totalNumOfParticles);
int		offSetOfParticles = 0;			// manage offset for better assignment of particle
GLint	color_mode = 0;					// used for toggling color mode. 0 : default, 1 : plain, 2: white
float	speed = 0.22f;					// used for speed control factor
float	controllerX = 0.0f;				// used for control hero
float	controllerPrevX = 0.0f;			// used for smooth effect
float	controllerY = 0.0f;				// used for control hero
float	controllerPrevY = 0.0f;			// used for smooth effect
float	immortalTime = 0.3f;			// immortal time after hit
float	accTime = 0.0f;					// accumulated time
int		numOfHit = 0;					// num Of hero hit
float	score = 0.0f;					// total score
float	score_weight = 0.1f;			// score weight
float	a = 0.0f;						// flash effect
char	scoreMessage[32] = "SCORE:  ";	// pointer to save scoreMessage, we can use offset 7
char    lifeMessage[32] = "LIFE LEFT:  ";// pointer to save lifeMessage, we can use only offset 11 ,when life is 1 digit
char	coolTimeMessage[32] = "Skill Cool Time:  .  ";// pointer to save skillMessage, we can use only 17th, 19th index, when cooltime is X.X
float	alpha = 1.0f;					// alpha value for submit to fragment		
GLuint	squareIndexOffset;
GLuint	circleIndexOffset;


//*******************************************************************
// scene objects
//mesh*		pMesh = nullptr;
camera		cam;
trackball	tb;
light_t		light;
material_t	material;

//*******************************************************************
// holder of vertices and indices of objects
std::vector<vertex>	unit_circle_vertices;	// host-side vertices
std::vector<vertex>	unit_triangle_vertices;	// host-side vertices
std::vector<vertex>	unit_square_vertices;	// host-side vertices

//*******************************************************************
// forward declarations for stb_truetype text
void text_init();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color);

//*******************************************************************
void update()
{
	// update global simulation parameter
	t = float(glfwGetTime());
	delT = t - prevT;
	score_weight = 1.0f;
	if (isHelpMode) {
		delT = 0.0f;
	}
	if (numMoving == 0) {
		delT *= 0.4f;
		score_weight *= 0.1f;
	}
	if (isEasyMode) {
		delT *= 0.5f;
		score_weight *= 0.3f;
	}
	if (isHardMode) {
		delT *= 1.3f;
		score_weight *= 2.0f;
	}
	delT *= speed;
	accTime += delT;


	score += delT * 1000.0f * score_weight;
	speed += delT * 0.001f;// speed up
	prevT = t;

	a = abs(sin(float(glfwGetTime()) * 10.0f));

	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dnear, cam.dfar);


	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

	// setup texture
	glActiveTexture(GL_TEXTURE0);								// select the texture slot to bind
	glBindTexture(GL_TEXTURE_2D, GALAXY_TEX_SRC);
	glUniform1i(glGetUniformLocation(program, "TEX0"), 0);	 // GL_TEXTURE0

	glActiveTexture(GL_TEXTURE1);								// select the texture slot to bind
	glBindTexture(GL_TEXTURE_2D, WOOD_TEX_SRC);
	glUniform1i(glGetUniformLocation(program, "TEX1"), 1);	 // GL_TEXTURE1
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	// notify GL that we use our own program and buffers
	glUseProgram(program);

	// bind vertex attributes to your shader program
	cg_bind_vertex_attributes(program);


	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	GLint tmploc = glGetUniformLocation(program, "isTex0");				if (tmploc > -1) glUniform1i(tmploc, true);// use 0th texture

	// handle hero
	controllerPrevX = controllerX + controllerPrevX / 1.4f;
	controllerPrevY = controllerY + controllerPrevY / 1.4f;
	if (skillReq) {
		hero.center = skillPos;
		skillCoolTime = 3.0f;
		skillReq = false;
		isTimeFreeze = true;
		freezeRemain = 0.3f;
		engine->play2D(time_pause_sound_src, false);// second arg = loop or not
	}
	if (isTimeFreeze) {
		freezeRemain -= delT;
		delT = 0.0f;
		if (freezeRemain < 0) {
			isTimeFreeze = false;
		}
	}
	if (skillCoolTime >= 0) {
		skillCoolTime -= delT;
	}
	if (!isHelpMode) {
	hero.update(delT, controllerX + controllerPrevX, controllerY + controllerPrevY);
	}
	alpha = isImmortal?a:1.0f;// blinking when hero is immortal

	GLint uloc;
	uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1,vec4(hero.color.x, hero.color.y, hero.color.z, hero.color.w*alpha));	// pointer version
	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, hero.model_matrix);
	uloc = glGetUniformLocation(program, "color_mode");	if (uloc > -1) glUniform1i(uloc, color_mode);
	glDrawElements(GL_TRIANGLES, NUM_TESS * 3, GL_UNSIGNED_INT, (void*)circleIndexOffset);
	

	// handle bullets
	// render bullets: trigger shader program to process vertex data
	for (auto& b : bullets)
	{
		if (!b.nowAlive) {
			if (attackReq) {
				attackReq = false;
				b.nowAlive = true;
				b.delta = (attackPos - hero.center).normalize();
				b.angle = acos(b.delta.dot(vec2(1.0f, 0.f)));
				
				//bullet fatigue
				b.radius = max(0.5f, b.radius * 0.94f);
				b.speed = max(90.0f, b.speed * 0.94f);
				

				revive_bullet(b, hero.center.x, hero.center.y);// set rad, speed, lifetime
				if (b.delta.y < 0) {
					b.angle = 2 * PI - b.angle;
				}
				b.angle -= PI / 2.0f;
			}
			else {
				continue;
			}
		}
		b.update(delT);
		b.life -= delT;
		if (b.life < 0) {
			b.nowAlive = false;
		}

		// enemy crash check
		for (auto& c : enemy_circles) {
			if (!c.nowAlive) {
				continue;
			}
			if (length(b.center - c.center) < (b.radius + c.radius)/2.0f) {
				c.nowAlive = false;
				c.timeToRevive = max(2.0f, 5.0f - accTime*0.0045f);

				//bullet level up
				b.radius = min(5.0f, b.radius * 1.09f);
				b.speed = min(900.0f, b.speed * 1.09f);

				//gain score
				score += 400.0f * score_weight;

				engine->play2D(c_d_sound_src, false);// second arg = loop or not

				//particle effect
				for (int i = 0; i < unitExplosionParticles; i++) {
					setParticle(particles[offSetOfParticles], c.center.x, c.center.y, 10.f + c.radius, c.color);
					offSetOfParticles = (offSetOfParticles == totalNumOfParticles - 1) ? 0 : offSetOfParticles + 1;
				}
			}
		}
		for (auto& t : enemy_triangles) {
			if (!t.nowAlive) {
				continue;
			}
			if (length(b.center - t.center) < b.radius + t.radius) {
				t.nowAlive = false;
				t.timeToRevive = max(1.5f, 4.0f - accTime * 0.0055f);

				//bullet level up
				b.radius = min(5.0f, b.radius * 1.05f);
				b.speed = min(900.0f, b.speed * 1.05f);
				//gain score
				score += 200.0f * score_weight;

				engine->play2D(t_d_sound_src, false);// second arg = loop or not

				//particle effect
				for (int i = 0; i < unitExplosionParticles; i++) {
					setParticle(particles[offSetOfParticles], t.center.x, t.center.y, 2.f, t.color);
					offSetOfParticles = (offSetOfParticles == totalNumOfParticles - 1) ? 0 : offSetOfParticles + 1;
				}
			}
		}
		for (auto& s : enemy_squares) {
			if (!s.nowAlive) {
				continue;
			}
			if (length(b.center - s.center) < b.radius + s.radius) {
				if (s.phase < 0) { continue; }
				s.nowAlive = false;
				s.timeToRevive = max(1.5f, 7.0f - accTime * 0.055f);

				//bullet level up
				b.radius = min(5.0f, b.radius * 1.13f);
				b.speed = min(900.0f, b.speed * 1.13f);
				//gain score
				score += 800.0f * score_weight;

				engine->play2D(s_d_sound_src, false);// second arg = loop or not

				//particle effect
				for (int i = 0; i < unitExplosionParticles; i++) {
					setParticle(particles[offSetOfParticles], s.center.x, s.center.y, 4.f, s.color);
					offSetOfParticles = (offSetOfParticles == totalNumOfParticles - 1) ? 0 : offSetOfParticles + 1;
				}
			}
		}


		// update per-triangle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, vec4(b.color.x, b.color.y, b.color.z, 1.0f));	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, b.model_matrix);

		// per-triangle draw calls
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
	}

	// handle enemy_circle
	// render enemy_circles: trigger shader program to process vertex data
	for (auto& c : enemy_circles)
	{
		if (!c.nowAlive) {
			c.timeToRevive -= delT;
			if (c.timeToRevive < 0) {
				revive_circle(c, -70.f, 70.f, -40.f, 40.f, 1.0f);

				// circle level up
				c.scaleSpeed = min(100.0f, c.scaleSpeed * 1.2f);
			}
			continue;
		}

		// per-circle update
		c.update(delT);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, vec4(c.color.x, c.color.y, c.color.z, c.color.w*c.radius*0.3f));	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, c.model_matrix);

		// per-circle draw calls
		glDrawElements(GL_TRIANGLES, NUM_TESS * 3, GL_UNSIGNED_INT, (void*)circleIndexOffset);
	}

	// handle enemy_triangle
	// render enemy_triangle: trigger shader program to process vertex data
	for (auto& t : enemy_triangles)
	{
		if (!t.nowAlive) {
			t.timeToRevive -= delT;
			if (t.timeToRevive < 0) {
				revive_triangle(t, -70.f, 70.f, -40.f, 40.f);

				// triangle level up
				t.speed = min(80.0f, t.speed * 1.05f);
				t.radius = min(30.0f, t.radius * 1.06f);
			}
			continue;
		}
		if (length(t.destination - t.center) < 1.0f) {
			t.hasDestination = false;
		}
		if (!t.hasDestination) {
			t.destination = hero.center;
			t.delta = (t.destination - t.center).normalize();
			t.angle = acos(t.delta.dot(vec2(1.0f, 0)));
			if (t.delta.y < 0) {
				t.angle = 2 * PI - t.angle;
			}
			t.angle -= PI / 2.0f;
			t.hasDestination = true;
			t.speed *= 0.35f;
		}
		t.center +=  t.delta * delT * 30.0f;

		// per-triangle update
		t.update(delT);

		// update per-triangle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, vec4(t.color.x, t.color.y, t.color.z, t.color.w*t.speed*0.1f));	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, t.model_matrix);

		// per-triangle draw calls
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
	}

	tmploc = glGetUniformLocation(program, "isTex0");				if (tmploc > -1) glUniform1i(tmploc, false);
	// handle enemy_square
	// render enemy_squares: trigger shader program to process vertex data
	for (auto& s : enemy_squares)
	{
		if (!s.nowAlive) {
			s.timeToRevive -= delT;
			if (s.timeToRevive < 0) {
				revive_square(s, -70.f, 70.f, -40.f, 40.f);

				// square level up
				s.phaseSpeed = min(110.0f, 1.1f * s.phaseSpeed);
			}
			continue;
		}
		if (s.phase > 0 && s.phase - delT*s.phaseSpeed < 0) {
			s.hasDestination = false;
		}
		if (!s.hasDestination) {
			s.destination = hero.center;
			s.delta = (s.destination - s.center).normalize();
			s.angle = acos(s.delta.dot(vec2(1.0f, 0)));
			if (s.delta.y < 0) {
				s.angle = 2 * PI - s.angle;
			}
			s.angle -= PI / 2.0f;
			s.hasDestination = true;
		}
		if (s.phase < -10.0f) {
			s.phase = -s.phase;
		}
		// per-square update
		s.update(delT);

		// update per-square uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, vec4(s.color.x, s.color.y, s.color.z, s.color.w*s.length*0.4f));	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, s.model_matrix);

		// per-square draw calls
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)squareIndexOffset);
	}

	// handle particles
	// render particles: trigger shader program to process vertex data
	for (auto& p : particles)
	{
		if (!p.nowAlive) {
			continue;
		}
		// per-particle update
		p.update(delT);
		if (p.timeOfLife >= 0) {

			p.timeOfLife -= delT;
		}
		else if (p.timeOfLife < 0) {
			p.nowAlive = false;
		}
		// update per-particle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, vec4(p.color.x, p.color.y, p.color.z, 1.0f));	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, p.model_matrix);

		// per-particle draw calls
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
	}

	if (isImmortal) {
		immortalTime -= delT;
		if (immortalTime < 0) {
			isImmortal = false;
		}
	}

	// hit test
	isAttacked = false;
	if (!isAttacked && !isImmortal) {
		for (auto& c : enemy_circles) {
			if (c.response_time > 0) {
				c.response_time -= delT;
			}
			if (!c.nowAlive || c.response_time>0) {
				continue;
			}
			if (length(hero.center - c.center) < hero.radius + c.radius) {
				isAttacked = true;
			}
		}
	}
	if (!isAttacked && !isImmortal) {
		for (auto& t : enemy_triangles) {
			if (t.response_time > 0) {
				t.response_time -= delT;
			}
			if (!t.nowAlive || t.response_time > 0) {
				continue;
			}
			if (length(hero.center - t.center) < hero.radius + t.radius) {
				isAttacked = true;
			}
		}
	}
	
	if (!isAttacked && !isImmortal) {
		for (auto& s : enemy_squares) {
			if (s.response_time > 0) {
				s.response_time -= delT;
			}
			if (!s.nowAlive || s.response_time>0) {
				continue;
			}
			if (s.phase > 0) {
				continue;
			}
			if ((abs((hero.center - s.center).dot(vec2(s.delta.y, -s.delta.x))) < hero.radius + s.radius)
			&& ((hero.center - s.center).dot(s.delta) < s.length)) {
				isAttacked = true;
			}
		}
	}
	if (isAttacked) {
		engine->play2D(hit_sound_src, false);// second arg = loop or not
		speed *= 0.9f;
		numOfHit++;
		immortalTime = 0.3f;
		isImmortal = true;
		for (int i = 0; i < unitExplosionParticles; i++) {
			setParticle(particles[offSetOfParticles], hero.center.x, hero.center.y, 4.f, hero.color);
			offSetOfParticles = (offSetOfParticles == totalNumOfParticles - 1) ? 0 : offSetOfParticles + 1;
		}
	}	

	if (numOfHit > 3 && !isGameFinished) {// if hit is over 3, the game will be over
		engine->play2D(end_sound_src, false);// second arg = loop or not
		speed = 0.0f;
		printf("game over\n\n\nYour score: %f", score);
		isGameFinished = true;
	}

	if (!isHelpMode) {
		// game mode text rendering
		render_text("Press [ Space ] to See The Help Message", 100, 55, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.5f));
		// processing score
		int point = (int)score;
		int i = 1;
		while (i <= point) {
			i *= 10;
		}
		i /= 10;// i is the most significant digit
		int j = 7;// offset is 7 (refer to definition of scoreMessage)
		while (i > 0) {
			scoreMessage[j] = (point / i) + '0';
			point %= i;
			i /= 10;
			j++;
		}
		scoreMessage[j] = '\0';// FILL END OF STRING WITH NULL CHAR
		// processing life message
		lifeMessage[11] = 4 - numOfHit + '0';
		if (skillCoolTime < 0) {
			render_text("Skill is Ready!!!", 100, 165, 1.0f, vec4(0.5f, 0.7f, 0.7f, a));
		}
		else {
			// processing cooltime message
			coolTimeMessage[17] = '0' + (int)skillCoolTime;
			coolTimeMessage[19] = '0' + (int)(skillCoolTime * 10.0f) % 10;
			render_text(coolTimeMessage, 100, 165, 1.0f, vec4(0.5f, 0.7f, 0.7f, 0.8f));
		}
		if (isEasyMode) {
			render_text("EASY MODE", 100, 185, 0.3f, vec4(0.1f, 1.0f, 0.1f, 0.8f));
		}
		if (isHardMode) {
			render_text("Hard MODE", 100, 185, 0.3f, vec4(1.0f, 0.1f, 0.1f, 0.8f));
		}
		render_text(scoreMessage, 100, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 0.8f));
		render_text(lifeMessage, 100, 125, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f));

		if (isGameFinished) {
			render_text("Game Over", 200, 250, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
			render_text("Press [ Q ] or [ ESC ] to Quit", 200, 320, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
			render_text("Press [ R ] to Restart", 200, 380, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
			render_text(scoreMessage, 200, 460, 1.0f, vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}
	
	else if (isHelpMode) {
		// game mode text rendering
		render_text("Press [ Q ] or [ ESC ] to Quit The Game", 100, 35, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.6f));
		render_text("Press [ Space ] to Quit The Help Message And Continue The Game", 100, 55, 0.3f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
		render_text("Press Right Mouse Button To Shoot The Bullet", 100, 100, 0.8f, vec4(0.1f, 1.0f, 0.2f, 0.8f));
		render_text("Press Left Mouse Button To Use The Skill: Teleport with Time Pause", 100, 150, 0.6f, vec4(1.0f, 0.5f, 0.2f, 0.8f));
		render_text("When You Stop, The Time Goes Slow", 100, 220, 1.0f, vec4(1.0f, 1.0f, 0.0f, 1.0f));
		render_text("Press Y To Select EASY MODE, Press H To Select Hard Mode, Default is Normal Mode", 100, 280, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f));
		render_text("Use [ W ] [ A ] [ S ] [ D ] To Move The Hero", 100, 320, 0.5f, vec4(1.0f, 0.3f, 0.1f, 0.8f));
		render_text("Use [ C ] To Toggle the Color Mode : original, plain, white", 100, 360, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.9f));
		render_text("The Bullet Gets Weaker By Wear Out. But It Also Gets Stronger When It Killed The Enemy", 100, 410, 0.5f, vec4(0.0f, 0.3f, 1.0f, 0.8f));
		render_text("You Will Get The Better Score When You Are Keep Moving", 100, 450, 0.5f, vec4(1.0f, 0.4f, 0.5f, 1.0f));
		render_text("But Think Carefully Before You Move", 100, 490, 0.5f, vec4(1.0f, 1.0f, 0.1f, 1.0f));
		render_text("Enemy - Triangle: Follows Your Trace, Gets Faster", 100, 560, 0.5f, vec4(0.0f, 1.0f, 0.8f, 1.0f));
		render_text("Enemy - Circle: Gets Bigger And Bigger To Cover All. Destroy The Core", 100, 630, 0.5f, vec4(1.0f, 0.3f, 0.0f, 1.0f));
		render_text("Enemy - Square: It Thrusts So Fast. Find It While It Is Sleeping", 100, 700, 0.5f, vec4(0.6f, 0.3f, 0.9f, 1.0f));
	}
	
	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press 'y' to use easy mode\n");
	printf("- press 'h' to use hard mode\n");
	printf("- press 'r' to restart\n");
	printf("\n");
}

std::vector<vertex> create_circle_vertices(uint N)
{
	std::vector<vertex> v = { { vec3(0), vec3(0,0,-1.0f), vec2(0.5f) } }; // origin
	for (uint k = 0; k <= N; k++)
	{
		float t = PI * 2.0f * k / float(N), c = cos(t), s = sin(t);
		v.push_back({ vec3(c,s,0), vec3(c,s,-1.0f).normalize(), vec2(c,s) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	}
	return v;
}

std::vector<vertex> create_square_vertices()
{
	std::vector<vertex> v = {};
	float x, y;
	x = -1.0f;
	y = -1.0f;
	v.push_back({ vec3(x,y,0), vec3(0.0f,0.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	x = 1.0f;
	y = -1.0f;
	v.push_back({ vec3(x,y,0), vec3(0.0f,0.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	x = 1.0f;
	y = 1.0f;
	v.push_back({ vec3(x,y,0), vec3(0.0f,0.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	x = -1.0f;
	y = 1.0f;
	v.push_back({ vec3(x,y,0), vec3(0.0f,0.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])

	return v;
}

std::vector<vertex> create_triangle_vertices()
{
	std::vector<vertex> v = {};
	float x, y;
	x = -1.0f;
	y = -1.0f;
	v.push_back({ vec3(x,y,0), vec3(-1.73f,-1.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	x = 1.0f;
	y = -1.0f;
	v.push_back({ vec3(x,y,0), vec3(1.73f,-1.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])
	x = 0.0f;
	y = 1.0f;
	v.push_back({ vec3(x,y,0), vec3(0,2.0f,-1.0f).normalize(), vec2(x,y) * 0.5f + 0.5f });// vertex pos, normal vector, texture coordinate in ([0,1], [0,1])

	return v;
}

void update_vertex_buffer(const std::vector<vertex>& circle_vertices, const std::vector<vertex>& triangle_vertices, const std::vector<vertex>& square_vertices, uint N)
{
	// clear and create new buffers
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 1;

	// check exceptions
	if (circle_vertices.empty()) { printf("[error] circle vertices is empty.\n"); return; }
	if (triangle_vertices.empty()) { printf("[error] triangle vertices is empty.\n"); return; }
	if (square_vertices.empty()) { printf("[error] sqaure vertices is empty.\n"); return; }

	// create buffers

	std::vector<uint> indicesT;
	std::vector<uint> indicesS;
	std::vector<uint> indicesC;
	uint acc = 0;// provide index setting from 0 in each indices
	indicesT.push_back(0);// triangle
	indicesT.push_back(1);
	indicesT.push_back(2);
	acc += triangle_vertices.size();

	indicesS.push_back(acc + 0);// square
	indicesS.push_back(acc + 1);
	indicesS.push_back(acc + 2);
	indicesS.push_back(acc + 2);
	indicesS.push_back(acc + 3);
	indicesS.push_back(acc + 0);
	acc += square_vertices.size();

	for (uint k = 0; k < N; k++)// circle
	{
		indicesC.push_back(acc );	// the origin
		indicesC.push_back(acc + k + 1);
		indicesC.push_back(acc + k + 2);
	}
	acc += indicesC.size();

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * (triangle_vertices.size() + square_vertices.size() + circle_vertices.size()), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex) * triangle_vertices.size(), &triangle_vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex) * triangle_vertices.size(), sizeof(vertex) * square_vertices.size(), &square_vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex) * (triangle_vertices.size() + square_vertices.size()), sizeof(vertex) * circle_vertices.size(), &circle_vertices[0]);

	squareIndexOffset = sizeof(uint) * indicesT.size();
	circleIndexOffset = sizeof(uint) * (indicesT.size() + indicesS.size());
	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * (indicesT.size() + indicesS.size() + indicesC.size()), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint) * indicesT.size(), &indicesT[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesT.size(), sizeof(uint) * indicesS.size(), &indicesS[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * (indicesT.size() + indicesS.size()), sizeof(uint) * indicesC.size(), &indicesC[0]);
}

void restart() {
	isGameFinished = false;
	isHardMode = false;
	isEasyMode = false;
	isHelpMode = false;
	attackReq = false;	
	skillReq = false;				// flag representing about skill request
	isAttacked = false;				// check if hero is attacked
	isImmortal = false;				// check if hero is immortal
	isTimeFreeze = false;			// check if time is freezed
	freezeRemain = 0.0f;			// the amount of time freeze
	numMoving = 0;					// the flag to find if hero is moving
	attackPos;						// the attack position
	skillPos;						// the skill position
	skillCoolTime = 0.0f;			// the skill cool time
	enemy_circles = std::move(create_circles(-70.f, 70.f, -40.f, 40.f));
	enemy_triangles = std::move(create_triangles(-70.f, 70.f, -40.f, 40.f));
	enemy_squares = std::move(create_squares(-70.f, 70.f, -40.f, 40.f));
	hero = create_hero();
	bullets = create_bullets();
	unitExplosionParticles = 32;	// number of particles in unit explosion 
	totalNumOfParticles = 8 * unitExplosionParticles;// total num of particles
	particles = create_explosion_particles(totalNumOfParticles);
	offSetOfParticles = 0;			// manage offset for better assignment of particle
	color_mode = 0;					// used for toggling color mode. 0 : default, 1 : plain, 2: dark
	speed = 0.22f;					// used for speed control factor
	controllerX = 0.0f;				// used for control hero
	controllerPrevX = 0.0f;			// used for smooth effect
	controllerY = 0.0f;				// used for control hero
	controllerPrevY = 0.0f;			// used for smooth effect
	immortalTime = 0.3f;			// immortal time after hit
	accTime = 0.0f;					// accumulated time
	numOfHit = 0;					// num Of hero hit
	score = 0.0f;					// total score
	score_weight = 0.1f;			// score weight
	a = 0.0f;						// flash effect
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_W)
		{
			controllerY = 1.0f;
			numMoving++;
		}
		if (key == GLFW_KEY_A)
		{
			controllerX = -1.0f;
			numMoving++;
		}
		if (key == GLFW_KEY_S)
		{
			controllerY = -1.0f;
			numMoving++;
		}
		if (key == GLFW_KEY_D)
		{
			controllerX = 1.0f;
			numMoving++;
		}
		if (key == GLFW_KEY_Y)
		{
			if (!isHardMode) {
				isEasyMode = true;
				printf("Easy Mode On\n");
			}
			else {
				printf("Mode Selection is allowed only once\n");
			}
		}
		if (key == GLFW_KEY_H)
		{
			if (!isEasyMode) {
				isHardMode = true;
				printf("Hard Mode On\n");
			}
			else {
				printf("Mode Selection is allowed only once\n");
			}
		}
		if (key == GLFW_KEY_R)
		{
			if (isGameFinished) {// for prevent mistake restart
				restart();
				printf("Game Restart\n");
			}
		}
		if (key == GLFW_KEY_C)
		{
			color_mode = (color_mode + 1) % 3;
			GLint uloc = glGetUniformLocation(program, "color_mode");	if (uloc > -1) glUniform1i(uloc, color_mode);
			printf("toggled the color mode to ");
			if(color_mode==0) { printf("original mode\n"); }
			else if(color_mode==1) { printf("plain mode\n"); }
			else if(color_mode==2) { printf("dark mode\n"); }
		}

		if (key == GLFW_KEY_SPACE)
		{
			isHelpMode = !isHelpMode;// toggle the help mode
		}
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W)
		{
			controllerY = 0.0f;
			numMoving--;
		}
		if (key == GLFW_KEY_A)
		{
			controllerX = 0.0f;
			numMoving--;
		}
		if (key == GLFW_KEY_S)
		{
			controllerY = 0.0f;
			numMoving--;
		}
		if (key == GLFW_KEY_D)
		{
			controllerX = 0.0f;
			numMoving--;
		}
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
	vec2 npos = vec2(float(pos.x) / float(window_size.x - 1), float(pos.y) / float(window_size.y - 1));
	vec2 mypos = npos * 2.0f - vec2(1.0f);
	if (button == GLFW_MOUSE_BUTTON_LEFT && mods == 0) {// skill
		if (action == GLFW_PRESS && skillCoolTime<0 && !isHelpMode) {
			skillReq = true;
			skillPos = vec2(mypos.x * 73.4f, -mypos.y * 41.0f);
			immortalTime = 0.3f;
			isImmortal = true;
		}
		//else if (action == GLFW_RELEASE)	
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT || (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_SHIFT))) {// attack
		if (action == GLFW_PRESS && !isHelpMode) {
			attackReq = true;
			attackPos = vec2(mypos.x*73.4f,-mypos.y*41.0f);// fine tuned position
			engine->play2D(bullet_src, false);// second arg = loop or not
		}
		//else if (action == GLFW_RELEASE)	
	}
}

void motion(GLFWwindow* window, double x, double y)
{
	// vec2 npos = vec2(float(x) / float(window_size.x - 1), float(y) / float(window_size.y - 1));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}


void update_render_target_textures(int width, int height)
{
	if (HORZ) glDeleteTextures(1, &HORZ);
	if (VERT) glDeleteTextures(1, &VERT);

	// since we are using render-to-texture, we resize the textures by the window size
	glGenTextures(1, &HORZ);
	glBindTexture(GL_TEXTURE_2D, HORZ);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &VERT);
	glBindTexture(GL_TEXTURE_2D, VERT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

bool user_init()
{
	// set random seed
	srand((unsigned)time(NULL));

	// log hotkeys
	print_help();

	// init stb_truetype
	text_init();

	// init GL states
	glLineWidth(1.0f);
	glClearColor(9 / 255.0f, 20 / 255.0f, 25 / 255.0f, 1.0f);	// set clear color
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glUseProgram(program);

	// load the mesh
	//pMesh = new mesh();
	//pMesh = cg_load_mesh(mesh_vertex_path, mesh_index_path);
	//if (pMesh == nullptr) { printf("Unable to load mesh\n"); return false; }

	// define the position of vertices
	unit_circle_vertices = std::move(create_circle_vertices(NUM_TESS));
	unit_triangle_vertices = std::move(create_triangle_vertices());
	unit_square_vertices = std::move(create_square_vertices());

	// create vertex buffer;
	update_vertex_buffer(unit_circle_vertices, unit_triangle_vertices, unit_square_vertices, NUM_TESS);

	//tex0
	// load and flip an image
	int width, height, comp=3;
	unsigned char* pimage0 = stbi_load( galaxy_texture_path, &width, &height, &comp, 3 ); if(comp==1) comp=3; /* convert 1-channel to 3-channel image */
	int stride0 = width*comp, stride1 = (stride0+3)&(~3);	// 4-byte aligned stride
	unsigned char* pimage = (unsigned char*) malloc( sizeof(unsigned char)*stride1*height );
	for( int y=0; y < height; y++ ) memcpy( pimage+(height-1-y)*stride1, pimage0+y*stride0, stride0 ); // vertical flip

	// create textures
	glGenTextures( 1, &GALAXY_TEX_SRC );
	glBindTexture( GL_TEXTURE_2D, GALAXY_TEX_SRC );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage );

	// allocate and create mipmap
	int mip_levels = miplevels( window_size.x, window_size.y );
	for( int k=1, w=width>>1, h=height>>1; k<mip_levels; k++, w=max(1,w>>1), h=max(1,h>>1) )// this for-loop can be replaced with glTexStorage2D
		glTexImage2D( GL_TEXTURE_2D, k, GL_RGB8 /* GL_RGB for legacy GL */, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
	glGenerateMipmap( GL_TEXTURE_2D );

	// release the new image
	free(pimage);

	//tex1
	// load and flip an image
	comp = 3;
	pimage0 = stbi_load(thunder_texture_path, &width, &height, &comp, 3); if (comp == 1) comp = 3; /* convert 1-channel to 3-channel image */
	stride0 = width * comp, stride1 = (stride0 + 3) & (~3);	// 4-byte aligned stride
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0); // vertical flip

	// create textures
	glGenTextures(1, &WOOD_TEX_SRC);
	glBindTexture(GL_TEXTURE_2D, WOOD_TEX_SRC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);

	// allocate and create mipmap
	mip_levels = miplevels(window_size.x, window_size.y);
	for (int k = 1, w = width >> 1, h = height >> 1; k < mip_levels; k++, w = max(1, w >> 1), h = max(1, h >> 1))// this for-loop can be replaced with glTexStorage2D
		glTexImage2D(GL_TEXTURE_2D, k, GL_RGB8 /* GL_RGB for legacy GL */, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);


	// create a frame buffer object for render-to-texture
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// configure texture parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);

	// release the new image
	free(pimage);



	// sound
	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;

	// add sound source from the sound file
	bullet_src = engine->addSoundSourceFromFile(bullet_sound_path);
	background_music_src = engine->addSoundSourceFromFile(background_music_path);
	hit_sound_src = engine->addSoundSourceFromFile(hit_sound_path);
	end_sound_src = engine->addSoundSourceFromFile(end_sound_path);
	t_d_sound_src = engine->addSoundSourceFromFile(t_d_sound_path);
	c_d_sound_src = engine->addSoundSourceFromFile(c_d_sound_path);
	s_d_sound_src = engine->addSoundSourceFromFile(s_d_sound_path);
	time_pause_sound_src = engine->addSoundSourceFromFile(time_pause_sound_path);

	// set default volume
	bullet_src->setDefaultVolume(0.28f);
	background_music_src->setDefaultVolume(0.9f);
	hit_sound_src->setDefaultVolume(1.2f);
	end_sound_src->setDefaultVolume(1.0f);
	t_d_sound_src->setDefaultVolume(0.9f);
	c_d_sound_src->setDefaultVolume(0.6f);
	s_d_sound_src->setDefaultVolume(1.5f);
	time_pause_sound_src->setDefaultVolume(1.3f);

	// play the sound file
	engine->play2D(background_music_src, true);// second arg = loop or not

	return true;
}

void user_finalize()
{
	// close the engine
	engine->drop();
}

int main(int argc, char* argv[])
{
	// initialization
	if (!glfwInit()) { printf("[error] failed in glfwInit()\n"); return 1; }

	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }	// create and compile shaders/program
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
	glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
	glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
	glfwSetCursorPosCallback(window, motion);		// callback for mouse movements
	glfwSetScrollCallback(window, scroll_callback); // callback for wheel scroll

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
