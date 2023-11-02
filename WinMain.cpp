#include "SDL2/SDL.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring> // Agregamos esta línea para usar memcpy
#include <windows.h>
#include "Raycaster.h"
#include "color.h"
#include "include/SDL_image.h"
#include <thread>

// Variables globales
SDL_Window* window;
bool isMusicPlaying = true; 
bool showWelcome = true;
bool winnable = false;

void clear() {
  SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
  SDL_RenderClear(renderer);
}

void draw_floor() {
  // floor color
  SDL_SetRenderDrawColor(renderer, 112, 122, 122, 255);
  SDL_Rect rect = {
    0, 
    SCREEN_HEIGHT / 2,
    SCREEN_WIDTH,
    SCREEN_HEIGHT / 2
  };
  SDL_RenderFillRect(renderer, &rect);
}


void showWelcomeScreen(SDL_Renderer* renderer, bool& showWelcome) {
    std::vector<std::string> imageKeys = {"hs1", "hs2", "hs3", "hs4", "hs5", "hs6", "hs7"};

    Uint32 frameInterval = 250;
    int frameIndex = 0;  // Índice del cuadro actual

    bool quit = false;
    while (!quit) {
        // Dibuja el cuadro actual en la ventana
        ImageLoader::render(renderer, imageKeys[frameIndex], 0, 0, 1050, 550);
        SDL_RenderPresent(renderer);

        // Espera el tiempo de intervalo entre cuadros
        SDL_Delay(frameInterval);

        // Avanza al siguiente cuadro
        frameIndex++;
        if (frameIndex >= imageKeys.size()) {
            frameIndex = 0;  // Reiniciar la animación al llegar al final
        }

        // Maneja eventos, como la salida de la ventana y eventos de teclado
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Salir si el usuario cierra la ventana
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                // Al presionar cualquier tecla, ocultar la pantalla de bienvenida
                quit = true;
                showWelcome = false;
            }
        }
    }
}



void draw_ui(SDL_Renderer* renderer, int width, int height){
  int sizeP = 192;
  int sizeB = 112;
  ImageLoader::render(renderer, "p", SCREEN_WIDTH - sizeP/1.0f, SCREEN_HEIGHT - sizeP, sizeP,sizeP);
  ImageLoader::render(renderer, "b", SCREEN_WIDTH/2 - sizeB/2.0f, SCREEN_HEIGHT - sizeB, 170, sizeB);
}

