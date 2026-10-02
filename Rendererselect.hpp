#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <string>
#include <vector>

// Consulta todos os drivers de renderizacao disponiveis no sistema
// operacional atual (SDL_GetRenderDriver ja retorna essa lista na ordem
// que o proprio SDL considera mais razoavel para a plataforma) e escolhe
// explicitamente o mais performatico para cada uma: Direct3D no Windows,
// Vulkan no Linux, Metal no macOS. Essas macros de plataforma
// (SDL_PLATFORM_*) sao resolvidas em tempo de COMPILACAO -- cada binario
// so "sabe" sobre a plataforma para a qual foi compilado. Se nenhum driver
// da lista de preferencia estiver disponivel, retorna nullptr, o que faz o
// SDL_CreateRenderer escolher automaticamente pela propria ordem de
// prioridade interna.
inline const char *pickPreferredRenderer() {
  int numDrivers = SDL_GetNumRenderDrivers();
  std::vector<std::string> available;
  for (int i = 0; i < numDrivers; ++i) {
    available.emplace_back(SDL_GetRenderDriver(i));
  }

#if defined(SDL_PLATFORM_WIN32)
  static const std::vector<const char *> priority = {"direct3d12",
                                                     "direct3d11"};
#elif defined(SDL_PLATFORM_MACOS)
  static const std::vector<const char *> priority = {"metal"};
#elif defined(SDL_PLATFORM_LINUX)
  static const std::vector<const char *> priority = {"vulkan"};
#else
  static const std::vector<const char *> priority = {};
#endif

  for (const char *name : priority) {
    if (std::find(available.begin(), available.end(), name) !=
        available.end()) {
      return name; // driver preferido para esta plataforma esta disponivel
    }
  }
  return nullptr; // nenhum preferido disponivel: deixa o SDL escolher
}
