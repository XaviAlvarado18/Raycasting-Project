#include "SDL2/SDL.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring> // Agregamos esta línea para usar memcpy
#include <windows.h>
#include "Raycaster.h"
#include "color.h"
#include "include/SDL_image.h"


SDL_Window* window;
SDL_Renderer* renderer;

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

void draw_ui(){
  int sizeP = 224;
  int sizeB = 125;
  ImageLoader::render(renderer, "p", SCREEN_WIDTH - sizeP/1.0f, SCREEN_HEIGHT - sizeP, sizeP);
  ImageLoader::render(renderer, "b", SCREEN_WIDTH/2 - sizeB/2.0f, SCREEN_HEIGHT - sizeB, sizeB);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

  SDL_Init(SDL_INIT_VIDEO);
  int imgFlags = IMG_INIT_PNG;
  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/wall2.png");
  ImageLoader::loadImage("-", "assets/iron.png");
  ImageLoader::loadImage("|", "assets/wall2.png");
  ImageLoader::loadImage("p", "assets/ironpickaxe.png");
  ImageLoader::loadImage("b", "assets/background.png");


  Raycaster r = { renderer };
  r.load_map("assets/map2.txt");

  int speed = 10;
  bool running = true;
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
            r.player.a += 3.14/24;
            break;
          case SDLK_RIGHT:
            r.player.a -= 3.14/24;
            break;
          case SDLK_UP:
            r.player.x += speed * cos(r.player.a);
            r.player.y += speed * sin(r.player.a);
            break;
          case SDLK_DOWN:
            r.player.x -= speed * cos(r.player.a);
            r.player.y -= speed * sin(r.player.a);
            break;
           default:
            break;
        }
      }
    }

    clear();
    draw_floor();

    r.render();

    draw_ui();

    // render

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
