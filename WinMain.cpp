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
int countedFrames = 0;
Uint32 frameStart;
Uint32 capTimer = 0;
const int SCREEN_FPS = 144;  // Aumentado a 144 FPS
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const char* windowTitle;

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

//ANIMACIÓN DE SPRITES DE IMAGENES DE BIENVENIDA
void showWelcomeScreen(SDL_Renderer* renderer, bool& showWelcome) {
    std::vector<std::string> imageKeys = {"hs1", "hs2", "hs3", "hs4", "hs5", "hs6", "hs7"};

    Uint32 frameInterval = 250;
    int frameIndex = 0;  

    bool quit = false;
    while (!quit) {
        ImageLoader::render(renderer, imageKeys[frameIndex], 0, 0, 800, 550);
        SDL_RenderPresent(renderer);
        SDL_Delay(frameInterval);
        frameIndex++;
        if (frameIndex >= imageKeys.size()) {
            frameIndex = 0;  
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                
                quit = true;
                showWelcome = false;
            }
        }
    }
}



void draw_ui(SDL_Renderer* renderer, int width, int height){
  int sizeP = 182;
  int sizeB = 102;
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

    ++countedFrames;
    int frameTicks = SDL_GetTicks() - frameStart;
    if (frameTicks < SCREEN_TICKS_PER_FRAME) {
      SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
    }

    windowTitle = ("DOOM - FPS: " + std::to_string(countedFrames)).c_str();

    if (SDL_GetTicks() - capTimer >= 1000) {
      capTimer = SDL_GetTicks();
      
      SDL_SetWindowTitle(window, windowTitle);
      countedFrames = 0;
    }

  SDL_Init(SDL_INIT_VIDEO);
  int imgFlags = IMG_INIT_PNG;
  window = SDL_CreateWindow(windowTitle, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
  ImageLoader::loadImage("e", "assets/emerald.png");
  ImageLoader::loadImage("s", "assets/redstone.png");
  ImageLoader::loadImage("o", "assets/obsidian.png");
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
  float newPlayerX = 0.0f;
  float newPlayerY = 0.0f;

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
        switch(event.key.keysym.sym ){
          case SDLK_LEFT:
            r.player.a += M_PI/500;
            break;
          case SDLK_RIGHT:
            r.player.a -= M_PI/500;
            break;
          case SDLK_w:
            newPlayerX = r.player.x;
            newPlayerY = r.player.y;
            r.incertidumbreX += M_PI/12;
            r.incertidumbreY += M_PI/8;
            newPlayerX += speed * cos(r.player.a);
            newPlayerY += speed * sin(r.player.a);

            if (!r.isWallCollision(newPlayerX, newPlayerY)) {
                    r.player.x = newPlayerX;
                    r.player.y = newPlayerY;
            }
            break;
          case SDLK_s:
            newPlayerX = r.player.x;
            newPlayerY = r.player.y;
            r.incertidumbreX -= M_PI/12;
            r.incertidumbreY += M_PI/8;
            newPlayerX -= speed * cos(r.player.a);
            newPlayerY -= speed * sin(r.player.a);

            if (!r.isWallCollision(newPlayerX, newPlayerY)) {
                    r.player.x = newPlayerX;
                    r.player.y = newPlayerY;
            }
            break;
            case SDLK_1: //Tecla para salir
                exit(1);
           default:
            break;
        }
      }

      if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        int deltaX = (mouseX > SCREEN_WIDTH/2)? 1 : -1;
        deltaX = (mouseX == SCREEN_WIDTH/2)? 0:deltaX;

        // Ajustar la velocidad de rotación según la posición del mouse
        double rotationSpeed = - M_PI / 30;
        r.player.a += rotationSpeed * deltaX;
        SDL_WarpMouseInWindow(NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT / 2);
        // Actualizar la posición anterior del mouse
      }
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


    

    SDL_RenderPresent(renderer);
  }

  musicThread.join();

  SDL_DestroyWindow(window);
  SDL_Quit();
}
