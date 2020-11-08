#ifndef TEDIT_WINDOW_
#define TEDIT_WINDOW_

#include "pch.h"

struct GLFWwindow;

/**
 * GLFW RAII wrapper
 */
class Window final
{
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void pollInput();
    void close();
    bool shouldClose() const;
    void swapBuffers();

    void setTitle(const std::string& title);

    GLFWwindow* getHandle() const;
    int getWidth() const;
    int getHeight() const;

    /**
     * @param listener arguments are (xpos, ypos)
     */
    void addMouseMoveListener(std::function<void(double, double)> listener);
    /**
     * @param listener arguments are (button, action, modifiers)
     */
    void addMouseButtonListener(std::function<void(int, int, int)> listener);
    /**
     * @param listener arguments are (key, action, modifiers)
     */
    void addKeyListener(std::function<void(int, int, int)> listener);

private:
    GLFWwindow* handle;
    std::string title;
    int width;
    int height;

    struct
    {
        std::vector<std::function<void(double, double)>> mouseMove;
        std::vector<std::function<void(int, int, int)>> mouseButton;
        std::vector<std::function<void(int, int, int)>> key;
    } listeners;

    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int modifiers);
    void onKey(int key, int scancode, int action, int modifiers);
    void onResize(int width, int height);

    friend void global_HandleGLFWMouseMove(GLFWwindow* handle, double xpos, double ypos);
    friend void global_HandleGLFWMouseButton(GLFWwindow* handle, int button, int action, int modifiers);
    friend void global_HandleGLFWKey(GLFWwindow* handle, int key, int scancode, int action, int modifiers);
    friend void global_HandleGLFWResize(GLFWwindow* handle, int width, int height);
}; // class Window

#endif // TEDIT_WINDOW_
