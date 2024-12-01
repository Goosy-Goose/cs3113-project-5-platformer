#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_LEFT_EDGE 6.0f
#define MAX_RGB 255

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Start.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 1280 / 1.25,
          WINDOW_HEIGHT = 960 / 1.25;

constexpr float BG_RED = 27.0f / MAX_RGB,
            BG_BLUE    = 43.0f / MAX_RGB,
            BG_GREEN   = 52.0f / MAX_RGB,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

Mix_Music* g_bgm;
constexpr char BGM_FILEPATH[] = "assets/Doobly_doo.mp3";

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_curr_scene = nullptr;
Start* g_levelStart = nullptr;
LevelA *g_levelA = nullptr;
LevelB* g_levelB = nullptr;
LevelC* g_levelC = nullptr;

Effects *g_effects = nullptr;
Scene   *g_levels[4];

SDL_Window* g_display_window = nullptr;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

bool g_paused = false;
bool g_game_over = true;
int g_num_lives = 3;
GLuint g_text_id;
glm::vec3 g_camera_position;

AppStatus g_app_status = RUNNING;


void switch_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

void switch_to_scene(Scene *scene)
{    
    g_curr_scene = scene;
    g_curr_scene->m_projection_mat = &g_projection_matrix;
    g_curr_scene->m_view_matrix = &g_view_matrix;
    //g_curr_scene->get_state().effect = new Effects(g_projection_matrix, g_view_matrix);
    g_curr_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("platfomers that are trying so hard not to cry (im platformers)",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-3.75f, 3.75f, -2.8125f, 2.8125f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_text_id = Utility::load_texture("assets/font1.png");
    g_camera_position = glm::vec3(0.0f, 0.0f, 0.0f);

    // ----- LEVELS ----- //
    g_levelStart = new Start();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    g_levels[0] = g_levelStart;
    // TODO: Change levels back to correct order
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
    switch_to_scene(g_levels[0]);
    
    // ---- EFFECTS ----- //
    g_effects = new Effects(g_projection_matrix, g_view_matrix);

    // ----- AUDIO ----- //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    g_bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_curr_scene->get_state().player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q: g_app_status = TERMINATED;     break;
                    case SDLK_ESCAPE: g_app_status = TERMINATED;     break;
                       
                    case SDLK_SPACE:
                        // TODO: stop game on die or pause
                        //if (g_curr_scene->get_state().player->get_player_died()) break;

                        // Jump
                        if (g_curr_scene->get_state().player->get_collided_bottom())
                        {
                            g_curr_scene->get_state().player->jump();
                            g_curr_scene->get_state().player->set_animation_state(JUMP);
                            /* jump sfx*/
                            if (!g_paused)
                            Mix_PlayChannel(-1, g_curr_scene->get_state().jump_sfx, 0);
                            Mix_VolumeChunk(g_curr_scene->get_state().jump_sfx, MIX_MAX_VOLUME / 4);
                        }
                        break;
                    case SDLK_t:
                        if (g_curr_scene->get_state().curr_scene_id == 0) {
                            switch_to_scene(g_levels[g_curr_scene->get_state().curr_scene_id + 1]);
                        }
                        break;
                    case SDLK_p:
                        g_paused = !g_paused;
                        break;
                    case SDLK_r:
                        g_curr_scene->get_state().player->set_position(g_curr_scene->m_player_init_pos);
                        g_curr_scene->get_state().enemy->set_position(g_curr_scene->m_enemy_init_pos);
                        if (g_curr_scene->m_player_dead) {
                            g_curr_scene->m_player_dead = false;
                            g_curr_scene->get_state().player->set_player_died(false);
                            g_paused = false;
                            g_num_lives -= 1;
                        }
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(nullptr);
    // if player is nonexistent (start), return
    if (!g_curr_scene->get_state().player->get_texture_id()) { return; }
    /* set player to idle when not pressing buttons */
    g_curr_scene->get_state().player->set_animation_state(IDLE);
    /* left and rigt movement */
    if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A])
    {
        g_curr_scene->get_state().player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D])
    {
        g_curr_scene->get_state().player->move_right();
    }
    /* normalize movement */
    if (glm::length(g_curr_scene->get_state().player->get_movement()) > 1.0f)
    {
        g_curr_scene->get_state().player->normalise_movement();
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    if (g_num_lives == 0) {
        g_paused = true;
        g_game_over = true;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        if (g_curr_scene->m_player_dead) {
            g_paused = true;
        }
        if (!g_paused) {
            g_curr_scene->update(FIXED_TIMESTEP);
        }
        g_effects->update(FIXED_TIMESTEP);
        
        g_is_colliding_bottom = g_curr_scene->get_state().player->get_collided_bottom();
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Camera Follows the player
    g_view_matrix = glm::mat4(1.0f);
    /* special START screen camera placement*/
    if (g_curr_scene->get_state().curr_scene_id == 0) {
        g_camera_position = glm::vec3(0.0f, 0.0f, 0);
        g_view_matrix = glm::translate(g_view_matrix, g_camera_position);
        return;
    }
    // TODO: limit right edge camera maybe individual edges?
    float cam_x = -g_curr_scene->get_state().player->get_position().x;
    float cam_y = 3.25;
    // X camera limits
    /* limit camera to the left*/
    if (g_curr_scene->get_state().player->get_position().x < g_curr_scene->m_left_edge) {
        cam_x = -g_curr_scene->m_left_edge;
    }
    /* limit camera to the right*/
    else if (g_curr_scene->get_state().player->get_position().x > g_curr_scene->m_right_edge) {
        cam_x = -g_curr_scene->m_right_edge;
    }
    //Y camera limits
    /*if (g_curr_scene->m_bottom_edge < 0) {
        if (g_curr_scene->get_state().player->get_position().y < g_curr_scene->m_bottom_edge - 1.0f) {
            cam_y = -g_curr_scene->m_bottom_edge;
        }
        else {
            cam_y = -g_curr_scene->get_state().player->get_position().y - 1.0f;
        }
    }*/
    g_camera_position = glm::vec3(cam_x, cam_y, 0);
    g_view_matrix = glm::translate(g_view_matrix, g_camera_position);
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glClear(GL_COLOR_BUFFER_BIT);
       
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_curr_scene->render(&g_shader_program);
       
    g_effects->render();

    // ----- Text Display ---- // 
    /* display num lives */
    float text_x = (g_curr_scene->get_state().player->get_position().x - 3.2) /*+ g_camera_position.x*/;
    float text_y = (g_curr_scene->get_state().player->get_position().y - 1.0f) /*-1.0f + g_curr_scene->m_bottom_edge*/ ;
    std::string lives_display_txt = std::to_string(g_num_lives) + " Lives";
    glm::vec3 lives_txt_pos = glm::vec3(text_x, -1, 0.0f);
    if (lives_txt_pos.x < 0.0f) {
        lives_txt_pos = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    Utility::draw_text(&g_shader_program, g_text_id, lives_display_txt, 0.2, 0.0, 
                       lives_txt_pos);

    /* display restart prompt */
    if (g_curr_scene->m_player_dead) {
        Utility::draw_text(&g_shader_program, g_text_id, "Press [r] to restart", 0.2, 0.0,
            glm::vec3(text_x + 1.0f, -3.0f, 0.0f));
    }

    /* display lose */
    if (g_game_over && g_num_lives == 0) {
        Utility::draw_text(&g_shader_program, g_text_id, "You Lose", 0.2, 0.0,
            glm::vec3(text_x + 1.0f, - 3.0f, 0.0f));
    }
    
    /* display win*/
    if (g_game_over && g_curr_scene->get_state().next_scene_id >= 4) {
        Utility::draw_text(&g_shader_program, g_text_id, "You Win", 0.2, 0.0,
            glm::vec3(text_x + 1.0f, -3.0f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    Mix_FreeMusic(g_bgm);
    delete g_levelStart;
    delete g_levelA;
    delete g_levelB;
    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        if (g_curr_scene->get_state().next_scene_id > 0 && g_curr_scene->get_state().next_scene_id < 4) {
            switch_to_scene(g_levels[g_curr_scene->get_state().next_scene_id]);
        }
        else if (g_curr_scene->get_state().next_scene_id >= 4) {
            g_game_over = true;
        }
        render();
    }
    
    shutdown();
    return 0;
}
