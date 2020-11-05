
# API:
#   type Image
#   draw(Image)
#

import nimgl/opengl
import stb_image/read as stbi
import std/[tables, sequtils]
import glm
import ./wnd

proc unsafeCAddr*[M, N, T](m: Mat[M, N, T]): ptr T = m.arr[0].arr[0].unsafeAddr

converter toGLint*(val: GLenum): GLint = int32(uint32(val))

proc toRGB*(vec: Vec3[float32]): Vec3[float32] =
    return vec3(vec.x / 255, vec.y / 255, vec.z / 255)

type GLError* = object of CatchableError

#template raiseGLError(msg: untyped) = raise newException(GLError, msg)
#                 ██ ███    ███  █████   ██████  ███████ 
#                 ██ ████  ████ ██   ██ ██       ██      
#                 ██ ██ ████ ██ ███████ ██   ███ █████ 
#                 ██ ██  ██  ██ ██   ██ ██    ██ ██    
#                 ██ ██      ██ ██   ██  ██████  ███████ 

type Image* = object
    handle*: GLuint
    width*: int
    height*: int
    kind*: GLenum

proc `=destroy`*(img: var Image) =
    glDeleteTextures(1, addr img.handle)

type ImageOptions* = object
    wrap_s*: GLenum
    wrap_t*: GLenum
    filter_min*: GLenum
    filter_mag*: GLenum

proc newImage*(src: string, kind: GLenum, options: ImageOptions): Image =
    var width, height, channels: int
    stbi.setFlipVerticallyOnLoad(true)
    var data = stbi.load(src, width, height, channels, stbi.Default)

    assert channels == 4, "Only RGBA is supported"

    var texture: uint32
    glGenTextures(1, addr texture)
    glBindTexture(kind, texture)
    glTexParameteri(kind, GL_TEXTURE_WRAP_S, options.wrap_s)
    glTexParameteri(kind, GL_TEXTURE_WRAP_T, options.wrap_t)
    glTexParameteri(kind, GL_TEXTURE_MIN_FILTER, options.filter_min)
    glTexParameteri(kind, GL_TEXTURE_MAG_FILTER, options.filter_mag)
    glTexImage2D(kind, 0, GL_RGBA, int32(width), int32(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, addr data[0])
    glGenerateMipmap(kind)

    return Image(
        handle: texture,
        width: width,
        height: height,
        kind: kind
    )

proc newImage*(src: string, kind: GLenum): Image =
    newImage(src, kind, ImageOptions(
        wrap_s: GL_REPEAT,
        wrap_t: GL_REPEAT,
        filter_min: GL_NEAREST,
        filter_mag: GL_NEAREST
    ))

proc attach*(img: Image, slot: GLenum) =
    glActiveTexture(slot)
    glBindTexture(img.kind, img.handle)
# proc detach()

#                ███████ ██   ██  █████  ██████  ███████ ██████ 
#                ██      ██   ██ ██   ██ ██   ██ ██      ██   ██ 
#                ███████ ███████ ███████ ██   ██ █████   ██████  
#                     ██ ██   ██ ██   ██ ██   ██ ██      ██   ██ 
#                ███████ ██   ██ ██   ██ ██████  ███████ ██   ██ 

type
    Shader* = object
        handle*: GLuint
        uniforms*: Table[string, Uniform]

    Uniform* = object
        name*: string
        `type`*: GLenum
        location*: int32
        size*: int32
        length*: int32

proc `=destroy`*(shader: var Shader) =
    glDeleteProgram(shader.handle)

proc checkShader(shader: uint32) =
    var status: int32
    glGetShaderiv(shader, GL_COMPILE_STATUS, addr status);
    if status != GL_TRUE.ord:
        var
            log_length: int32
            message = newSeq[char](1024)
        glGetShaderInfoLog(shader, 1024, addr log_length, addr message[0]);
        raise newException(GLError, cast[string](message))

proc checkProgram(program: uint32) =
    var status: int32
    glGetProgramiv(program, GL_LINK_STATUS, addr status);
    if status != GL_TRUE.ord:
        var
            log_length: int32
            message = newSeq[char](1024)
        glGetProgramInfoLog(program, 1024, addr log_length, addr message[0]);
        raise newException(GLError, cast[string](message))

proc queryUniforms(program: uint32): Table[string, Uniform] =
    glUseProgram(program)
    # TODO(?): don't unbind
    defer: glUseProgram(0)

    var count: GLint
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, addr count)
    for i in 0..<count:
        var `type`: GLenum
        var length, size: int32
        var buffer = newString(32) # max uniform name length is 32, hopefully enough
        glGetActiveUniform(program, (GLuint)i, 32, addr length, addr size, addr `type`, buffer)
        if length == 0: continue # in this case we got empty name, which is invalid

        var name: string = buffer[0..<length]
        var location = glGetUniformLocation(program, name)
        result[name] = Uniform(name: name, `type`: `type`, location: location, size: size, length: length)

