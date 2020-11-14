#include "pch.h"

#ifndef TEDIT_CAMERA_
#define TEDIT_CAMERA_

class Window;

namespace gfx {

class Camera
{
public:
    Camera(Window* window);

    void resize(int width, int height);
    void move(glm::vec2 to);
    void zoom(float wheel);
    void zoom(float wheel, glm::vec2 mouse);

    /**
     * Map mouse position from screen to world coordinates
     */
    glm::vec2 world(glm::vec2 mouse) const;

    const glm::mat4& projection() const;
    const glm::mat4& view() const;
    const glm::vec2& pos() const;
    float zoom() const;

private:
    Window* window_;
    glm::mat4 proj_;
    glm::mat4 view_;
    glm::vec2 pos_;
    glm::vec4 viewport_;
    float zoom_;
}; // class Camera

} // namespace gfx

#endif // TEDIT_CAMERA_
