cmake_minimum_required(VERSION 3.20)
project(render_demo)

find_package(SDL2 REQUIRED)
find_package(Halide REQUIRED)

add_executable(diffusion_generator reaction_diffusion_generator.cpp)
target_link_libraries(diffusion_generator PRIVATE Halide::Generator)

add_halide_library(reaction_diffusion_init FROM diffusion_generator
                   AUTOSCHEDULER Halide::Adams2019)
add_halide_library(reaction_diffusion_update FROM diffusion_generator
                   AUTOSCHEDULER Halide::Adams2019)
add_halide_library(reaction_diffusion_render FROM diffusion_generator
                   AUTOSCHEDULER Halide::Adams2019)

add_executable(main main.cpp)
target_link_libraries(
    main
    PRIVATE
    Halide::Halide
    SDL2::SDL2
    SDL2::SDL2main
    reaction_diffusion_init
    reaction_diffusion_update
    reaction_diffusion_render
)
target_compile_features(main PRIVATE cxx_std_11)
