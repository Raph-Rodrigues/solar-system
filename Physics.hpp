#pragma once

#include "Body.hpp"
#include "Vec2.hpp"

// Constante gravitacional AJUSTADA para a escala de pixels desta
// simulacao. O valor real de G (6.674e-11 N*m^2/kg^2) so faz sentido
// fisico com distancias em METROS e massas em QUILOGRAMAS -- aqui as
// distancias sao medidas em pixels e as massas sao numeros arbitrarios
// escolhidos a dedo, entao usar o G real resultaria numa forca
// praticamente nula. Este valor foi ajustado para as orbitas ficarem
// visualmente razoaveis nesta escala; ele nao representa nenhuma unidade
// fisica real. (O proximo passo natural do projeto, mais pra frente, e
// introduzir uma escala real metros-por-pixel e usar o G verdadeiro.)
inline constexpr float G = 500.0f;

// distancia minima (ao quadrado) permitida entre dois corpos antes de
// calcularmos a forca entre eles. Sem isso, se dois corpos chegassem muito
// perto um do outro, r^2 se aproximaria de zero e a forca (que depende de
// 1/r^2) explodiria para um valor absurdamente grande -- um artefato
// numerico, nao um efeito fisico real. Aqui so evitamos o problema; uma
// simulacao mais completa trataria isso como uma colisao de verdade.
inline constexpr float minDistanceSquared = 25.0f;

// Calcula a aceleracao que o corpo "other" produz sobre o corpo "self",
// pela Lei da Gravitacao Universal de Newton:
//
//   F = G * m_self * m_other / r^2      (modulo da forca entre os dois)
//   a_self = F / m_self = G * m_other / r^2   (aceleracao sobre self)
//
// Repare que a massa do PROPRIO corpo (m_self) cancela entre as duas
// linhas -- a aceleracao que um corpo sofre NAO depende da propria massa
// dele, so da massa do corpo que o atrai e da distancia entre eles. Esse e
// o mesmo principio por tras da demonstracao classica de Galileu: numa
// queda livre, uma pena e uma bola de ferro aceleram igual (ignorando o
// atrito do ar), porque a aceleracao gravitacional nao "sabe" quanto cada
// uma pesa.
inline Vec2 gravitationalAcceleration(const Body &self, const Body &other) {
  Vec2 delta = other.position - self.position; // vetor de self ate other
  float distSqr = delta.x * delta.x + delta.y * delta.y; // r^2

  if (distSqr < minDistanceSquared) {
    return {0.0f, 0.0f}; // corpos colidindo/sobrepostos: ignora a forca
  }

  float accelMagnitude = G * other.mass / distSqr; // modulo: G*M_other/r^2
  Vec2 direction = delta.normalized(); // so a direcao (modulo 1) de self
                                       // para other
  return direction * accelMagnitude;   // aceleracao vetorial completa
}

// Avanca posicao e velocidade de um corpo por um passo de tempo dt, dada
// uma aceleracao (considerada constante durante esse passo curto).
//
// Metodo: Euler semi-implicito. A diferenca para o Euler "ingenuo"
// (explicito) esta na ORDEM das duas linhas: aqui a velocidade e
// atualizada PRIMEIRO, e e essa velocidade JA NOVA que move a posicao
// logo em seguida (em vez de usar a velocidade antiga). Essa pequena
// mudanca faz o metodo ser muito mais estavel em simulacoes longas, como
// uma orbita rodando por varios minutos seguidos -- o Euler explicito
// tende a injetar energia artificial no sistema com o tempo, fazendo
// orbitas espiralarem para fora mesmo sem nenhuma forca extra agindo.
inline void integrate(Vec2 &position, Vec2 &velocity, Vec2 acceleration,
                      float dt) {
  velocity = velocity + acceleration * dt; // v = v + a*dt
  position = position + velocity * dt;     // x = x + v*dt (v ja atualizada)
}
