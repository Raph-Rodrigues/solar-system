#include "Body.hpp"

#include <utility>

// Construtor: inicializa cada campo do corpo com o valor recebido pelo
// chamador. O nome e MOVIDO (std::move) em vez de copiado -- uma
// std::string pode alocar memoria internamente para guardar seus
// caracteres, e copiar isso tem um custo; como o chamador normalmente
// passa uma string temporaria (ex: Body("Sol", ...)), mover e so transferir
// a posse dessa memoria, sem realocar nada.
Body::Body(std::string bodyName, Vec2 startPosition, Vec2 startVelocity,
           float bodyMass, float bodyRadius, SDL_FColor bodyColor,
           size_t trailLength)
    : name(std::move(bodyName)), position(startPosition),
      velocity(startVelocity), mass(bodyMass), radius(bodyRadius),
      color(bodyColor), maxTrailLength(trailLength) {}

// registra a posicao atual no historico de rastro, descartando o ponto
// mais antigo quando o limite maxTrailLength e ultrapassado. Chamado uma
// vez por frame, depois que a posicao do corpo ja foi atualizada pela
// integracao numerica daquele passo de tempo.
void Body::recordTrailPoint() {
  trail.push_back(position);
  if (trail.size() > maxTrailLength) {
    trail.pop_front(); // remove o ponto mais antigo do rastro
  }
}
