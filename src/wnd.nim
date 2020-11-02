import nimgl/[glfw]
import std/[atomics]

var HAS_WINDOW = false

type Window* = object
    handle*: GLFWWindow

proc `=destroy`*(window: var Window) =
    destroyWindow(window.handle)
    glfwTerminate()

proc close*(window: Window) = setWindowShouldClose(window.handle, true)
proc shouldClose*(window: Window): bool = windowShouldClose(window.handle)
proc swapBuffers*(window: Window) = swapBuffers(window.handle)

proc newWindow*(name: string, onKey: GLFWKeyFun): Window =
    assert not HAS_WINDOW
    HAS_WINDOW = true

    assert glfwInit()

    glfwWindowHint(GLFWContextVersionMajor, 3)
    glfwWindowHint(GLFWContextVersionMinor, 3)
    glfwWindowHint(GLFWOpenglForwardCompat, GLFW_TRUE)
    glfwWindowHint(GLFWOpenglProfile, GLFW_OPENGL_CORE_PROFILE)
    glfwWindowHint(GLFWResizable, GLFW_TRUE)

    let w = glfwCreateWindow(1600, 900, "NimGL", nil, nil)
    assert w != nil

    discard w.setKeyCallback(onKey)
    w.makeContextCurrent()

    Window(
        handle: w
    )
