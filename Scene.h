#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Entity.h"
#include "Effects.h"
#include "Map.h"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState
{
    // ————— GAME OBJECTS ————— //
    Map *map;
    Entity *player;
    Entity* enemy;
    Effects* effect;

    // ----- MUSIC OBJECTS ----- //
    Mix_Chunk* jump_sfx;
    
    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id;
    int curr_scene_id;
};

class Scene {
protected:
    GameState m_game_state;
    
public:
    // ————— ATTRIBUTES ————— //
    int m_number_of_enemies = 0;
    bool m_player_dead = false;
    float m_left_edge = 0.0f;
    float m_right_edge = 0.0f;
    float m_bottom_edge = 0.0f;
    glm::vec3 m_enemy_init_pos = glm::vec3(0.0f,0.0f, 0.0f);
    glm::vec3 m_player_init_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4* m_projection_mat;
    glm::mat4* m_view_matrix;
    
    // ————— METHODS ————— //
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
    // ————— GETTERS ————— //
    GameState const get_state() const { return m_game_state;             }
    int const get_number_of_enemies() const { return m_number_of_enemies; }
};
