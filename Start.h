#include "Scene.h"

class Start : public Scene {
public:
    GLuint g_font_texture_id;
    ShaderProgram g_program;

    ~Start();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
