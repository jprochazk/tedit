# Copyright 2018, NimGL contributors.

import nimgl/[glfw, opengl, imgui], nimgl/imgui/[impl_opengl, impl_glfw]
import glm

proc keyProc(window: GLFWWindow, key: int32, scancode: int32, action: int32, mods: int32): void {.cdecl.} =
    if key == GLFWKey.Escape and action == GLFWPress:
        window.setWindowShouldClose(true)
    if key == GLFWKey.Space:
        glPolygonMode(GL_FRONT_AND_BACK, if action != GLFWRelease: GL_LINE else: GL_FILL)

proc statusShader(shader: uint32) =
    var status: int32
    glGetShaderiv(shader, GL_COMPILE_STATUS, addr status);
    if status != GL_TRUE.ord:
        var
            log_length: int32
            message = newSeq[char](1024)
        glGetShaderInfoLog(shader, 1024, addr log_length, addr message[0]);
        echo message

proc toRGB(vec: Vec3[float32]): Vec3[float32] =
    return vec3(vec.x / 255, vec.y / 255, vec.z / 255)

when isMainModule:
    # GLFW
    assert glfwInit()

    glfwWindowHint(GLFWContextVersionMajor, 3)
    glfwWindowHint(GLFWContextVersionMinor, 3)
    glfwWindowHint(GLFWOpenglForwardCompat, GLFW_TRUE)
    glfwWindowHint(GLFWOpenglProfile, GLFW_OPENGL_CORE_PROFILE)
    glfwWindowHint(GLFWResizable, GLFW_TRUE)

    let w: GLFWWindow = glfwCreateWindow(1600, 900, "NimGL", nil, nil)
    assert w != nil

    discard w.setKeyCallback(keyProc)
    w.makeContextCurrent

    # Opengl
    assert glInit()

    echo $glVersionMajor & "." & $glVersionMinor

    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    var
        mesh: tuple[vbo, vao, ebo: uint32]
        vertex: uint32
        fragment: uint32
        program: uint32

    var vert = @[
         1.0f, 1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 1.0f
    ]

    var ind = @[
        0'u32, 1'u32, 3'u32,
        1'u32, 2'u32, 3'u32
    ]

    glGenBuffers(1, addr mesh.vbo)
    glGenBuffers(1, addr mesh.ebo)
    glGenVertexArrays(1, addr mesh.vao)

    glBindVertexArray(mesh.vao)

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo)

    glBufferData(GL_ARRAY_BUFFER, cint(cfloat.sizeof * vert.len), addr vert[0], GL_STATIC_DRAW)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cint(cuint.sizeof * ind.len), addr ind[0], GL_STATIC_DRAW)

    glEnableVertexAttribArray(0)
    glVertexAttribPointer(0'u32, 2, EGL_FLOAT, false, cfloat.sizeof * 2, nil)

    vertex = glCreateShader(GL_VERTEX_SHADER)
    var vsrc: cstring = """
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0) * uMVP;
}
    """
    glShaderSource(vertex, 1'i32, addr vsrc, nil)
    glCompileShader(vertex)
    statusShader(vertex)

    fragment = glCreateShader(GL_FRAGMENT_SHADER)
    var fsrc: cstring = """
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0f);
}
    """
    glShaderSource(fragment, 1, addr fsrc, nil)
    glCompileShader(fragment)
    statusShader(fragment)

    program = glCreateProgram()
    glAttachShader(program, vertex)
    glAttachShader(program, fragment)
    glLinkProgram(program)

    var
        log_length: int32
        message = newSeq[char](1024)
        pLinked: int32
    glGetProgramiv(program, GL_LINK_STATUS, addr pLinked);
    if pLinked != GL_TRUE.ord:
        glGetProgramInfoLog(program, 1024, addr log_length, addr message[0]);
        echo message

    let
        uColor = glGetUniformLocation(program, "uColor")
        uMVP = glGetUniformLocation(program, "uMVP")
    var
        bg = vec3(33f, 33f, 33f).toRgb()
        color = vec3(50f, 205f, 50f).toRgb()
        mvp = ortho(-2f, 2f, -1.5f, 1.5f, -1f, 1f)

    let context = igCreateContext()

    # ImGui
    assert igGlfwInitForOpengl(w, true)
    assert igOpenGL3Init()

    igStyleColorsDark()

    var show_demo: bool = true
    var somefloat: float32 = 0.0f
    var counter: int32 = 0

    while not w.windowShouldClose:
        glfwPollEvents()

        igOpenGL3NewFrame()
        igGlfwNewFrame()
        igNewFrame()

        if show_demo:
            igShowDemoWindow(addr show_demo)

        igBegin("Hello, world!")
        igText("This is some useful text.")
        igCheckbox("Demo Window", show_demo.addr)
        igSliderFloat("float", somefloat.addr, 0.0f, 1.0f)
        if igButton("Button", ImVec2(x: 0, y: 0)): inc(counter)
        igSameLine()
        igText("counter = %d", counter)
        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO().framerate, igGetIO().framerate)
        igEnd()

        igRender()

        glClearColor(bg.r, bg.g, bg.b, 1f)
        glClear(GL_COLOR_BUFFER_BIT)

        glUseProgram(program)
        glUniform3fv(uColor, 1, color.caddr)
        glUniformMatrix4fv(uMVP, 1, false, mvp.caddr)

        glBindVertexArray(mesh.vao)
        glDrawElements(GL_TRIANGLES, ind.len.cint, GL_UNSIGNED_INT, nil)

        igOpenGL3RenderDrawData(igGetDrawData())

        w.swapBuffers

    igOpenGL3Shutdown()
    igGlfwShutdown()
    context.igDestroyContext()

    w.destroyWindow

    glfwTerminate()

    glDeleteVertexArrays(1, addr mesh.vao)
    glDeleteBuffers(1, addr mesh.vbo)
    glDeleteBuffers(1, addr mesh.ebo)
