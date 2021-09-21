#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <cmath>

constexpr float epsilon = 0.1f;
constexpr float eepsilon = 0.01f;

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// player state
	Scene::Transform *player = nullptr;
	Scene::Transform *cone_icon = nullptr;
	struct Elevated{
		uint8_t current = 0;
		uint8_t jumped_from = 0;
	} elevated;
	float jump_time = 0.f;

	// dangerous donut pointers
	Scene::Drawable *donut1 = nullptr;
	Scene::Drawable *frosting1 = nullptr;
	Scene::Drawable *donut2 = nullptr;
	Scene::Drawable *frosting2 = nullptr;
	Scene::Drawable *donut3 = nullptr;
	Scene::Drawable *frosting3 = nullptr;
	Scene::Drawable *donut4 = nullptr;
	Scene::Drawable *frosting4 = nullptr;
	uint8_t donut1eat = 0;
	uint8_t donut2eat = 0;
	uint8_t donut3eat = 0;
	uint8_t donut4eat = 0;

	// helpers for getting heights
	inline float ground_height(uint8_t elev){
		return elev * 0.1f + 0.2f;
	}
	inline float get_height(float air_time){
		return air_time * (1 - air_time) + ground_height(elevated.jumped_from);
	}
	
	uint8_t on_ground = true;

	//determines if elevated ground or not
	inline uint8_t get_elevated(glm::vec3 pos){
		if(pos.x < 0.f) return false;
		if(pos.x < 6.f && pos.y > 6.f && pos.y < 8.f) return false;
		if(pos.x > 6.f && pos.x < 14.f && pos.y > 4.f && pos.y < 10.f) return false;
		if(pos.x > 10.f && pos.x < 14.f && pos.y < 10.f) return false;
		if(pos.x < 14.f && pos.y < 0.f && pos.y > -2.f) return false;
		return true;
	}

	inline void clip_building(glm::vec3 &pos, float x_lo, float x_hi, float y_lo, float y_hi){
		if (!(x_lo < pos.x && pos.x < x_hi && y_lo < pos.y && pos.y < y_hi)) return;
		float xlo_diff = pos.x - x_lo,
		xhi_diff = x_hi - pos.x,
		ylo_diff = pos.y - y_lo,
		yhi_diff = y_hi - pos.y;
		float min_diff = fmin(fmin(xlo_diff, xhi_diff), fmin(ylo_diff, yhi_diff));
		if (min_diff == xlo_diff) pos.x = x_lo;
		if (min_diff == xhi_diff) pos.x = x_hi;
		if (min_diff == ylo_diff) pos.y = y_lo;
		if (min_diff == yhi_diff) pos.y = y_hi;
	}

	// actual sketchy shit, if it crashes i blame it on this
	inline bool consume(Scene::Drawable *donut, Scene::Drawable *frosting, float diffx, float diffy){
		if (donut && (diffx * diffx + diffy * diffy) < 0.04f){
			donut->pipeline.vao = 0;
			frosting->pipeline.vao = 0;
			donut = nullptr; frosting = nullptr;
			return true;
		}
		return false;
	}
	
	//camera:
	Scene::Camera *camera = nullptr;

};
