-- add a debug and release build
add_rules("mode.debug", "mode.release")

-- bring in dependencies from xmake's own package repository
add_requires("glfw >=3.3.2")
-- or from conan (among others)
add_requires("CONAN::glm/0.9.9.8", { alias = "glm" })

-- create a target
target("tedit")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("src")
    add_includedirs("vendor")

    -- include files from any source
    -- glad
    add_files("vendor/glad/glad/glad.c")
    add_includedirs("vendor/glad")

    -- imgui using OpenGL3/GLFW backend
    add_files("vendor/imgui/*.cpp")
    add_files("vendor/imgui/backends/imgui_impl_opengl3.cpp")
    add_files("vendor/imgui/backends/imgui_impl_glfw.cpp")
    add_includedirs("vendor/imgui")

    -- and link the dependencies we brought in
    add_packages("glfw")
    add_packages("glm")