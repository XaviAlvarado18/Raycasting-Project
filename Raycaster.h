#pragma once

#include <iostream>
#include <fstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"
#include <omp.h>

const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;
SDL_Renderer* renderer;


struct Player {
  int x;
  int y;
  float a;
  int mapx;
  int mapy;
  float mapA;
  float fov;
}; 

std::unordered_map<std::string, Color> colors = {
  { "-", { 240, 200, 0 } },
  { "|", { 220, 36, 33 } },
  { "+", { 64, 169, 68 } }
};

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
};


struct Enemy {
  int x;
  int y;
  std::string texture;
};

void showGameOverScreen() {
    // Código para mostrar la pantalla de "Game Over"
    // Puedes usar SDL para dibujar una imagen de Game Over o un texto en la pantalla, y pausar el juego.
    // Aquí deberías incluir las funciones necesarias para mostrar la pantalla de Game Over de manera adecuada.
    ImageLoader::loadImage("go","assets/gameover.png");
    ImageLoader::render(renderer, "go", 0, 0, 1050, 550);
}


std::vector<Enemy> enemies;

class Raycaster {
public:
  Raycaster(SDL_Renderer* renderer)
    : renderer(renderer) {

    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI/2.0f;

    scale = 50;
    tsize = 128;
    textSize = 128;
    enemies =  {Enemy{2*BLOCK, 5*BLOCK , "e1"}, Enemy{BLOCK + 30, BLOCK - 28 ,"e2"}};
    incertidumbreY = 0.0f;
    incertidumbreX = 0.0f;
  }

