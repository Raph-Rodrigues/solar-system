#pragma once

#include <SDL3/SDL.h>
#include <deque>
#include <string>

#include "Vec2.hpp"

// Body representa QUALQUER corpo celeste da simulacao -- planeta, luas,
// estrela, nao importa. Fisicamente, nada na Lei da Gravitacao Universal
// ou nas Leis de Newton faz distincao especial entre um planeta e uma
// estrela: as duas sao so massas pontuais (ou esfericas) que se atraem
// mutuamente. A unica coisa que diferencia uma "estrela" de um "planeta"
// aqui e a escolha de massa e raio -- por isso uma unica classe cobre os
// dois casos, em vez de precisar de uma classe Star separada.
//
// Esta e so a INTERFACE da classe (quais dados ela guarda, quais operacoes
// ela oferece). As implementacoes de verdade ficam em Body.cpp -- assim,
// quem so usa Body (como main.cpp) nao precisa reler/recompilar a logica
// interna toda vez, e os detalhes de implementacao ficam escondidos de
// quem so quer usar a classe.
class Body {
public:
  std::string name; // nome identificador, so para debug/log (ex: "Sol")

  Vec2 position; // posicao atual do corpo, em pixels na tela
  Vec2 velocity; // velocidade atual do corpo, em pixels por segundo

  float mass;   // massa do corpo, em unidades arbitrarias da simulacao
                // (nao sao quilogramas reais -- veja o comentario sobre a
                // constante G em Physics.hpp)
  float radius; // raio visual do corpo na tela, em pixels

  SDL_FColor color; // cor de desenho (componentes de 0.0 a 1.0, formato
                    // que o SDL3 espera em SDL_RenderGeometry)

  std::deque<Vec2> trail; // historico de posicoes recentes, usado para
                          // desenhar o rastro (a "cauda" que o corpo deixa
                          // para tras). E um deque (fila de duas pontas)
                          // porque precisamos adicionar no fim e remover
                          // do inicio com eficiencia O(1) a cada frame.
  size_t maxTrailLength;  // quantos pontos de rastro manter no maximo
                          // antes de comecar a descartar os mais antigos

  // construtor: apenas DECLARADO aqui. A implementacao (o que de fato
  // acontece dentro dele) esta em Body.cpp.
  Body(std::string bodyName, Vec2 startPosition, Vec2 startVelocity,
       float bodyMass, float bodyRadius, SDL_FColor bodyColor,
       size_t trailLength = 200);

  // registra a posicao atual no historico de rastro -- implementacao em
  // Body.cpp
  void recordTrailPoint();
};
