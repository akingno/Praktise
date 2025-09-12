cmake_minimum_required(VERSION 3.27)
project(world_temp)

set(CMAKE_CXX_STANDARD 17)

add_executable(world_temp main.cpp
        World.h
        Chunk.h
        Block.h
        Camera.h
        Character.h
        Character.cpp
        Chunk.cpp
        Block.cpp
        Noise.h)
