# Copyright 2018, NimGL contributors.

import nimgl/[glfw, opengl]
import glm

import ./wnd, ./gl, ./ui

# TODO camera
# TODO tile map rendering - try naïve approach first (each tile separate uniform/draw calls)
# TODO sprite rendering - naïve approach is fine

proc keyProc(window: GLFWWindow, key: int32, scancode: int32, action: int32, mods: int32): void {.cdecl.} =
    if key == GLFWKey.Escape and action == GLFWPress:
        window.setWindowShouldClose(true)
    if key == GLFWKey.Space:
        glPolygonMode(GL_FRONT_AND_BACK, if action != GLFWRelease: GL_LINE else: GL_FILL)

proc main =
    var window = newWindow("Hello, World!", keyProc)

    initUI(window)
    defer: deinitUI()

    var
        uiState = newUIState()
        renderer = newRenderer()

    var image: Image = newImage("TILESET.png", GL_TEXTURE_2D)
    var model = mat4f().scale(image.width.float32 / 2, image.height.float32 / 2, 1f)

    while not window.shouldClose():
        glfwPollEvents()

        glClear(GL_COLOR_BUFFER_BIT)
        renderer.begin(window)
        renderer.draw(image, model)
        renderer.flush()
        render(uiState)

        window.swapBuffers()

when isMainModule:
    main()
