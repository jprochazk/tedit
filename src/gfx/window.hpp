#ifndef TEDIT_WINDOW_
#define TEDIT_WINDOW_

#include <cstdint>
#include <cassert>
#include <string>

struct GLFWwindow;

/**
 * GLFW RAII wrapper
 */
class Window final
{
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void resize(int width, int height);

    void close();
    bool shouldClose() const;
    void swapBuffers();

    void setTitle(const std::string& title);

    uint32_t getId() const;
    GLFWwindow* getHandle() const;
    int getWidth() const;
    int getHeight() const;

private:
    uint32_t id;
    GLFWwindow* handle;
    std::string title;
    int width;
    int height;

    friend void onMouseMove(GLFWwindow* handle, double xpos, double ypos);
    friend void onMouseButton(GLFWwindow* handle, int button, int action, int modifiers);
    friend void onKey(GLFWwindow* handle, int key, int scancode, int action, int modifiers);
    friend void onResize(GLFWwindow* handle, int width, int height);
}; // class Window

#endif // TEDIT_WINDOW_
