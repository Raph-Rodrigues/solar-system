#pragma once

#include <SDL3/SDL.h>
#include <vector>

#include "Body.hpp"

// Engine gerencia a lista de corpos celestes da simulacao inteira: aplica
// a fisica (gravitacao mutua entre todos os pares de corpos + integracao
// numerica) a cada passo de tempo, e desenha todos os corpos na tela.
// Adicionar um novo planeta, lua ou estrela e so uma chamada a addBody --
// a fisica generaliza sozinha para qualquer quantidade de corpos, sem
// precisar escrever nenhum codigo novo.
//
// Repare que este header NAO inclui Physics.hpp nem Render.hpp -- essas
// duas dependencias so existem dentro da IMPLEMENTACAO de update()/render()
// (em Engine.cpp), nao na interface publica da classe. Isso e deliberado:
// qualquer arquivo que so precise CHAMAR update()/render() (como main.cpp)
// nao precisa incluir essas dependencias transitivamente, o que reduz o
// acoplamento entre arquivos e acelera a recompilacao (mudar Physics.hpp
// agora so forca a recompilacao de Engine.cpp, nao de todo arquivo que usa
// Engine).
class Engine {
public:
  std::vector<Body> bodies; // todos os corpos celestes da simulacao

  // adiciona um novo corpo a simulacao -- implementacao em Engine.cpp
  void addBody(Body body);

  // avanca a simulacao inteira por um passo de tempo dt -- implementacao
  // em Engine.cpp
  void update(float dt);

  // desenha a simulacao inteira -- implementacao em Engine.cpp
  void render(SDL_Renderer *renderer) const;
};
