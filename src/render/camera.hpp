#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    void update(float dt);

    glm::vec3 position {0.0f, 1.2f, 3.0f};
    glm::vec3 target   {0.0f, 1.0f, 0.0f};

private:
    float sway_phase = 0.0f;
};
