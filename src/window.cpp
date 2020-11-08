
#include "pch.h"
#include "window.hpp"
#include "gfx/gl.h"

// window instance for event listeners
static Window* window = nullptr;

void
global_HandleGLFWMouseMove(GLFWwindow*, double xpos, double ypos)
{
    window->onMouseMove(xpos, ypos);
}

void
global_HandleGLFWMouseButton(GLFWwindow*, int button, int action, int modifiers)
{
    window->onMouseButton(button, action, modifiers);
}

void
global_HandleGLFWKey(GLFWwindow*, int key, int scancode, int action, int modifiers)
{
    window->onKey(key, scancode, action, modifiers);
}

static void
global_HandleGLFWResize(GLFWwindow*, int width, int height)
{
    window->onResize(width, height);
}

static void
onError(int error, const char* description)
{
    spdlog::error("GLFW Error {}: {}", error, description);
}

Window::Window(const std::string& title, int width, int height)
  : handle(nullptr)
  , title(title)
  , width(width)
  , height(height)
  , listeners()
{
    glfwSetErrorCallback(onError);
    assert(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    // sorry apple
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->handle = glfwCreateWindow(this->width, this->height, this->title.c_str(), NULL, NULL);
    assert(this->handle);

    glfwSetCursorPosCallback(this->handle, ::global_HandleGLFWMouseMove);
    glfwSetMouseButtonCallback(this->handle, ::global_HandleGLFWMouseButton);
    glfwSetKeyCallback(this->handle, ::global_HandleGLFWKey);
    glfwSetFramebufferSizeCallback(this->handle, ::global_HandleGLFWResize);

    glfwMakeContextCurrent(this->handle);
    glfwSwapInterval(1); // Enable vsync

    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    assert(window == nullptr);
    window = this;
}

Window::~Window()
{
    glfwDestroyWindow(this->handle);
    glfwTerminate();
}

void
Window::pollInput()
{
    glfwPollEvents();
}

void
Window::close()
{
    glfwSetWindowShouldClose(this->handle, GLFW_TRUE);
}

bool
Window::shouldClose() const
{
    return glfwWindowShouldClose(this->handle);
}

void
Window::swapBuffers()
{
    glfwSwapBuffers(this->handle);
}

void
Window::setTitle(const std::string& title)
{
    this->title = title;
    glfwSetWindowTitle(this->handle, this->title.c_str());
}

GLFWwindow*
Window::getHandle() const
{
    return this->handle;
}

int
Window::getWidth() const
{
    return this->width;
}

int
Window::getHeight() const
{
    return this->height;
}

void
Window::addMouseMoveListener(std::function<void(double, double)> listener)
{
    this->listeners.mouseMove.emplace_back(listener);
}

void
Window::addMouseButtonListener(std::function<void(int, int, int)> listener)
{
    this->listeners.mouseButton.emplace_back(listener);
}

void
Window::addKeyListener(std::function<void(int, int, int)> listener)
{
    this->listeners.key.emplace_back(listener);
}

void
Window::onMouseMove(double xpos, double ypos)
{
    for (const auto& listener : this->listeners.mouseMove) {
        listener(xpos, ypos);
    }
}

void
Window::onMouseButton(int button, int action, int modifiers)
{
    for (const auto& listener : this->listeners.mouseButton) {
        listener(button, action, modifiers);
    }
}

void
Window::onKey(int key, int /* scancode */, int action, int modifiers)
{
    for (const auto& listener : this->listeners.key) {
        listener(key, action, modifiers);
    }
}

void
Window::onResize(int width, int height)
{
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
}