#include <SDL.h>
#include <GL/glew.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <cstdlib>

#include "core/app.hpp"
#include "render/camera.hpp"

// ------------------------------------------------------------
// Data
// ------------------------------------------------------------
struct MouseState {
    float x = 0.0f;     // NDC (-1..1)
    float y = 0.0f;
    float dx = 0.0f;
    float dy = 0.0f;
    bool  active = false;
};

struct Particle {
    float x, y, vx, vy, life;
};

struct AuraParticle {
    float x, y;
    float vx, vy;
    float life;
};

struct AvatarVisual {
    float y = 0.0f;
    float phase = 0.0f;
};

// ------------------------------------------------------------
// Utilities
// ------------------------------------------------------------
static std::string load_file(const char* path)
{
    std::ifstream f(path);
    if (!f.is_open()) return {};
    return { std::istreambuf_iterator<char>(f),
             std::istreambuf_iterator<char>() };
}

static GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

static GLuint make_program(const char* vsPath, const char* fsPath)
{
    GLuint vs = compile_shader(
        GL_VERTEX_SHADER, load_file(vsPath).c_str());
    GLuint fs = compile_shader(
        GL_FRAGMENT_SHADER, load_file(fsPath).c_str());

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    return p;
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main()
{
    App app;
    if (!app.init()) return 1;

    AvatarVisual avatar;
    MouseState mouse;
    float time = 0.0f;

    // --------------------------------------------------------
    // Particles
    // --------------------------------------------------------
    constexpr int MAX      = 900;
    constexpr int AURA_MAX = 220;

    std::vector<Particle> particles(MAX);
    std::vector<AuraParticle> aura(AURA_MAX);

    for (auto& p : particles) {
        p.x = (rand() % 1000) / 500.0f - 1.0f;
        p.y = (rand() % 1000) / 500.0f - 1.0f;
        p.vx = (rand() % 1000) / 1500000.0f - 0.0003f;
        p.vy = (rand() % 1000) / 1200000.0f + 0.0002f;
        p.life = 1.0f;
    }

    for (auto& p : aura) {
        float a = (rand() % 1000) / 1000.0f * 6.28318f;
        float r = (rand() % 1000) / 1000.0f * 0.08f;
        p.x = std::cos(a) * r;
        p.y = std::sin(a) * r;
        p.vx = (rand() % 1000) / 600000.0f - 0.0008f;
        p.vy = (rand() % 1000) / 600000.0f - 0.0008f;
        p.life = 1.0f;
    }

    // --------------------------------------------------------
    // Avatar geometry
    // --------------------------------------------------------
    float avatarVerts[] = {
        -0.07f,-0.10f,  0.07f,-0.10f,  0.07f, 0.08f,
        -0.07f,-0.10f,  0.07f, 0.08f, -0.07f, 0.08f,

         0.00f, 0.18f,  0.06f, 0.14f,  0.04f, 0.22f,
         0.00f, 0.18f,  0.04f, 0.22f, -0.04f, 0.22f,
         0.00f, 0.18f, -0.04f, 0.22f, -0.06f, 0.14f,
         0.00f, 0.18f, -0.06f, 0.14f, -0.04f, 0.12f,
         0.00f, 0.18f, -0.04f, 0.12f,  0.04f, 0.12f,
         0.00f, 0.18f,  0.04f, 0.12f,  0.06f, 0.14f,

        -0.085f, 0.05f, -0.15f,-0.02f, -0.12f,-0.06f,
        -0.085f, 0.05f, -0.12f,-0.06f, -0.085f,-0.02f,

         0.085f, 0.05f,  0.15f,-0.02f,  0.12f,-0.06f,
         0.085f, 0.05f,  0.12f,-0.06f,  0.085f,-0.02f,

        -0.05f,-0.10f, -0.025f,-0.22f, -0.01f,-0.10f,
         0.05f,-0.10f,  0.025f,-0.22f,  0.01f,-0.10f
    };

    constexpr int AVATAR_VERTS = 42;

    GLuint vaoAvatar, vboAvatar;
    glGenVertexArrays(1, &vaoAvatar);
    glGenBuffers(1, &vboAvatar);
    glBindVertexArray(vaoAvatar);
    glBindBuffer(GL_ARRAY_BUFFER, vboAvatar);
    glBufferData(GL_ARRAY_BUFFER, sizeof(avatarVerts),
                 avatarVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // --------------------------------------------------------
    // Particle buffer (packed)
    // --------------------------------------------------------
    GLuint vaoParticles, vboParticles;
    glGenVertexArrays(1, &vaoParticles);
    glGenBuffers(1, &vboParticles);
    glBindVertexArray(vaoParticles);
    glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
    glBufferData(GL_ARRAY_BUFFER,
        (MAX + AURA_MAX) * 2 * sizeof(float),
        nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // --------------------------------------------------------
    // Shaders
    // --------------------------------------------------------
    GLuint meshProg = make_program(
        "assets/shaders/mesh.vert",
        "assets/shaders/mesh.frag");

    GLuint particleProg = make_program(
        "assets/shaders/particles.vert",
        "assets/shaders/particles.frag");

    GLint mOffset = glGetUniformLocation(meshProg, "uOffset");
    GLint mColor  = glGetUniformLocation(meshProg, "uColor");
    GLint mTime   = glGetUniformLocation(meshProg, "uTime");

    GLint pColor = glGetUniformLocation(particleProg, "uColor");
    GLint pSize  = glGetUniformLocation(particleProg, "uPointSize");

    // --------------------------------------------------------
    // Loop
    // --------------------------------------------------------
    while (app.running()) {
        float dt = app.frame_dt();
        time += dt;

        // ---------------- Mouse ----------------
        int mx, my, w, h;
        uint32_t buttons = SDL_GetMouseState(&mx, &my);
        SDL_GetWindowSize(app.get_window(), &w, &h);

        float nx = (mx / (float)w) * 2.0f - 1.0f;
        float ny = 1.0f - (my / (float)h) * 2.0f;

        mouse.dx = nx - mouse.x;
        mouse.dy = ny - mouse.y;
        mouse.x = nx;
        mouse.y = ny;
        mouse.active = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

        avatar.phase += dt;
        avatar.y = std::sin(avatar.phase) * 0.035f;

        std::vector<float> packed;
        packed.reserve((MAX + AURA_MAX) * 2);

        // ---------------- Particles ----------------
        constexpr float SCATTER_RADIUS = 0.15f;
        constexpr float SCATTER_FORCE  = 0.0025f;

        for (auto& p : particles) {
            p.x += p.vx;
            p.y += p.vy;

            if (mouse.active) {
                float dx = p.x - mouse.x;
                float dy = p.y - mouse.y;
                float d2 = dx*dx + dy*dy;

                if (d2 < SCATTER_RADIUS * SCATTER_RADIUS) {
                    float d = std::sqrt(d2) + 0.0001f;
                    float f = 1.0f - d / SCATTER_RADIUS;
                    p.vx += (dx / d) * f * SCATTER_FORCE + mouse.dx * 0.01f;
                    p.vy += (dy / d) * f * SCATTER_FORCE + mouse.dy * 0.01f;
                }
            }

            if ((p.life -= dt * 0.04f) <= 0.0f) {
                p.x = (rand() % 1000) / 500.0f - 1.0f;
                p.y = -1.2f;
                p.life = 1.0f;
            }

            packed.push_back(p.x);
            packed.push_back(p.y);
        }

        // ---------------- Aura ----------------
        constexpr float AURA_RADIUS = 0.20f;
        constexpr float AURA_FORCE  = 0.0012f;

        for (auto& p : aura) {
            float px = p.x;
            float py = p.y + avatar.y;

            if (mouse.active) {
                float dx = px - mouse.x;
                float dy = py - mouse.y;
                float d2 = dx*dx + dy*dy;

                if (d2 < AURA_RADIUS * AURA_RADIUS) {
                    float d = std::sqrt(d2) + 0.0001f;
                    float f = 1.0f - d / AURA_RADIUS;
                    p.vx += (dx / d) * f * AURA_FORCE;
                    p.vy += (dy / d) * f * AURA_FORCE;
                }
            }

            p.x += p.vx;
            p.y += p.vy;

            packed.push_back(p.x);
            packed.push_back(p.y + avatar.y);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            packed.size() * sizeof(float), packed.data());

        // ---------------- Render ----------------
        glClearColor(0.04f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(particleProg);
        glUniform1f(pSize, 6.0f);
        glUniform4f(pColor, 0.55f, 0.7f, 1.0f, 0.25f);
        glBindVertexArray(vaoParticles);
        glDrawArrays(GL_POINTS, 0, MAX);

        glUniform4f(pColor, 1.0f, 0.55f, 0.2f, 0.35f);
        glDrawArrays(GL_POINTS, MAX, AURA_MAX);

        glUseProgram(meshProg);
        glUniform1f(mTime, time);
        glUniform2f(mOffset, 0.0f, avatar.y);
        glUniform4f(mColor, 0.7f, 0.76f, 0.86f, 1.0f);
        glUniform1f(glGetUniformLocation(meshProg, "uScale"), 1.0f);
        glBindVertexArray(vaoAvatar);
        glDrawArrays(GL_TRIANGLES, 0, AVATAR_VERTS);

        app.present();
    }

    app.shutdown();
    return 0;
}