proc newShader*(vsrc: cstring, fsrc: cstring): Shader =
    var program: uint32
    block:
        var vertex = glCreateShader(GL_VERTEX_SHADER)
        var fragment = glCreateShader(GL_FRAGMENT_SHADER)
        # compile vertex & fragment stages
        glShaderSource(vertex, 1'i32, unsafeAddr vsrc, nil)
        glShaderSource(fragment, 1, unsafeAddr fsrc, nil)
        glCompileShader(vertex)
        glCompileShader(fragment)
        checkShader(vertex)
        checkShader(fragment)
        # link program
        program = glCreateProgram()
        glAttachShader(program, vertex)
        glAttachShader(program, fragment)
        glLinkProgram(program)
        checkProgram(program)
        # discard intermediates
        glDeleteShader(vertex)
        glDeleteShader(fragment)

    return Shader(
        handle: program,
        uniforms: queryUniforms(program)
    )

proc attach*(obj: Shader) = glUseProgram(obj.handle)
# proc detach()

#                ███    ███ ███████ ███████ ██   ██ 
#                ████  ████ ██      ██      ██   ██ 
#                ██ ████ ██ █████   ███████ ███████ 
#                ██  ██  ██ ██           ██ ██   ██ 
#                ██      ██ ███████ ███████ ██   ██ 

type
    Mesh* = object
        vbo*, ebo*, vao*: uint32
        icount: int
    Attribute* = object
        loc*: uint32
        size*: int32
        `type`*: GLenum

proc `=destroy=`*(mesh: var Mesh) =
    glDeleteBuffers(1, addr mesh.vbo)
    glDeleteBuffers(1, addr mesh.ebo)
    glDeleteVertexArrays(1, addr mesh.vao)

proc sizeofGLType(`type`: GLenum): int =
    case `type`
    of EGL_BYTE: sizeof(byte)
    of EGL_FLOAT: sizeof(cfloat)
    of EGL_DOUBLE: sizeof(cdouble)
    of EGL_SHORT: sizeof(cshort)
    of EGL_INT: sizeof(cint)
    else: raise newException(GLError, "Invalid GL type: " & $uint32(`type`))

proc newMesh*(vert: seq[float32], ind: seq[uint32], attribs: seq[Attribute]): Mesh =
    var vbo, ebo, vao: uint32
    glGenBuffers(1, addr vbo)
    glGenBuffers(1, addr ebo)
    glGenVertexArrays(1, addr vao)

    # vertex array
    glBindVertexArray(vao)
    # TODO(?): don't unbind
    defer: glBindVertexArray(0)

    # index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cint(cuint.sizeof * ind.len), unsafeAddr ind[0], GL_STATIC_DRAW)

    # vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, cint(cfloat.sizeof * vert.len), unsafeAddr vert[0], GL_STATIC_DRAW)

    # vertex buffer descriptor
    let stride: int32 = foldl(attribs, a + (int32(sizeofGLType(b.`type`)) * b.size), 0'i32)
    var offset: int = 0
    for attrib in attribs:
        glVertexAttribPointer(attrib.loc, attrib.size, attrib.`type`, false, stride, cast[pointer](offset))
        glEnableVertexAttribArray(attrib.loc)
        offset += sizeofGLType(attrib.`type`) * attrib.size
        #glVertexAttribPointer(1'u32, 2, EGL_FLOAT, false, (cfloat.sizeof * 2) * 2, cast[pointer](cfloat.sizeof * 2))
        #glEnableVertexAttribArray(1)

    return Mesh(
        vbo: vbo, ebo: ebo, vao: vao,
        icount: ind.len
    )


proc attach*(mesh: Mesh) =
    glBindVertexArray(mesh.vao)

proc render*(mesh: Mesh, mode: GLenum) =
    glDrawElements(mode, cint(mesh.icount), GL_UNSIGNED_INT, nil)

#                ██████  ███████ ███    ██ ██████  ███████ ██████  ███████ ██████ 
#                ██   ██ ██      ████   ██ ██   ██ ██      ██   ██ ██      ██   ██ 
#                ██████  █████   ██ ██  ██ ██   ██ █████   ██████  █████   ██████  
#                ██   ██ ██      ██  ██ ██ ██   ██ ██      ██   ██ ██      ██   ██ 
#                ██   ██ ███████ ██   ████ ██████  ███████ ██   ██ ███████ ██   ██ 


# TODO: draw buckets - sorting or fixed buckets
# TODO: sort solid front -> back
# TODO: sort transparent back -> front
type
    Command* = object
        image: Image
        #[ uvMin: Vec2f
        uvMax: Vec2f ]#
        model: Mat4f

    Renderer* = object
        # textured quad shader
        shader*: Shader
        # quad mesh
        mesh*: Mesh
        # draw command buffer
        cmds*: seq[Command]

        # uniforms
        uProj*: Uniform
        uView*: Uniform
        uModel*: Uniform
        uTexture*: Uniform

const SHADER_QUAD_TEXTURED_VSRC = """#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = uProj * uView * uModel * vec4(aPos, 0.0, 1.0);
}
"""
const SHADER_QUAD_TEXTURED_FSRC = """#version 330 core
out vec4 FragColor;
in vec2 vUV;
uniform sampler2D uTexture;
void main() {
    FragColor = texture(uTexture, vUV);
}
"""
const QUAD_VERTICES = @[
    +1.0f, +1.0f, 1.0f, 1.0f,
    +1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, +1.0f, 0.0f, 1.0f
]
const QUAD_INDICES = @[
    0'u32, 1'u32, 3'u32,
    1'u32, 2'u32, 3'u32
]

proc newRenderer*(): Renderer =
    var shader = newShader(SHADER_QUAD_TEXTURED_VSRC, SHADER_QUAD_TEXTURED_FSRC)
    var mesh = newMesh(QUAD_VERTICES, QUAD_INDICES, @[
        Attribute(loc: 0, size: 2, `type`: EGL_FLOAT),
        Attribute(loc: 1, size: 2, `type`: EGL_FLOAT)
    ])

    var
        uProj = shader.uniforms["uProj"]
        uView = shader.uniforms["uView"]
        uModel = shader.uniforms["uModel"]
        uTexture = shader.uniforms["uTexture"]

    Renderer(
        shader: shader, mesh: mesh, cmds: @[],
        uProj: uProj,
        uView: uView,
        uModel: uModel,
        uTexture: uTexture,
    )

proc begin*(renderer: var Renderer, window: var Window) =
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    let bg = vec3(33f, 33f, 33f).toRGB()
    glClearColor(bg.r, bg.g, bg.b, 1f)

    let hw = window.width.float32 / 2
    let hh = window.height.float32 / 2
    var
        p = ortho(hw, -hw, -hh, hh, -1f, 1f)
        v = lookAt(vec3(0f, 0f, 0f), vec3(0f, 0f, 1f), vec3(0f, 1f, 0f))

    renderer.shader.attach()
    glUniformMatrix4fv(renderer.uProj.location, 1, false, p.caddr)
    glUniformMatrix4fv(renderer.uView.location, 1, false, v.caddr)
    renderer.mesh.attach()

proc draw*(
    renderer: var Renderer,
    image: Image,
    #[ uv: Vec4f, ]#
    model: Mat4f) =
    renderer.cmds.add(Command(image: image, model: model))

proc flush*(renderer: var Renderer) =
    var lastTexture: GLuint = high(uint32)
    for cmd in renderer.cmds:
        if cmd.image.handle != lastTexture:
            lastTexture = cmd.image.handle
            cmd.image.attach(GL_TEXTURE0)
            glUniform1i(renderer.uTexture.location, 0)
        glUniformMatrix4fv(renderer.uModel.location, 1, false, cmd.model.unsafeCAddr)
        renderer.mesh.render(GL_TRIANGLES)
    renderer.cmds.setLen(0)

#                ██    ██ ██████ 
#                ██    ██   ██ 
#                ██    ██   ██ 
#                ██    ██   ██ 
#                 ██████  ██████

import nimgl/imgui, nimgl/imgui/[impl_opengl, impl_glfw]

import ./ui

proc iv2(x: float32, y: float32): ImVec2 = ImVec2(x: x, y: y)

proc initUI*(window: Window) =
    igCreateContext()
    assert igGlfwInitForOpengl(window.handle, true)
    assert igOpenGL3Init()
    igStyleColorsDark()

proc deinitUI*() =
    igOpenGL3Shutdown()
    igGlfwShutdown()
    igDestroyContext(igGetCurrentContext())

proc render*(ui: var UIState) =
    ## Render a specific UI state.
    ## Call after rendering the main scene.

    igOpenGL3NewFrame()
    igGlfwNewFrame()
    igNewFrame()

    if ui.show_demo: igShowDemoWindow(addr ui.show_demo)

    let io = igGetIO()

    igBegin("Hello, world!", nil, ImGuiWindowFlags.NoTitleBar)
    igCheckbox("Demo Window", addr ui.show_demo)
    igSliderFloat("float", addr ui.somefloat, 0.0f, 1.0f)
    if igButton("Button", iv2(0, 0)): inc(ui.counter)
    igSameLine()
    igText("counter = %d", ui.counter)
    igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.framerate, io.framerate)

    igEnd()
    igRender()
    igOpenGL3RenderDrawData(igGetDrawData())
