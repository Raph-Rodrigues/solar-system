cmake_minimum_required(VERSION 3.16)
project("Solar System" CXX)

# Exige o padrão C++20 ou superior
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Encontra a instalação oficial do SDL3 no Arch Linux
find_package(SDL3 REQUIRED)

# Configura o executável do projeto
add_executable(solar-system main.cpp)

# Vincula as definições e binários do SDL3 ao projeto
target_link_libraries(solar-system PRIVATE SDL3::SDL3)

