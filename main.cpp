#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H
#include FT_GLYPH_H

#include <cstddef>
#include <iostream>
#include <vector>
#include <stdexcept>

#include "Preprocess.hpp"

constexpr auto openSans = "../fonts/Open_Sans/OpenSans-Regular.ttf";
constexpr auto inter = "../fonts/Inter/static/Inter-Regular.ttf";

void error_callback(int error, const char* description) noexcept
{
    std::cerr << "Error: " << description << std::endl;
}

void Run()
{
    FT_Error error;
    FT_Library library;
    FT_Face face;

    error = FT_Init_FreeType(&library);

    if (error != FT_Err_Ok)
    {
        throw std::runtime_error{ "Could not initialize Freetype." };
    }

    error = FT_New_Face(
            library,
            inter,
            0,
            std::addressof(face));

    if (error == FT_Err_Unknown_File_Format)
    {
        throw std::runtime_error{ "Font format is unsupported." };
    }
    else if (error != FT_Err_Ok)
    {
        throw std::runtime_error{ "Font file could not be read." };
    }

    std::cout << "Loaded font face " << face->family_name << std::endl << std::flush;

    std::vector<vt::GlyphGrid> grids;
    std::vector<float> curvesBuffer;
    std::vector<std::uint16_t> gridsBuffer;
    vt::ProcessGlyphs(face, grids);
    vt::PackGlyphGrids(grids, 128, curvesBuffer, gridsBuffer);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        throw std::runtime_error{ "Could not initialize GLFW." };
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    auto window = glfwCreateWindow(640, 480, "VecType Demo", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{ "Could not create window." };
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / static_cast<float>(height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO
        // Set up OpenGL objects, etc.
        // Render some text to the screen
        // Show zooming functionality
        // Integrate with Nuklear UI

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void main()
{
    try
    {
        Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
