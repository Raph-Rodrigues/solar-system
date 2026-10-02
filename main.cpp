#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <iostream>

#include "Body.hpp"
#include "Engine.hpp"
#include "Physics.hpp"
#include "Rendererselect.hpp"
#include "Vec2.hpp"

int main(int argc, char *argv[]) {
  // inicializa o subsistema de video do SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Erro ao inicializar o SDL: %s", SDL_GetError());
    return -1;
  }

  // obtem a lista de monitores conectados
  int num_displays = 0;
  SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);

  if (displays == nullptr || num_displays == 0) {
    printf("Nenhum monitor encontrado ou falha: %s", SDL_GetError());
    SDL_Quit();
    return -1;
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

  // cria janela do tamanho do monitor principal
  SDL_Window *window =
      SDL_CreateWindow("Solar System", width, height, SDL_WINDOW_RESIZABLE);

  if (!window) {
    printf("Erro ao criar janela: %s", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  // cria o renderizador usando o driver mais performatico detectado para
  // este sistema operacional (ver RendererSelect.hpp)
  SDL_Renderer *renderer = SDL_CreateRenderer(window, pickPreferredRenderer());
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // habilita
                                                             // transparencia,
                                                             // usada no
                                                             // desvanecimento
                                                             // do rastro

  if (!renderer) {
    printf("Erro ao criar renderizador: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
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

  // --- construcao do sistema: Sol e um planeta em orbita circular ---
  //
  // essa e a primeira vez que a orbita nao e mais "desenhada" por uma
  // formula angular -- ela emerge de verdade da gravitacao mutua entre os
  // dois corpos, integrada passo a passo pelo Engine.

  Engine engine;

  // posicao do Sol: comeca fixo no centro da tela, mas repare que ele NAO
  // esta travado ali por codigo nenhum -- e so que a massa dele e tao
  // maior que a do planeta que o puxao gravitacional do planeta sobre ele
  // e minusculo, entao ele quase nao se move. Essa e a mesma razao fisica
  // pela qual dizemos, na pratica, que "a Terra orbita o Sol" e nao o
  // contrario: tecnicamente os dois orbitam o centro de massa comum do
  // sistema, so que esse centro fica extremamente perto do centro do Sol.
  Vec2 sunPosition = {static_cast<float>(width) / 2.0f,
                      static_cast<float>(height) / 2.0f};
  constexpr float sunMass = 25000.0f; // massa, em unidades arbitrarias
  constexpr float sunRadius = 50.0f;  // raio visual, em pixels

  engine.addBody(Body("Sol", sunPosition, Vec2{0.0f, 0.0f}, sunMass, sunRadius,
                      SDL_FColor{1.0f, 0.9f, 0.2f, 1.0f}));

  // posicao inicial do planeta: a uma distancia orbitRadius do Sol,
  // deslocado horizontalmente (a direita)
  constexpr float orbitRadius = 200.0f; // distancia inicial ao Sol, em px
  constexpr float planetMass = 10.0f;   // bem menor que a do Sol de proposito
  constexpr float planetRadius = 15.0f; // raio visual, em pixels

  Vec2 planetPosition = sunPosition + Vec2{orbitRadius, 0.0f};

  // velocidade necessaria para uma orbita CIRCULAR a essa distancia,
  // derivada igualando a forca gravitacional a forca centripeta
  // necessaria para manter o corpo em circulo:
  //
  //   G*M*m/R^2 = m*v^2/R   (forca gravitacional = forca centripeta)
  //        v = sqrt(G*M/R)
  //
  // essa e a mesma formula que chegamos deduzindo a_c = v^2/R a partir da
  // geometria de triangulos semelhantes -- agora ela entra diretamente na
  // condicao inicial do planeta, em vez de um periodo orbital arbitrario
  float orbitalSpeed = std::sqrt(G * sunMass / orbitRadius);

  // a velocidade tem que ser TANGENTE ao raio (perpendicular a ele) para a
  // orbita sair circular -- como o planeta comeca a direita do Sol
  // (deslocamento horizontal), a direcao tangente e a vertical
  Vec2 planetVelocity = {0.0f, -orbitalSpeed};

  engine.addBody(
      Body("Planeta", planetPosition, planetVelocity, planetMass, planetRadius,
           SDL_FColor{50.0f / 255.0f, 150.0f / 255.0f, 250.0f / 255.0f, 1.0f}));

  constexpr float dt = 1.0f / 60.0f; // passo de tempo fixo (~60 FPS)

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

    // atualizacao: aplica gravitacao mutua entre todos os corpos e integra
    // suas posicoes/velocidades por um passo de tempo dt
    engine.update(dt);

    // renderizacao
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // limpa com preto
    SDL_RenderClear(renderer);

    engine.render(renderer); // desenha rastros e corpos de todo o sistema

    SDL_RenderPresent(renderer); // apresenta o que foi desenhado

    SDL_Delay(16); // pequeno atraso para limitar o uso de CPU (Aprox. 60FPS)
  }

  // limpeza e encerramento
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_free(displays);
  SDL_Quit();

  return 0;
}
