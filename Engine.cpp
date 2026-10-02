#include "Engine.hpp"

#include <utility>

#include "Physics.hpp"
#include "Render.hpp"

// adiciona um novo corpo a simulacao (Sol, planeta, lua, outra estrela, o
// que for)
void Engine::addBody(Body body) { bodies.push_back(std::move(body)); }

// avanca a simulacao inteira por um passo de tempo dt
void Engine::update(float dt) {
  // 1. calcula a aceleracao resultante sobre cada corpo, somando a
  // atracao gravitacional de TODOS os outros corpos da simulacao
  // (interacao par a par: cada par de corpos se atrai mutuamente, pela
  // 3a Lei de Newton -- acao e reacao). Guardamos as aceleracoes num
  // vetor separado em vez de aplicar na hora porque a forca sobre o
  // corpo A depende da posicao ATUAL do corpo B, e nao queremos que
  // mover A no meio do calculo afete a forca que B sente de volta.
  std::vector<Vec2> accelerations(bodies.size(), Vec2{0.0f, 0.0f});

  for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = 0; j < bodies.size(); ++j) {
      if (i == j) {
        continue; // um corpo nao exerce forca gravitacional sobre si mesmo
      }
      accelerations[i] =
          accelerations[i] + gravitationalAcceleration(bodies[i], bodies[j]);
    }
  }
  // nota: esse duplo loop e O(n^2) -- compara cada corpo com todos os
  // outros. Para a quantidade de corpos de um sistema solar (dezenas),
  // isso e instantaneo; sistemas com milhares de corpos (simulacoes de
  // galaxias, por exemplo) precisariam de algoritmos mais espertos, como
  // Barnes-Hut, para nao ficar lento.

  // 2. integra cada corpo com a aceleracao total que ele recebeu, e
  // registra sua nova posicao no rastro
  for (size_t i = 0; i < bodies.size(); ++i) {
    integrate(bodies[i].position, bodies[i].velocity, accelerations[i], dt);
    bodies[i].recordTrailPoint();
  }
}

// desenha a simulacao inteira: todos os rastros primeiro, depois todos os
// corpos por cima -- assim nenhum rastro aparece sobreposto a um corpo que
// deveria estar na frente dele
void Engine::render(SDL_Renderer *renderer) const {
  for (const auto &body : bodies) {
    drawTrail(renderer, body);
  }
  for (const auto &body : bodies) {
    drawCircle(renderer, body);
  }
}
