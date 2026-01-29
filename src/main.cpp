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

static std::string load_file(const char* path)
{
    std::ifstream f(path);
    if (!f.is_open()) return {};
    return { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
}

static GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

int main()
{
    App app;
    if (!app.init()) return 1;

    Camera camera;
    AvatarVisual avatar;
    float time = 0.0f;

    // ---------------- Particles ----------------
    constexpr int MAX = 900;
    std::vector<Particle> particles(MAX);

constexpr int AURA_MAX = 220;
std::vector<AuraParticle> aura(AURA_MAX);

for (auto& p : aura) {
    float angle = (rand() % 1000) / 1000.0f * 6.28318f;
    float radius = (rand() % 1000) / 1000.0f * 0.08f;

    p.x = std::cos(angle) * radius;
    p.y = std::sin(angle) * radius;

    p.vx = (rand() % 1000) / 600000.0f - 0.0008f;
    p.vy = (rand() % 1000) / 600000.0f - 0.0008f;

    p.life = (rand() % 1000) / 1000.0f;
}


    for (auto& p : particles) {
        p.x = (rand() % 1000) / 500.0f - 1.0f;
        p.y = (rand() % 1000) / 500.0f - 1.0f;
        p.vx = (rand() % 1000) / 1500000.0f - 0.0003f;
        p.vy = (rand() % 1000) / 1200000.0f + 0.0002f;
        p.life = (rand() % 1000) / 1000.0f;
    }




    // ---------------- Full humanoid geometry ----------------
    float avatarVerts[] = {
        // torso
        -0.07f, -0.10f,   0.07f, -0.10f,   0.07f,  0.08f,
        -0.07f, -0.10f,   0.07f,  0.08f,  -0.07f,  0.08f,

        // rounded head (6 tris)
         0.00f,  0.18f,   0.06f,  0.14f,   0.04f,  0.22f,
         0.00f,  0.18f,   0.04f,  0.22f,  -0.04f,  0.22f,
         0.00f,  0.18f,  -0.04f,  0.22f,  -0.06f,  0.14f,
         0.00f,  0.18f,  -0.06f,  0.14f,  -0.04f,  0.12f,
         0.00f,  0.18f,  -0.04f,  0.12f,   0.04f,  0.12f,
         0.00f,  0.18f,   0.04f,  0.12f,   0.06f,  0.14f,

        // left arm
        -0.085f,  0.05f,  -0.15f, -0.02f,  -0.12f, -0.06f,
        -0.085f,  0.05f,  -0.12f, -0.06f,  -0.085f, -0.02f,

        // right arm
         0.085f,  0.05f,   0.15f, -0.02f,   0.12f, -0.06f,
         0.085f,  0.05f,   0.12f, -0.06f,   0.085f, -0.02f,

        // left upper leg
        -0.05f, -0.10f,  -0.025f, -0.22f,   -0.01f, -0.10f,

        // right upper leg
         0.05f, -0.10f,   0.025f, -0.22f,    0.01f, -0.10f,
    };

    constexpr int AVATAR_VERTS = 42;

    GLuint vaoAvatar, vboAvatar;
    glGenVertexArrays(1, &vaoAvatar);
    glGenBuffers(1, &vboAvatar);
    glBindVertexArray(vaoAvatar);
    glBindBuffer(GL_ARRAY_BUFFER, vboAvatar);
    glBufferData(GL_ARRAY_BUFFER, sizeof(avatarVerts), avatarVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ---------------- Particles buffer ----------------
    GLuint vaoParticles, vboParticles;
    glGenVertexArrays(1, &vaoParticles);
    glGenBuffers(1, &vboParticles);
    glBindVertexArray(vaoParticles);
    glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
    glBufferData(GL_ARRAY_BUFFER, MAX * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ---------------- Fullscreen quad (vignette) ----------------
    float quad[] = {
        -1,-1,  1,-1,  1, 1,
        -1,-1,  1, 1, -1, 1
    };

    GLuint vaoQuad, vboQuad;
    glGenVertexArrays(1, &vaoQuad);
    glGenBuffers(1, &vboQuad);
    glBindVertexArray(vaoQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ---------------- Shaders ----------------
    GLuint vs = compile_shader(GL_VERTEX_SHADER, load_file("assets/shaders/basic.vert").c_str());
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, load_file("assets/shaders/basic.frag").c_str());

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint uOffset   = glGetUniformLocation(program, "uOffset");
    GLint uScale    = glGetUniformLocation(program, "uScale");
    GLint uColor    = glGetUniformLocation(program, "uColor");
    GLint uCircle   = glGetUniformLocation(program, "uCircle");
    GLint uGlow     = glGetUniformLocation(program, "uGlow");
    GLint uVignette = glGetUniformLocation(program, "uVignette");
    GLint uTime     = glGetUniformLocation(program, "uTime");
    GLint uPointSize= glGetUniformLocation(program, "uPointSize");

    while (app.running()) {
        float dt = app.frame_dt();
        time += dt;

        avatar.phase += dt;
        avatar.y = std::sin(avatar.phase * 1.0f) * 0.035f;

        std::vector<float> pts;
        pts.reserve(MAX * 2);
        for (auto& p : particles) {
            p.x += p.vx;
            p.y += p.vy;
            p.life -= dt * 0.04f;
            if (p.life <= 0.0f) {
                p.x = (rand() % 1000) / 500.0f - 1.0f;
                p.y = -1.2f;
                p.life = 1.0f;
            }
            pts.push_back(p.x);
            pts.push_back(p.y);
        }

        std::vector<float> auraPts;
auraPts.reserve(AURA_MAX * 2);

for (auto& p : aura) {
    p.x += p.vx;
    p.y += p.vy;
    p.life -= dt * 0.5f;

    if (p.life <= 0.0f) {
        float angle = (rand() % 1000) / 1000.0f * 6.28318f;
        float radius = (rand() % 1000) / 1000.0f * 0.08f;

        p.x = std::cos(angle) * radius;
        p.y = std::sin(angle) * radius;

        p.vx = (rand() % 1000) / 600000.0f - 0.0008f;
        p.vy = (rand() % 1000) / 600000.0f - 0.0008f;

        p.life = 1.0f;
    }

    auraPts.push_back(p.x);
    auraPts.push_back(p.y + avatar.y);
}

        glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pts.size() * sizeof(float), pts.data());
        glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
glBufferSubData(GL_ARRAY_BUFFER, 0,
                auraPts.size() * sizeof(float),
                auraPts.data());

        glClearColor(0.04f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniform1f(uTime, time);

        // particles
        glUniform1f(uPointSize, 26.0f);
        glUniform4f(uColor, 0.55f, 0.7f, 1.0f, 0.25f);
        glUniform1i(uCircle, 1);
        glUniform1i(uGlow, 0);
        glUniform1i(uVignette, 0);
        glUniform2f(uOffset, 0, 0);
        glUniform1f(uScale, 1.0f);
        glBindVertexArray(vaoParticles);
        glDrawArrays(GL_POINTS, 0, MAX);


        
// avatar silhouette glow (geometry pass)
glUniform1f(uPointSize, 1.0f);
glUniform4f(uColor, 1.0f, 0.7f, 0.3f, 0.22f);
glUniform1i(uGlow, 1);
glUniform1i(uCircle, 0);

glUniform1f(uScale, 1.06f);
glUniform2f(uOffset, 0.0f, avatar.y);

glBindVertexArray(vaoAvatar);
glDrawArrays(GL_TRIANGLES, 0, AVATAR_VERTS);


        // slightly scale up silhouette
        glUniform1f(uScale, 1.06f);
        glUniform2f(uOffset, 0.0f, avatar.y);

        glBindVertexArray(vaoAvatar);
        glDrawArrays(GL_TRIANGLES, 0, AVATAR_VERTS);

        // avatar geometry (foreground)
        glUniform1f(uPointSize, 1.0f);
        glUniform4f(uColor, 0.92f, 0.92f, 1.0f, 1.0f);
        glUniform1i(uGlow, 0);
        glUniform1i(uCircle, 0);
        glUniform1f(uScale, 1.0f);
        glUniform2f(uOffset, 0.0f, avatar.y);

        glBindVertexArray(vaoAvatar);
        glDrawArrays(GL_TRIANGLES, 0, AVATAR_VERTS);

        // avatar aura particles (orange dust)
glUniform1f(uPointSize, 10.0f);
glUniform4f(uColor, 1.0f, 0.55f, 0.2f, 0.35f);
glUniform1i(uCircle, 1);
glUniform1i(uGlow, 0);
glUniform2f(uOffset, 0.0f, 0.0f);
glUniform1f(uScale, 1.0f);

glBindVertexArray(vaoParticles);
glDrawArrays(GL_POINTS, 0, AURA_MAX);


        // vignette
        glUniform1i(uVignette, 1);
        glBindVertexArray(vaoQuad);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        app.present();
    }

    app.shutdown();
    return 0;
}
