#include "render/camera.hpp"
#include <cmath>

void Camera::update(float dt)
{
    sway_phase += dt;
    float sway = std::sin(sway_phase * 0.6f) * 0.03f;
    position.y = 1.2f + sway;
}
