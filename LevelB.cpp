#include "LevelB.h"
#include "Utility.h"
#include <vector>

#define LEVEL_HEIGHT 7
#define LEVEL_WIDTH 16


unsigned int LevelB_DATA[] =
{
   24, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
   24, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
   24, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
   24, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
   24, 42, 42, 42, 61, 63, 42, 42, 42, 42,122, 42, 42, 61, 62, 63,
   24, 42, 42, 42, 42, 42, 42, 42, 42,122, 42, 42, 42, 42, 42, 42,
   24,  0,  1,  2, 42, 42, 42,100,102, 42, 42, 42, 42, 42, 42, 42,
};

LevelB::~LevelB()
{
    delete m_game_state.player;
    delete m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
}

void LevelB::initialise()
{
    // ----- ATTRIBUTES ----- //
    //CHANGE: curr scene id
    m_game_state.curr_scene_id = 2;
    m_game_state.next_scene_id = -1;

    m_number_of_enemies = 1;

    // CHANGE: left and right edge x positions
    m_left_edge = 3.5f;
    m_right_edge = 11.8f;
    m_bottom_edge = -5.4;

    // ------ MAP ----- //
    GLuint map_texture_id = Utility::load_texture("assets/TerrainTileSheet.png");
    // CHANGE: map width / height
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LevelB_DATA, map_texture_id,
        1.0f, 22, 11);

    // ----- EFFECTS ----- //
    m_game_state.effect = new Effects(*m_projection_mat, *m_view_matrix);
    m_game_state.effect->start(FADEIN, 1.0f);

    // ----- PLAYER ----- //
    std::vector<std::vector<int>> player_animations =
    {
        {0, 1, 2, 3},              // IDLE animation frames
        {6, 7, 8, 9, 10, 11},      // MOVE_RIGHT animation frames
        {12, 13, 14, 15, 16, 17},  // MOVE_LEFT animation frames
        {19}                       // JUMP animation frames
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    GLuint player_texture_id = Utility::load_texture("assets/DinoSpritesSheet.png");

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.0f,                      // speed
        acceleration,              // acceleration
        4.0f,                      // jumping power
        player_animations,         // animation index sets
        IDLE,                      // animation type
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        6,                         // animation column amount
        4,                         // animation row amount
        0.8333f,                   // width (15/18)
        1.0f,                      // height (18/18)
        PLAYER                     // entity type
    );
    // CHANGE: player init pos?
    m_player_init_pos = glm::vec3(2.0f, -4.5f, 0.0f);
    m_game_state.player->set_position(m_player_init_pos);

    // ----- ENEMY ----- // 
    GLuint enemy_texture_id = Utility::load_texture("assets/EnemySpritesSheet.png");
    std::vector<std::vector<int>> enemy_animations = {
       {0, 1, 2, 3},              // AI_IDLE animation frames
       {6, 7, 8, 9, 10, 11},      // AI_RIGHT animation frames
       {12, 13, 14, 15, 16, 17},  // AI_LEFT animation frames
       {0},                       // AI_JUMP animation frames
       {0, 1, 2}                        // AI_SHOOT animation frames
    };
    // CHANGE: enemy type
    m_game_state.enemy = new Entity(
        enemy_texture_id,
        0.0f,                   // speed
        0.8333f,                // width (15/18)
        1.0f,                   // height (18/18)
        ENEMY,                  // entity type
        JUMPER,                 // AI type
        JUMPING,                // AI State 
        AI_JUMP,                // AI Animation
        enemy_animations,
        0.7f,                   // jump power
        1,                      // animation frames
        6,                      // animation cols
        3,                      // animation rows
        acceleration
    );
    // CHANGE: enemy init pos
    m_enemy_init_pos = glm::vec3(7.0f, -2.0f, 0.0f);
    m_game_state.enemy->set_position(m_enemy_init_pos);

    // ----- AUDIO ----- //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.jump_sfx = Mix_LoadWAV("assets/player_jump.wav");
}

void LevelB::update(float delta_time)
{
    /* update the player */
    m_game_state.player->update(delta_time, NULL,
        m_game_state.enemy, 1, m_game_state.map);
    /* update the enemy */
    m_game_state.enemy->update(delta_time, m_game_state.player, NULL,
        0, m_game_state.map);
    /* update the effects */
    m_game_state.effect->update(delta_time);

    /* death from hitting the bottom of screen */
    // CHANGE
    if (m_game_state.player->get_position().y < m_bottom_edge) {
        m_player_dead = true;
    }
    /* death from hitting ai */
    if (m_game_state.player->get_player_died()) {
        m_player_dead = true;
    }

    /* flip the next scene flag from -1 to scene # */
    //CHANGE: next scene x pos val
    if (m_game_state.player->get_position().x > 15.0) {
        m_game_state.next_scene_id = m_game_state.curr_scene_id + 1;
    }

}

void LevelB::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    m_game_state.enemy->render(program);
    m_game_state.effect->render();

}
