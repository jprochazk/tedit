
#include "pch.h"
#include "window.hpp"
#include "gfx/gl.h"
#include <portable-file-dialogs.h>

// window instance for event listeners
static Window* window = nullptr;

static void
global_HandleGLFWMouseMove(GLFWwindow*, double xpos, double ypos)
{
    window->onMouseMove(xpos, ypos);
}

static void
global_HandleGLFWMouseButton(GLFWwindow*, int button, int action, int modifiers)
{
    window->onMouseButton(button, action, modifiers);
}

static void
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
global_HandleGLFWScroll(GLFWwindow*, double xoffset, double yoffset)
{
    window->onScroll(xoffset, yoffset);
}

static void
onError(int error, const char* description)
{
    spdlog::error("GLFW Error {}: {}", error, description);
}

Window::Window(const std::string& title, int width, int height)
  : handle_(nullptr)
  , title_(title)
  , width_(width)
  , height_(height)
  , listeners_()
  , dialogOpen_(false)
{
    glfwSetErrorCallback(onError);
    assert(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    // sorry apple
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->handle_ = glfwCreateWindow(this->width_, this->height_, this->title_.c_str(), NULL, NULL);
    assert(this->handle_);

    glfwSetCursorPosCallback(this->handle_, ::global_HandleGLFWMouseMove);
    glfwSetMouseButtonCallback(this->handle_, ::global_HandleGLFWMouseButton);
    glfwSetKeyCallback(this->handle_, ::global_HandleGLFWKey);
    glfwSetScrollCallback(this->handle_, ::global_HandleGLFWScroll);
    glfwSetFramebufferSizeCallback(this->handle_, ::global_HandleGLFWResize);

    glfwMakeContextCurrent(this->handle_);
    glfwSwapInterval(1); // Enable vsync

    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    assert(window == nullptr);
    window = this;
}

Window::~Window()
{
    std::unique_lock<std::mutex> lock(this->dialogMutex_);
    glfwDestroyWindow(this->handle_);
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
    glfwSetWindowShouldClose(this->handle_, GLFW_TRUE);
}

bool
Window::shouldClose() const
{
    return glfwWindowShouldClose(this->handle_);
}

void
Window::swapBuffers()
{
    glfwSwapBuffers(this->handle_);
}

void
Window::openDialog(Dialog type,
                   const std::string& title,
                   const std::vector<std::string>& filters,
                   std::function<void(bool, std::vector<std::string>&)> callback)
{
    if (this->dialogOpen_) {
        spdlog::warn("Dialog already open!");
        return;
    }

    this->dialogOpen_ = true;

    std::thread([type = type,
                 title = title,
                 filters = filters,
                 callback = callback,
                 dialogOpen = &dialogOpen_,
                 dialogMutex = &dialogMutex_] {
        std::unique_lock<std::mutex> lock(*dialogMutex);
        bool success = false;
        std::vector<std::string> selection;
        switch (type) {
            case Dialog::OpenFile: {
                auto fd = pfd::open_file(title, "", filters, pfd::opt::multiselect);
                while (!fd.ready()) {
                    if (window->shouldClose()) {
                        fd.kill();
                        break;
                    }
                }
                auto temp_selection = fd.result();
                if (!temp_selection.empty()) {
                    success = true;
                    selection = std::move(temp_selection);
                }
                break;
            }
            case Dialog::SaveFile: {
                auto fd = pfd::save_file(title, "", filters);
                while (!fd.ready()) {
                    if (window->shouldClose()) {
                        fd.kill();
                        break;
                    }
                }
                auto temp_selection = fd.result();
                if (!temp_selection.empty()) {
                    success = true;
                    selection = { std::move(temp_selection) };
                }
                break;
            }
            case Dialog::SelectFolder: {
                auto fd = pfd::select_folder(title);
                while (!fd.ready()) {
                    if (window->shouldClose()) {
                        fd.kill();
                        break;
                    }
                }
                auto temp_selection = fd.result();
                if (!temp_selection.empty()) {
                    success = true;
                    selection = { std::move(temp_selection) };
                }
                break;
            }
        }
        *dialogOpen = false;
        callback(success, selection);
    }).detach();
}

bool
Window::isDialogOpen() const
{
    return this->dialogOpen_;
}

void
Window::setTitle(const std::string& title)
{
    this->title_ = title;
    glfwSetWindowTitle(this->handle_, this->title_.c_str());
}

GLFWwindow*
Window::handle() const
{
    return this->handle_;
}

int
Window::width() const
{
    return this->width_;
}

int
Window::height() const
{
    return this->height_;
}

void
Window::addMouseMoveListener(std::function<void(double, double)> listener)
{
    this->listeners_.mouseMove.emplace_back(listener);
}

void
Window::addMouseButtonListener(std::function<void(int, int, int)> listener)
{
    this->listeners_.mouseButton.emplace_back(listener);
}

void
Window::addKeyListener(std::function<void(int, int, int)> listener)
{
    this->listeners_.key.emplace_back(listener);
}

void
Window::addScrollListener(std::function<void(double, double)> listener)
{
    this->listeners_.scroll.emplace_back(listener);
}

void
Window::onMouseMove(double xpos, double ypos)
{
    for (const auto& listener : this->listeners_.mouseMove) {
        listener(xpos, ypos);
    }
}

void
Window::onMouseButton(int button, int action, int modifiers)
{
    for (const auto& listener : this->listeners_.mouseButton) {
        listener(button, action, modifiers);
    }
}

void
Window::onKey(int key, int /* scancode */, int action, int modifiers)
{
    for (const auto& listener : this->listeners_.key) {
        listener(key, action, modifiers);
    }
}

void
Window::onScroll(double xoffset, double yoffset)
{
    for (const auto& listener : this->listeners_.scroll) {
        listener(xoffset, yoffset);
    }
}

void
Window::onResize(int width, int height)
{
    this->width_ = width;
    this->height_ = height;
    glViewport(0, 0, width, height);
}