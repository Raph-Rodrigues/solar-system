#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_render.h>
#include <cstdio>
#include <iostream>

int main(int argc, char *argv[]) {
  // inicializa o subsistema de video do SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Erro ao inicializar o SDL: %s", SDL_GetError());
    return 1;
  }

  // obtem a lista de monitores conectados
  int num_displays = 0;
  SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);

  if (displays == nullptr || num_displays == 0) {
    printf("Nenhum monitor encontrado ou falha: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // captura o monitor principal
  SDL_DisplayID main_monitor = displays[0];
  SDL_Rect limits;
  int width;
  int height;

  // armazena a resolucao do monitor principal nas variaveis inteiras
  if (SDL_GetDisplayBounds(main_monitor, &limits)) {
    width = limits.w;
    height = limits.h;

    std::cout << "Largura: " << width << " pixels\n" << std::endl;
    std::cout << "Altura: " << height << " pixels\n" << std::endl;
  } else {
    printf("Erro ao obter tamanho do monitor: %s", SDL_GetError());
  }

  // cria janela
  SDL_Window *window =
      SDL_CreateWindow("Solar System", width, height, SDL_WINDOW_RESIZABLE);

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

  // obtem a tabela de propriedades vinculada a este renderizador especifico
  SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
  // captura o nome do driver que foi ativado
  const char *driver_name = SDL_GetStringProperty(
      props, SDL_PROP_RENDERER_NAME_STRING, "Desconhecido");
  std::cout << "========================================" << std::endl;
  std::cout << " O driver de renderizacao ativo e: " << driver_name
            << std::endl;
  std::cout << "========================================" << std::endl;

  bool running = true;
  SDL_Event event;

  // criando um retangulo com Ponto Flutuante do SDL
  SDL_FRect rect = {100.0f, 250.0f, 100.0f, 100.0f};
  float speed = 4.0f;

  // loop principal
  while (running) {
    // processa eventos
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    // atualização
    rect.x += speed;
    if (rect.x + rect.w > width || rect.x < 0.0f) {
      speed = -speed;
    }

    // renderização

    // limpa tela com uma cor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // preto
    SDL_RenderClear(renderer);

    // alterar para cor do pincel e desenhar o retangulo com a cor desejada
    SDL_SetRenderDrawColor(renderer, 50, 150, 250, 255); // azul
    SDL_RenderFillRect(renderer, &rect);

    // atualiza a tela apresentando o que foi desenhado
    SDL_RenderPresent(renderer);

    SDL_Delay(16); // pequeno atraso para limitar o uso de CPU (Aprox. 60FPS)
  }

  // limpeza e encerramento
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_free(displays);
  SDL_Quit();
  return 0;
}