// Función que maneja la reproducción de música en un hilo separado
int PlayMusicThread() {
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV("assets/moogcity2.wav", &wavSpec, &wavBuffer, &wavLength) == nullptr) {
        // Manejar el error en caso de que la carga de música falle.
        return 1;
    }


    if (SDL_OpenAudio(&wavSpec, nullptr) < 0) {
        // Manejar el error en caso de que la apertura de audio falle.
        return 1;
    }

    while (isMusicPlaying) {
        SDL_QueueAudio(1, wavBuffer, wavLength);
        SDL_PauseAudio(0);

        // Espera a que termine la reproducción actual
        SDL_Delay(wavLength * 1000 / wavSpec.freq);

        // Vuelve a cargar la música para reproducirla en bucle
        SDL_QueueAudio(1, wavBuffer, wavLength);
        SDL_PauseAudio(0);
    }

    // Libera los recursos al final de la reproducción
    SDL_CloseAudio();
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

  SDL_Init(SDL_INIT_VIDEO);
  int imgFlags = IMG_INIT_PNG;
  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/wall2.png");
  ImageLoader::loadImage("-", "assets/iron.png");
  ImageLoader::loadImage("|", "assets/wall2.png");
  ImageLoader::loadImage("bk", "assets/wall2.png");
  ImageLoader::loadImage("p", "assets/ironpickaxe.png");
  ImageLoader::loadImage("b", "assets/background.png");
  ImageLoader::loadImage("d", "assets/diamond.png");
  ImageLoader::loadImage("g", "assets/gold.png");
  ImageLoader::loadImage("r", "assets/coal.png");
  ImageLoader::loadImage("c", "assets/cartel.png");
  ImageLoader::loadImage("w", "assets/cartel2.png");
  ImageLoader::loadImage("e1","assets/creeperMini.png");
  ImageLoader::loadImage("e2","assets/zombie.png");
  ImageLoader::loadImage("hs1","assets/hs1.png");
  ImageLoader::loadImage("hs2","assets/hs2.png");
  ImageLoader::loadImage("hs3","assets/hs3.png");
  ImageLoader::loadImage("hs4","assets/hs4.png");
  ImageLoader::loadImage("hs5","assets/hs5.png");
  ImageLoader::loadImage("hs6","assets/hs6.png");
  ImageLoader::loadImage("hs7","assets/hs7.png");
  ImageLoader::loadImage("w1","assets/winnable.png");

  Raycaster r = { renderer };
  r.load_map("assets/map2.txt");
  int prevMouseX = 0;
  int speed = 10;
  int widthP = 80;
  int heightP = 80;
  bool isMovingLeft = false;
  bool isMovingRight = false;
  bool keys[SDL_NUM_SCANCODES] = {false};
  bool running = true;

  showWelcomeScreen(renderer, showWelcome);

  std::thread musicThread(PlayMusicThread);
  while(running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
                keys[event.key.keysym.scancode] = true;
            }
            if (event.type == SDL_KEYUP) {
                keys[event.key.keysym.scancode] = false;
            }

            // Captura los movimientos del mouse
            if (event.type == SDL_MOUSEMOTION) {
                int mouseX = event.motion.x;
                //print(event.motion.x);
                int mouseXDelta = mouseX - prevMouseX;
                prevMouseX = mouseX;
                // Ajusta la dirección de vista del jugador en función del movimiento del mouse
                r.player.a -= static_cast<float>(mouseXDelta) * 0.02f; // Puedes ajustar el factor de sensibilidad
                r.player.mapA -= static_cast<float>(mouseXDelta) * 0.02f;
          }
    }


    if (keys[SDL_SCANCODE_LEFT]) {
            r.player.a += 3.14/24;
            r.player.mapA += (M_PI/24);
            widthP = 120;
            heightP = 80;
            isMovingLeft = true;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            r.player.a -= 3.14/24;
            r.player.mapA -= (M_PI/24);
            widthP = 120;
            heightP = 80;
            isMovingRight = true;
        } else {
            // Cuando no se está moviendo a la izquierda o derecha, vuelve a la imagen predeterminada.
            if (isMovingLeft) {
                widthP = 80;
                heightP = 80;
                isMovingLeft = false;
            } else if (isMovingRight) {
                widthP = 80;
                heightP = 80;
                isMovingRight = false;
            }
        }
        if (keys[SDL_SCANCODE_UP]) {
            r.player.x += static_cast<int>(speed * cos(r.player.a));
            r.player.y += static_cast<int>(speed * sin(r.player.a));
            r.player.mapx = static_cast<int>(r.player.x / 3);
            r.player.mapy = static_cast<int>(r.player.y / 3);
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            r.player.x -= static_cast<int>(speed * cos(r.player.a));
            r.player.y -= static_cast<int>(speed * sin(r.player.a));
            r.player.mapx = static_cast<int>(r.player.x / 3);
            r.player.mapy = static_cast<int>(r.player.y / 3);
      }

    if (winnable) {
        r.draw_victory_screen();
    }

    if (r.has_won()) {
        winnable = true;
    }


    clear();
    draw_floor();

    r.render();

    draw_ui(renderer, widthP, heightP);

    // render

    SDL_RenderPresent(renderer);
  }

  musicThread.join();

  SDL_DestroyWindow(window);
  SDL_Quit();
}