  void load_map(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
      map.push_back(line);
    }
    file.close();
  }

  void points(const std::vector<std::pair<int, int>>& pointList, const std::vector<Color>& colors) {
        int count = static_cast<int>(pointList.size());
        std::vector<SDL_Point> sdlPoints(count);

        #pragma omp parallel for
        for (int i = 0; i < count; i++) {
            sdlPoints[i] = {pointList[i].first, pointList[i].second};
        }

        std::vector<Uint8> r(count);
        std::vector<Uint8> g(count);
        std::vector<Uint8> b(count);
        std::vector<Uint8> a(count);


        for (int i = 0; i < count; i++) {
            r[i] = colors[i].r;
            g[i] = colors[i].g;
            b[i] = colors[i].b;
            a[i] = colors[i].a;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Color de fondo (puedes cambiarlo)

        int startIndex = 0;
        int endIndex = 1;

        while (endIndex < count) {
            while (endIndex < count && r[endIndex] == r[startIndex] && g[endIndex] == g[startIndex] && b[endIndex] == b[startIndex] && a[endIndex] == a[startIndex]) {
                endIndex++;
            }

            // Establece el color una vez y traza todos los puntos del mismo color juntos
            SDL_SetRenderDrawColor(renderer, r[startIndex], g[startIndex], b[startIndex], a[startIndex]);
            SDL_RenderDrawPoints(renderer, sdlPoints.data() + startIndex, endIndex - startIndex);

            startIndex = endIndex;
            endIndex++;
        }

        // Trazar el último grupo de puntos
        SDL_SetRenderDrawColor(renderer, r[startIndex], g[startIndex], b[startIndex], a[startIndex]);
        SDL_RenderDrawPoints(renderer, sdlPoints.data() + startIndex, count - startIndex);
    }

  void rect(int x, int y, const std::string& mapHit) {
    for(int cx = x; cx < x + (BLOCK/5); cx++) {
      for(int cy = y; cy < y + (BLOCK/5); cy++) {
        int tx = ((cx - x) * tsize) / BLOCK;
        int ty = ((cy - y) * tsize) / BLOCK;

        //Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        //SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  void draw_enemy(Enemy enemy){
    float enemy_a = atan2(enemy.y - player.y, enemy.x - player.x);
    float enemy_d = sqrt(pow(player.x - enemy.x, 2) + pow(player.y - enemy.y, 2));
    int enemy_size = ((SCREEN_HEIGHT/enemy_d) * scale)/2;

    int enemy_x = (enemy_a - player.a) * (SCREEN_WIDTH/ player.fov) + SCREEN_WIDTH/2.0f - enemy_size/2.0f;
    int enemy_y = (SCREEN_HEIGHT/2.0f) - enemy_size/2.0f;

    for(int x = enemy_x; x < enemy_x + enemy_size; x++){
      for(int y = enemy_y; y < enemy_y + enemy_size; y++){
        int tx = (x - enemy_x) * tsize / enemy_size;
        int ty = (y - enemy_y) * tsize / enemy_size;

        Color c = ImageLoader::getPixelColor(enemy.texture, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, x, y);
      }
    }
  }

  Impact cast_ray(float a) {
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>(player.x + d * cos(a)); 
      int y = static_cast<int>(player.y + d * sin(a)); 
      
      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }
     
      //point(x, y, W);
      
      d += 1;
    }
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 

  void minimap_bg(SDL_Renderer* renderer) {
        int size = 230;
        ImageLoader::render(renderer, "bk", 0, 0, size, 110);
  }


 Impact cast_ray_map(float a) {
        float d = 0;
        std::string mapHit;
        int tx;
        int x = static_cast<int>(player.mapx + d * cos(a));
        int y = static_cast<int>(player.mapy + d * sin(a));

        std::vector<std::pair<int, int>> pointsToDraw; // Para almacenar los puntos a dibujar
        std::vector<Color> pointColors; // Para almacenar los colores de los puntos

        while (true) {
            int i = static_cast<int>(x / (BLOCK / 5));
            int j = static_cast<int>(y / (BLOCK / 5));

            if (map[j][i] != ' ' && map[j][i] != '.') {
                mapHit = map[j][i];
                int hitx = x - i * static_cast<int>(BLOCK / 5);
                int hity = y - j * static_cast<int>(BLOCK / 5);
                int maxHit;
                if (hitx == 0 || hitx == static_cast<int>(BLOCK / 5) - 1) {
                    maxHit = hity;
                } else {
                    maxHit = hitx;
                }
                tx = maxHit * textSize / static_cast<int>(BLOCK / 5);
                break;
            }

            d += 1;

            // Agrega el punto a la lista de puntos a dibujar
            pointsToDraw.push_back({x, y});
            pointColors.push_back(W); // Color de los puntos en el mapa (puedes cambiarlo si es necesario)

            x = static_cast<int>(player.mapx + d * cos(a));
            y = static_cast<int>(player.mapy + d * sin(a));
        }

        // Dibuja todos los puntos en el mapa a la vez con sus colores correspondientes
        points(pointsToDraw, pointColors);

        return Impact{d, mapHit, tx};
    }

  bool isWallCollision(float newX, float newY){
      bool isWall = false;
      if(newY> map.size() || newX>map[0].size()){
        int y = static_cast<int>((newY+0.01 *BLOCK)/BLOCK);
        int x = static_cast<int>((newX+0.01 *BLOCK)/BLOCK);

        isWall = map[y][x]!=' '&& map[y][x]!= '.';

        if(!isWall){
          y = static_cast<int>((newY-0.01 *BLOCK)/BLOCK);
          x = static_cast<int>((newX-0.01 *BLOCK)/BLOCK);

          isWall = map[y][x]!=' '&& map[y][x]!= '.';
        }

      }

      return isWall;
  }

  bool has_won() {
        int player_x = static_cast<int>(player.x / BLOCK);
        int player_y = static_cast<int>(player.y / BLOCK);

        // Comprueba si el jugador está en una posición de victoria ('y' en el mapa)
        if (map[player_y][player_x] == '.') {
            return true;
        }

        return false;
    }

    void draw_victory_screen() {
        // Limpia el renderizador
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dibuja la pantalla de victoria (imagen o texto)
        ImageLoader::render(renderer, "w1", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Refresca el renderizador
        SDL_RenderPresent(renderer);
    }

 
  void render() {
    
    // draw left side of the screen
    
    for (int x = 0; x < (SCREEN_WIDTH); x += BLOCK) {
      for (int y = 0; y < (SCREEN_HEIGHT); y += BLOCK) {
        int i = static_cast<int>(x / BLOCK);
        int j = static_cast<int>(y / BLOCK);
        
        if (map[j][i] != ' ') {
          std::string mapHit;
          mapHit = map[j][i];
          Color c = Color(255, 0, 0);
          rect(x, y, mapHit);
        }
      }
    }

    for (int i = 1; i < SCREEN_WIDTH; i++) {
      float a = player.a + player.fov / 2 - player.fov * i / SCREEN_WIDTH;
      cast_ray(a);
    }
    
        
    // draw right side of the screen

    // Recopila todos los puntos a dibujar junto con sus colores
    std::vector<std::pair<int, int>> pointsToDraw;
    std::vector<Color> pointColors;
    const double playerCosA = cos(player.a);
    const double playerSinA = sin(player.a);

    for (int i = 0; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a, playerCosA, playerSinA);
      float d = impact.d;
      Color c = Color(255, 0, 0);

      
      int x = i;
      float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d * cos(a - player.a)) * static_cast<float>(scale);
      float start = SCREEN_HEIGHT / 2.0f - h / 2.0f;
      float end = start + h;

            // Agrega los puntos y sus colores a las listas correspondientes
      #pragma omp parallel for // Inicio de la sección paralela
      for (int y = static_cast<int>(start); y < static_cast<int>(end); y++) {
        int ty = static_cast<int>(((y - start) * textSize) / h);
        Color pointColor = ImageLoader::getPixelColor(impact.mapHit, impact.tx, ty);
        int px = x;
        int py = y;
        pointsToDraw.push_back({px, py});
        pointColors.push_back(pointColor);
     }
    }

    // Dibuja los puntos en batch con sus colores correspondientes
    points(pointsToDraw, pointColors);    


    for(Enemy enemy : enemies){
      draw_enemy(enemy);
    }

  }

  Player player;
  float incertidumbreY;
  float incertidumbreX;
private:
  int scale;
  SDL_Renderer* renderer;
  std::vector<std::string> map;
  int tsize;
  int textSize;

  Impact cast_ray(float a, double playerCosA, double playerSinA) {
        float d = 0;
        std::string mapHit;
        int tx;
        int x = static_cast<int>(player.x);
        int y = static_cast<int>(player.y);
        double cosA = cos(a);
        double sinA = sin(a);

        while (true) {
            x = static_cast<int>(player.x + d * cosA);
            y = static_cast<int>(player.y + d * sinA);

            if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
                // El rayo está fuera de la pantalla, detén la búsqueda.
                break;
            }

            int i = static_cast<int>(x / BLOCK);
            int j = static_cast<int>(y / BLOCK);

            if (map[j][i] != ' ' && map[j][i] != '.') {
                mapHit = map[j][i];
                int hitx = x - i * BLOCK;
                int hity = y - j * BLOCK;
                int maxHit;
                if (hitx == 0 || hitx == BLOCK - 1) {
                    maxHit = hity;
                } else {
                    maxHit = hitx;
                }
                tx = maxHit * textSize / BLOCK;
                break;
            }

            

            d += 1;
        }

        return Impact{d, mapHit, tx};
    }
};