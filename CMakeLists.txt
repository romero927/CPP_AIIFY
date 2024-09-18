cmake_minimum_required(VERSION 3.10)

project(AIIFY)

# Set C++17 for MSVC
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add all your source files here
add_executable(${PROJECT_NAME} 
    main.cpp
    GitignoreParser.cpp
    FileProcessor.cpp
)

# If you have header files in a separate directory, add:
# target_include_directories(${PROJECT_NAME} PRIVATE include)

# ... rest of your CMakeLists.txt content ...