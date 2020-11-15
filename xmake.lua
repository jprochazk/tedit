-- add a debug and release build
add_rules("mode.debug", "mode.release")

-- bring in dependencies from xmake's own package repository
add_requires("glfw >=3.3.2")
-- or from conan (among others)
add_requires("CONAN::glm/0.9.9.8", { alias = "glm" })
add_requires("CONAN::stb/20200203", { alias = "stb" })
add_requires("CONAN::spdlog/1.8.0", { alias = "spdlog" }) --, configs = {options = "spdlog:header_only=True"} }) 
add_requires("CONAN::nlohmann_json/3.9.1", { alias = "json" })
add_requires("CONAN::concurrentqueue/1.0.2", { alias = "cqueue" });

-- create a target
target("tedit")
    set_kind("binary")
    set_languages("cxx17")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_includedirs("vendor")

    set_pcxxheader("src/pch.h")

    -- include files from any source
    -- glad
    add_files("vendor/glad/glad/glad.c")
    add_includedirs("vendor/glad")

    -- imgui using OpenGL3/GLFW backend
    add_files("vendor/imgui/*.cpp")
    add_includedirs("vendor/imgui")

    -- portabile-file-dialogs
    add_includedirs("vendor/pfd");

    -- and link the dependencies we brought in
    add_packages("glfw")
    add_packages("glm")
    add_packages("stb")
    add_packages("spdlog")
    add_packages("json")
    add_packages("cqueue");