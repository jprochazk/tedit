
#include "window.hpp"
#include "gfx/gl.h"
#include <map>
#include <spdlog/spdlog.h>

static Window* window;

// void
// onMouseMove(GLFWwindow* handle, double xpos, double ypos)
//{
//}
//
// void
// onMouseButton(GLFWwindow* handle, int button, int action, int modifiers)
//{
//
//}
//
// void
// onKey(GLFWwindow* handle, int key, int scancode, int action, int modifiers)
//{
//
//}

static void
onResize(GLFWwindow* handle, int width, int height)
{
    window->resize(width, height);
}

static void
onError(int error, const char* description)
{
    spdlog::error("GLFW Error {}: {}", error, description);
}

Window::Window(const std::string& title, int width, int height)
  : title(title)
  , width(width)
  , height(height)
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

    // glfwSetCursorPosCallback(this->handle, onMouseMove);
    // glfwSetMouseButtonCallback(this->handle, onMouseButton);
    // glfwSetKeyCallback(this->handle, onKey);
    glfwSetFramebufferSizeCallback(this->handle, onResize);

    glfwMakeContextCurrent(this->handle);
    glfwSwapInterval(1); // Enable vsync

    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    window = this;
}

Window::~Window()
{
    glfwDestroyWindow(this->handle);
    glfwTerminate();
}

void
Window::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
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

uint32_t
Window::getId() const
{
    return this->id;
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