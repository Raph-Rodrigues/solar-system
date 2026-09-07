#include <SDL3/SDL.h>
#include <cstdio>

int main(int argc, char *argv[]) {
  // inicializa o subsistema de video do SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Erro ao inicializar o SDL: %s", SDL_GetError());
    return 1;
  }

  // cria janela
  SDL_Window *window =
      SDL_CreateWindow("Solar System", 800, 600, SDL_WINDOW_RESIZABLE);

  if (!window) {
    printf("Erro ao criar janela: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // cria renderizador com janela e o driver (nullptr, faz procurar o melhor
  // driver disponivel no seu SO)
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

  if (!renderer) {
    printf("Erro ao criar renderizador: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  bool running = true;
  SDL_Event event;

  // loop principal
  while (running) {
    // processa eventos
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    // atualização

    // renderização

    // limpa tela com uma cor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // preto
    SDL_RenderClear(renderer);

    // atualiza a tela apresentando o que foi desenhado
    SDL_RenderPresent(renderer);
  }

  // limpeza e encerramento
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
