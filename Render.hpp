#pragma once

#include <SDL3/SDL.h>
#include <cmath>
#include <numbers>
#include <vector>

#include "Body.hpp"

// Desenha um corpo celeste como um circulo preenchido, aproximado por um
// poligono regular de "sides" lados inscrito na circunferencia do corpo
// (metodo de exaustao de Arquimedes: quanto mais lados o poligono tem,
// mais proximo ele fica de um circulo perfeito). O desenho e feito como um
// "leque" de triangulos saindo do centro -- SDL_RenderGeometry recebe uma
// lista de vertices e uma lista de indices dizendo quais vertices formam
// cada triangulo.
inline void drawCircle(SDL_Renderer *renderer, const Body &body,
                       int sides = 48) {
  std::vector<SDL_Vertex> vertices;
  vertices.reserve(sides + 1);

  // vertice central: compartilhado por todos os triangulos do leque
  SDL_Vertex center;
  center.position = {body.position.x, body.position.y};
  center.color = body.color;
  center.tex_coord = {0.0f, 0.0f};
  vertices.push_back(center);

  // vertices do poligono inscrito, um para cada lado, distribuidos em
  // angulos igualmente espacados ao redor do corpo (2*pi radianos / sides)
  for (int k = 0; k < sides; k++) {
    float angle = (2.0f * std::numbers::pi_v<float> * k) / sides;
    SDL_Vertex v;
    v.position = {body.position.x + body.radius * std::cos(angle),
                  body.position.y + body.radius * std::sin(angle)};
    v.color = body.color;
    v.tex_coord = {0.0f, 0.0f};
    vertices.push_back(v);
  }

  // indices: cada triangulo liga o centro (indice 0) a dois vertices
  // consecutivos do poligono, fechando o ultimo triangulo de volta ao
  // primeiro vertice do poligono (indice 1)
  std::vector<int> index;
  index.reserve(sides * 3);
  for (int k = 1; k <= sides; k++) {
    index.push_back(0);
    index.push_back(k);
    index.push_back(k == sides ? 1 : k + 1);
  }

  SDL_RenderGeometry(renderer, nullptr, vertices.data(),
                     static_cast<int>(vertices.size()), index.data(),
                     static_cast<int>(index.size()));
}

// Desenha o rastro (historico de posicoes recentes) de um corpo, com
// transparencia crescente nos pontos mais recentes -- cria o efeito visual
// de uma "cauda" que desvanece conforme o corpo se afasta daquele trecho
// da trajetoria que ja percorreu.
inline void drawTrail(SDL_Renderer *renderer, const Body &body) {
  size_t n = body.trail.size();
  if (n < 2) {
    return; // precisa de pelo menos 2 pontos para desenhar um segmento
  }

  // converte a cor do corpo, que esta em ponto flutuante (0.0-1.0, formato
  // usado por SDL_RenderGeometry em drawCircle), para bytes (0-255, formato
  // que SDL_SetRenderDrawColor espera)
  Uint8 r = static_cast<Uint8>(body.color.r * 255.0f);
  Uint8 g = static_cast<Uint8>(body.color.g * 255.0f);
  Uint8 b = static_cast<Uint8>(body.color.b * 255.0f);

  for (size_t i = 0; i + 1 < n; ++i) {
    // t vai de 0 (ponto mais antigo do rastro) ate 1 (ponto mais recente)
    float t = static_cast<float>(i) / static_cast<float>(n - 1);
    Uint8 alpha = static_cast<Uint8>(t * 255.0f);
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
    SDL_RenderLine(renderer, body.trail[i].x, body.trail[i].y,
                   body.trail[i + 1].x, body.trail[i + 1].y);
  }
}
