# Copyright 2018, NimGL contributors.

import nimgl/[glfw, opengl, imgui], nimgl/imgui/[impl_opengl, impl_glfw]
import glm
import options

import ./wnd, ./gl

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

proc main =
    var window = newWindow("Hello, World!", keyProc)

    # Opengl
    assert glInit()

    var image: Image = newImage("AYAYA.png")

    var
        vertex: uint32
        fragment: uint32
        program: uint32

    vertex = glCreateShader(GL_VERTEX_SHADER)
    var vsrc: cstring = """
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
uniform mat4 uMVP;
out vec2 vUV;
void main() {
    vUV = aUV;
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
in vec2 vUV;
uniform vec3 uColor;
uniform sampler2D uTexture;
void main() {
    FragColor = texture(uTexture, vUV);
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

    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    var vert = @[
        +1.0f, +1.0f, 1.0f, 1.0f,
        +1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, 0.0f, 1.0f
    ]

    var ind = @[
        0'u32, 1'u32, 3'u32,
        1'u32, 2'u32, 3'u32
    ]

    #var mesh: tuple[vbo, vao, ebo: uint32]
    var vbo, vao, ebo: uint32
    glGenBuffers(1, addr vbo)
    glGenBuffers(1, addr ebo)
    glGenVertexArrays(1, addr vao)

    glBindVertexArray(vao)

    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)

    glBufferData(GL_ARRAY_BUFFER, cint(cfloat.sizeof * vert.len), addr vert[0], GL_STATIC_DRAW)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cint(cuint.sizeof * ind.len), addr ind[0], GL_STATIC_DRAW)

    glVertexAttribPointer(0'u32, 2, EGL_FLOAT, false, (cfloat.sizeof * 2) * 2, nil)
    glEnableVertexAttribArray(0)

    glVertexAttribPointer(1'u32, 2, EGL_FLOAT, false, (cfloat.sizeof * 2) * 2, cast[pointer](cfloat.sizeof * 2))
    glEnableVertexAttribArray(1)

    let
        uColor = glGetUniformLocation(program, "uColor")
        uMVP = glGetUniformLocation(program, "uMVP")
        uTexture = glGetUniformLocation(program, "uTexture")
    var
        bg = vec3(33f, 33f, 33f).toRgb()
        color = vec3(50f, 205f, 50f).toRgb()
        mvp = ortho(-2f, 2f, -1.5f, 1.5f, -1f, 1f)

    let context = igCreateContext()

    # ImGui
    assert igGlfwInitForOpengl(window.handle, true)
    assert igOpenGL3Init()

    igStyleColorsDark()

    var show_demo: bool = true
    var somefloat: float32 = 0.0f
    var counter: int32 = 0

    while not window.shouldClose():
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
        igImage(cast[pointer](image.handle),
            ImVec2(x: float(image.width), y: float(image.height)),
            ImVec2(x: 0f, y: 1f), ImVec2(x: 1f, y: 0f))
        igEnd()

        igRender()

        glClearColor(bg.r, bg.g, bg.b, 1f)
        glClear(GL_COLOR_BUFFER_BIT)

        glUseProgram(program)
        glUniform3fv(uColor, 1, color.caddr)
        glUniformMatrix4fv(uMVP, 1, false, mvp.caddr)
        glUniform1i(uTexture, 0)

        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, image.handle)

        glBindVertexArray(vao)
        glDrawElements(GL_TRIANGLES, ind.len.cint, GL_UNSIGNED_INT, nil)

        igOpenGL3RenderDrawData(igGetDrawData())

        window.swapBuffers()

    igOpenGL3Shutdown()
    igGlfwShutdown()
    context.igDestroyContext()

    glfwTerminate()

    glDeleteVertexArrays(1, addr vao)
    glDeleteBuffers(1, addr vbo)
    glDeleteBuffers(1, addr ebo)

when isMainModule:
    main()
