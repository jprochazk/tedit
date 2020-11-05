import nimgl/[glfw, opengl]

var
    currentWidth: int = 1600
    currentHeight: int = 900

proc onResize(window: GLFWWindow, width: int32, height: int32) {.cdecl.} =
    currentWidth = width
    currentHeight = height
    glViewport(0, 0, width, height)

type Window* = object
    ## RAII-style wrapper over a GLFW window
    handle*: GLFWWindow

proc `=destroy`*(window: var Window) =
    destroyWindow(window.handle)
    glfwTerminate()

proc newWindow*(name: string, onKey: GLFWKeyFun): Window =
    ## Creates and initializes a window.
    ## Only one window may be created.

    var HAS_WINDOW {.global.} = false
    assert not HAS_WINDOW
    HAS_WINDOW = true

    assert glfwInit()

    glfwWindowHint(GLFWContextVersionMajor, 4)
    glfwWindowHint(GLFWContextVersionMinor, 5)
    glfwWindowHint(GLFWOpenglForwardCompat, GLFW_TRUE)
    glfwWindowHint(GLFWOpenglProfile, GLFW_OPENGL_CORE_PROFILE)
    glfwWindowHint(GLFWResizable, GLFW_TRUE)

    let w = glfwCreateWindow(1600, 900, "NimGL", nil, nil)
    assert w != nil

    discard w.setKeyCallback(onKey)
    discard w.setWindowSizeCallback(onResize)
    w.makeContextCurrent()

    assert glInit()

    Window(
        handle: w,
    )

proc close*(window: Window) = setWindowShouldClose(window.handle, true)
proc shouldClose*(window: Window): bool = windowShouldClose(window.handle)
proc swapBuffers*(window: Window) = swapBuffers(window.handle)
proc width*(window: Window): int = currentWidth
proc height*(window: Window): int = currentHeight
