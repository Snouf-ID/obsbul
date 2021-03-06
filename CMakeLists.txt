project(ObscureBulldozer)
cmake_minimum_required(VERSION 2.6)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(OpenGL REQUIRED)
find_package(PhysFS REQUIRED)
find_package(FreeImagePlus REQUIRED)
find_package(GLEW REQUIRED)
find_package(SFML 2 REQUIRED COMPONENTS window system)
find_package(TinyXML REQUIRED)
find_package(GLM REQUIRED)
find_package(UnitTest++)

set(SCRIPTS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)
set(Boost_LIBRARIES ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

add_subdirectory(src)

add_subdirectory(assets)
