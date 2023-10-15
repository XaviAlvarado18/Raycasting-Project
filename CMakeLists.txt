cmake_minimum_required(VERSION 3.10)
project(MySDLProject)

# Establecer la versión de C++
set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    WinMain.cpp
)


# Ruta a los archivos de encabezado de SDL
include_directories(src)

# Añade el directorio de encabezados de SDL_image a las rutas de inclusión
include_directories(src/include/SDL2_image-2.6.3)

set(SDL2_IMAGE_LIBRARY C:/Users/Alvar/OneDrive/Documentos/SemestreVI/Graficas/RayCasting/SDL2_image.dll)

set(CMAKE_WIN32_EXECUTABLE TRUE)

# Agregar el archivo fuente principal
add_executable(MySDLProject ${SOURCES})


# Ruta a las bibliotecas de SDL
target_link_libraries(MySDLProject PRIVATE SDL2 ${SDL2_IMAGE_LIBRARY})