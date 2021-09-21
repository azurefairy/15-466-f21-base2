#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <cmath>
#include <sstream>

constexpr glm::vec3 forward_mov(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 backward_mov(-1.0f, 0.0f, 0.0f);
constexpr glm::vec3 left_mov(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 right_mov(0.0f, -1.0f, 0.0f);

GLuint city_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > city_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("city.pnct"));
	city_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > city_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("city.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = city_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = city_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*city_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "aplayer") player = &transform;
		else if (transform.name == "Cone") cone_icon = &transform;
	}
	if (player == nullptr) throw std::runtime_error("aplayer not found.");
	if (cone_icon == nullptr) throw std::runtime_error("Cone not found.");

	for(auto &drawable : scene.drawables) {
		if (drawable.transform->name == "donut.001") donut1 = &drawable;
		else if (drawable.transform->name == "donut.002") donut2 = &drawable;
		else if (drawable.transform->name == "donut.003") donut3 = &drawable;
		else if (drawable.transform->name == "donut.004") donut4 = &drawable;
		else if (drawable.transform->name == "frosting.001") frosting1 = &drawable;
		else if (drawable.transform->name == "frosting.002") frosting2 = &drawable;
		else if (drawable.transform->name == "frosting.003") frosting3 = &drawable;
		else if (drawable.transform->name == "frosting.004") frosting4 = &drawable;
	}
	if (donut1 == nullptr) throw std::runtime_error("donut1 not found.");
	if (donut2 == nullptr) throw std::runtime_error("donut2 not found.");
	if (donut3 == nullptr) throw std::runtime_error("donut3 not found.");
	if (donut4 == nullptr) throw std::runtime_error("donut4 not found.");
	if (frosting1 == nullptr) throw std::runtime_error("frosting1 not found.");
	if (frosting2 == nullptr) throw std::runtime_error("frosting2 not found.");
	if (frosting3 == nullptr) throw std::runtime_error("frosting3 not found.");
	if (frosting4 == nullptr) throw std::runtime_error("frosting4 not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			float motion = evt.motion.xrel / float(window_size.y);
			player->rotation = glm::normalize(
				player->rotation
				* glm::angleAxis(-motion * camera->fovy, glm::vec3(0.0f, 0.0f, 1.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	// handle horizontal movement
	glm::vec3 mov(0.0f, 0.0f, 0.0f);
	if (left.pressed)
		mov += left_mov;
	if (right.pressed)
		mov += right_mov;
	if (up.pressed)
		mov += forward_mov;
	if (down.pressed)
		mov += backward_mov;

	if (fabs(mov.x) + fabs(mov.y) + fabs(mov.z) > 1.5f)
		mov /= sqrtf(2.f);
	player->position += player->rotation * mov * elapsed * 2.f;
	
	//clip in bounds
	if(player->position.x < -1.f + epsilon) player->position.x = -1.f + epsilon;
	if(player->position.x > 19.f - epsilon) player->position.x = 19.f - epsilon;
	if(player->position.y < -3.f + epsilon) player->position.y = -3.f + epsilon;
	if(player->position.y > 17.f - epsilon) player->position.y = 17.f - epsilon;
	//clip buildings
	clip_building(player->position, 1.f - epsilon, 5.f + epsilon, 1.f - epsilon, 3.f + epsilon);
	clip_building(player->position, 3.f - epsilon, 7.f + epsilon, 11.f - epsilon, 15.f + epsilon);
	clip_building(player->position, 9.f - epsilon, 11.f + epsilon, 11.f - epsilon, 15.f + epsilon);
	clip_building(player->position, 9.f - epsilon, 13.f + epsilon, 11.f - epsilon, 13.f + epsilon);
	// TODO: clip height for elevation
	if(cone_icon->position.z < 0.3f){
		clip_building(player->position, 14.f - eepsilon, 19.f + eepsilon, -3.f - eepsilon, 17.f + eepsilon);
		clip_building(player->position, 0.f - eepsilon, 19.f + eepsilon, 10.f - eepsilon, 17.f + eepsilon);
		clip_building(player->position, 0.f - eepsilon, 6.f + eepsilon, 8.f - eepsilon, 17.f + eepsilon);
		clip_building(player->position, 0.f - eepsilon, 10.f + eepsilon, -3.f - eepsilon, -2.f + eepsilon);
		clip_building(player->position, 0.f - eepsilon, 10.f + eepsilon, 0.f - eepsilon, 4.f + eepsilon);
		clip_building(player->position, 0.f - eepsilon, 6.f + eepsilon, 0.f - eepsilon, 6.f + eepsilon);
	}

	//handle vertical movement
	if (space.pressed && on_ground){
		jump_time = 1.f;
		on_ground = false;
	}
	jump_time -= elapsed;
	//presumed height
	float player_height = get_height(jump_time);
	if(jump_time < 0.5 && player_height < ground_height(elevated.current)){
		jump_time = 0.f;
		player_height = ground_height(elevated.current);
		on_ground = true;
	}

	cone_icon->position.z = player_height;

	// update status variables
	if(elevated.current && !get_elevated(player->position)) on_ground = false;
	elevated.current = get_elevated(player->position);
	if (on_ground) elevated.jumped_from = elevated.current;

	// CONSUME THE DONUTS
	if(consume(donut1, frosting1, player->position.x - 12.f, player->position.y - 14.f)) donut1eat = 1;
	if(consume(donut2, frosting2, player->position.x - 17.f, player->position.y - 7.4477f)) donut2eat = 1;
	if(consume(donut3, frosting3, player->position.x - 8.f, player->position.y - 13.f)) donut3eat = 1;
	if(consume(donut4, frosting4, player->position.x - 2.f, player->position.y - -0.5f)) donut4eat = 1;

	// //slowly rotates through [0,1):
	// wobble += elapsed / 10.0f;
	// wobble -= std::floor(wobble);

	// hip->rotation = hip_base_rotation * glm::angleAxis(
	// 	glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );
	// lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );

	// //move camera:
	// {

	// 	//combine inputs into a move:
	// 	constexpr float PlayerSpeed = 30.0f;
	// 	glm::vec2 move = glm::vec2(0.0f);
	// 	if (left.pressed && !right.pressed) move.x =-1.0f;
	// 	if (!left.pressed && right.pressed) move.x = 1.0f;
	// 	if (down.pressed && !up.pressed) move.y =-1.0f;
	// 	if (!down.pressed && up.pressed) move.y = 1.0f;

	// 	//make it so that moving diagonally doesn't go faster:
	// 	if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

	// 	glm::mat4x3 frame = camera->transform->make_local_to_parent();
	// 	glm::vec3 right = frame[0];
	// 	//glm::vec3 up = frame[1];
	// 	glm::vec3 forward = -frame[2];

	// 	camera->transform->position += move.x * right + move.y * forward;
	// }

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; Spacebar jumps; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; Spacebar jumps; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		
		std::stringstream ss;
		ss << "Remaining donuts: " << (4 - donut1eat - donut2eat - donut3eat - donut4eat);
		lines.draw_text(ss.str(),
			glm::vec3(-aspect + 0.1f * H, 1.0 - 1.2f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text(ss.str(),
			glm::vec3(-aspect + 0.1f * H + ofs, 1.0 - 1.2f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
