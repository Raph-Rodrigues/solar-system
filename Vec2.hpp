#pragma once

#include <cmath>

// Vec2: representa qualquer grandeza vetorial 2D usada na simulacao --
// posicao, velocidade ou aceleracao. Fisicamente essas tres grandezas tem
// natureza diferente (pixels, pixels/segundo, pixels/segundo^2), mas
// matematicamente todas obedecem as mesmas regras de soma e multiplicacao
// por um numero comum (escalar) -- por isso um unico tipo serve pras tres.
struct Vec2 {
  float x = 0.0f; // componente horizontal
  float y = 0.0f; // componente vertical

  // soma de vetores: usada para combinar deslocamentos, velocidades ou
  // aceleracoes (ex: velocidade nova = velocidade antiga + variacao de
  // velocidade causada pela aceleracao)
  Vec2 operator+(const Vec2 &other) const { return {x + other.x, y + other.y}; }

  // subtracao de vetores: usada principalmente para achar o vetor que liga
  // dois pontos (ex: posicao de B menos posicao de A = vetor que vai de A
  // ate B -- e exatamente isso que a lei da gravitacao usa pra saber em
  // que direcao um corpo puxa o outro)
  Vec2 operator-(const Vec2 &other) const { return {x - other.x, y - other.y}; }

  // multiplicacao por um numero comum (escalar): usada para escalar um
  // vetor por um tempo (velocidade * dt = deslocamento no intervalo dt) ou
  // por uma intensidade (direcao unitaria * modulo da aceleracao =
  // aceleracao vetorial completa)
  Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

  // modulo (comprimento) do vetor, via teorema de Pitagoras:
  // |v| = sqrt(x^2 + y^2). Usado para achar a distancia entre dois corpos
  // (quando aplicado sobre um vetor de deslocamento) ou a rapidez de um
  // corpo (quando aplicado sobre o vetor velocidade)
  float length() const { return std::sqrt(x * x + y * y); }

  // retorna um vetor unitario (modulo exatamente 1) apontando na mesma
  // direcao deste vetor. Serve pra separar "pra onde aponta" (direcao) de
  // "quao forte e" (modulo) -- exatamente como a lei da gravitacao faz:
  // a forca tem modulo G*M*m/r^2 e aponta na direcao do vetor entre os
  // dois corpos, entao calculamos essas duas partes separadamente e
  // multiplicamos no final
  Vec2 normalized() const {
    float len = length();
    if (len < 1e-6f) {
      return {0.0f, 0.0f}; // evita divisao por zero se o vetor for nulo
    }
    return {x / len, y / len};
  }
};
