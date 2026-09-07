#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <numbers>
#include <vector>

void drawCircle(SDL_Renderer *renderer, float cx, float cy, float radius,
                int sides, SDL_FColor color) {
  std::vector<SDL_Vertex> vertices;
  vertices.reserve(sides + 1);

  // vertice central: compartilhado por todos os triangulos do leque
  SDL_Vertex center;
  center.position = {cx, cy};
  center.color = color;
  center.tex_coord = {0.0f, 0.0f};
  vertices.push_back(center);

  // vertices do poligono inscrito, um para cada lado
  for (int k = 0; k < sides; k++) {
    float angle = (2.0f * std::numbers::pi * k) / sides;
    SDL_Vertex v;
    v.position = {cx + radius * std::cos(angle), cy + radius * std::sin(angle)};
    v.color = color;
    v.tex_coord = {0.0f, 0.0f};
    vertices.push_back(v);
  }

  // indices: cada triangulo liga o centro (indice 0) a dois vertices
  // consecutivos do poligono
  std::vector<int> index;
  index.reserve(sides * 3);
  for (int k = 1; k <= sides; k++) {
    index.push_back(0);
    index.push_back(k);
    index.push_back(k == sides ? 1 : k + 1); // fecha o leque no ultimo
  }

  SDL_RenderGeometry(renderer, nullptr, vertices.data(),
                     static_cast<int>(vertices.size()), index.data(),
                     static_cast<int>(index.size()));
}

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
  SDL_Renderer *renderer = SDL_CreateRenderer(window, "vulkan");

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

  constexpr float sunX = 400.0f;
  constexpr float sunY = 300.0f;
  constexpr float sunRadius = 50.0f;

  // parametros da orbita do planeta
  float orbitRadius = 150.0f; // distancia do planeta ao sol, em pixels
  float planetRadius = 15.0f; // raio visual do planeta
  float angle = 0.0f;         // angulo atual na orbita, em radianos
  float orbitalPeriod = 4.0f; // segundos para completar uma volta
  float angularSpeed =
      2.0f * std::numbers::pi_v<float> / orbitalPeriod; // rad/s
  constexpr float dt = 1.0f / 60.0f; // passo de tempo fixo (~60 FPS)

  // loop principal
  while (running) {
    // processa eventos
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    // atualização
    angle += angularSpeed * dt; // avanca o angulo do planeta em sua orbita

    // renderização

    // limpa tela com uma cor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // preto
    SDL_RenderClear(renderer);

    // posicao do planeta na orbita, calculada a partir do angulo atual
    float planetX = sunX + orbitRadius * std::cos(angle);
    float planetY = sunY + orbitRadius * std::sin(angle);

    // alterar para cor do pincel e desenhar o retangulo com a cor desejada
    SDL_FColor yellow = {1.0f, 0.9f, 0.2f, 1.0f};
    drawCircle(renderer, sunX, sunY, sunRadius, 48, yellow);

    SDL_FColor blue = {50.0f / 255.0f, 150.0f / 255.0f, 250.0f / 255.0f, 1.0f};
    drawCircle(renderer, planetX, planetY, planetRadius, 32, blue);

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
