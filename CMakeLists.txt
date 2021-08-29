cmake_minimum_required(VERSION 3.12)

include(ExternalProject)

project(VecType LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(EP_UPDATE_DISCONNECTED OFF CACHE BOOL "")
set_property(DIRECTORY PROPERTY EP_PREFIX libs)
set(DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED ${EP_UPDATE_DISCONNECTED})

# glad
ExternalProject_Add(glad-loader
    GIT_REPOSITORY git@github.com:Dav1dde/glad.git
    GIT_TAG tags/v0.1.33
    GIT_SHALLOW ON
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(glad-loader BINARY_DIR)
set(GLAD_BINARY_DIR ${BINARY_DIR})

# glfw
ExternalProject_Add(glfw
    GIT_REPOSITORY git@github.com:glfw/glfw.git
    GIT_TAG tags/3.3
    GIT_SHALLOW ON
    CMAKE_CACHE_ARGS
    -DGLFW_BUILD_DOCS:BOOL=OFF
    -DGLFW_BUILD_TESTS:BOOL=OFF
    -DGLFW_BUILD_EXAMPLES:BOOL=OFF
    -DGLFW_INSTALL:BOOL=OFF
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(glfw SOURCE_DIR)
ExternalProject_Get_Property(glfw BINARY_DIR)
set(GLFW_SOURCE_DIR ${SOURCE_DIR})
set(GLFW_BINARY_DIR ${BINARY_DIR})

# glm
ExternalProject_Add(glm
    GIT_REPOSITORY git@github.com:g-truc/glm.git
    GIT_TAG tags/0.9.9.5
    GIT_SHALLOW ON
    CMAKE_CACHE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -DBUILD_STATIC_LIBS:BOOL=OFF
    -DGLM_TEST_ENABLE:BOOL=OFF
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(glm SOURCE_DIR)
#ExternalProject_Get_Property(glm BINARY_DIR)
set(GLM_SOURCE_DIR ${SOURCE_DIR})
#set(GLM_BINARY_DIR ${BINARY_DIR})

# freetype windows binaries
if (NOT MINGW AND WIN32)
    ExternalProject_Add(freetype-bin
        GIT_REPOSITORY git@github.com:ubawurinna/freetype-windows-binaries.git
        GIT_TAG tags/v2.10.4
        GIT_SHALLOW ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )
    ExternalProject_Get_Property(freetype-bin SOURCE_DIR)
    set(ENV{FREETYPE_DIR} ${SOURCE_DIR})
    set(FREETYPE_INCLUDE_DIRS ${SOURCE_DIR}/include)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FREETYPE_LINK_DIR ${SOURCE_DIR}/win64)
    else()
        set(FREETYPE_LINK_DIR ${SOURCE_DIR}/win32)
    endif()
    link_directories(${FREETYPE_LINK_DIR})
    set(FREETYPE_LIBRARIES freetype)
else()
    find_package(Freetype REQUIRED)
endif()

# freetype-gl
ExternalProject_Add(freetype-gl-engine
    GIT_REPOSITORY git@github.com:rougier/freetype-gl.git
    GIT_TAG ffe1c71
    GIT_SHALLOW OFF
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FREETYPE_LINK_DIR}/freetype.lib"
    "<SOURCE_DIR>/windows/freetype"
    CMAKE_CACHE_ARGS
    -Dfreetype-gl_WITH_GLEW:BOOL=ON
    -Dfreetype-gl_WITH_GLAD:BOOL=OFF
    -Dfreetype-gl_BUILD_DEMOS:BOOL=OFF
    -Dfreetype-gl_BUILD_APIDOC:BOOL=OFF
    -Dfreetype-gl_BUILD_MAKEFONT:BOOL=ON
    -Dfreetype-gl_BUILD_TESTS:BOOL=OFF
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(freetype-gl-engine SOURCE_DIR)
ExternalProject_Get_Property(freetype-gl-engine BINARY_DIR)
set(FTGL_SOURCE_DIR ${SOURCE_DIR})
set(FTGL_BINARY_DIR ${BINARY_DIR})

add_dependencies(freetype-gl-engine glad-loader freetype-bin)

# nuklear
ExternalProject_Add(nuklear
    GIT_REPOSITORY git@github.com:vurtun/nuklear.git
    GIT_TAG 6b9f937
    GIT_SHALLOW OFF
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(nuklear SOURCE_DIR)
set(NK_SOURCE_DIR ${SOURCE_DIR})

# Catch2
#ExternalProject_Add(catch2
#    GIT_REPOSITORY git@github.com:catchorg/Catch2.git
#    GIT_TAG tags/v2.10.2
#    GIT_SHALLOW ON
#    INSTALL_COMMAND ""
#    )
#ExternalProject_Get_Property(catch2 SOURCE_DIR)
#set(CATCH_SOURCE_DIR ${SOURCE_DIR})

link_directories(
    ${GLAD_BINARY_DIR}
    ${GLFW_BINARY_DIR}/src
    ${FTGL_BINARY_DIR}
    )

#enable_testing()
#add_executable(test)
#add_test(NAME test COMMAND test)
#set_target_properties(test PROPERTIES LINKER_LANGUAGE CXX)
#target_include_directories(test)
#target_link_libraries(test)

add_executable(${PROJECT_NAME}_demo
    Geometry.hpp
    Preprocess.hpp

    Geometry.cpp
    Preprocess.cpp
    main.cpp
    )

add_dependencies(${PROJECT_NAME}_demo
    glad-loader
    glfw
    glm
    freetype-gl-engine
    nuklear
    )

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_demo)

target_include_directories(${PROJECT_NAME}_demo PUBLIC
    ${GLAD_BINARY_DIR}/include
    ${GLFW_SOURCE_DIR}/include
    ${GLM_SOURCE_DIR}
    ${NK_SOURCE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${FTGL_SOURCE_DIR}
    )

target_link_libraries(${PROJECT_NAME}_demo
    glad
    glfw3
    ${FREETYPE_LIBRARIES}
    freetype-gl
    )

if (NOT MINGW AND WIN32)
    add_dependencies(${PROJECT_NAME}_demo freetype-bin)
    add_custom_command(TARGET ${PROJECT_NAME}_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FREETYPE_LINK_DIR}/freetype.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}_demo>
        )
endif()
