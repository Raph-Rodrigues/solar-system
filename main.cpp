#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <deque>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

// Vec2: tipo basico para grandezas vetoriais 2D (posicao, velocidade,
// aceleracao, forca). Separado do SDL_FPoint porque aqui ele carrega as
// operacoes matematicas (soma, subtracao, multiplicacao por escalar) que
// a fisica realmente usa nas equacoes.
struct Vector2 {
  float x = 0.0f;
  float y = 0.0f;

  Vector2 operator+(const Vector2 &other) const {
    return {x + other.x, y + other.y};
  }

  Vector2 operator-(const Vector2 &other) const {
    return {x - other.x, y - other.y};
  }

  Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
};

const char *pickPreferredRenderer() {
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
      return name;
    }
  }
  return nullptr; // nenhum preferido disponivel: deixa o SDL escolher
}

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

using Trail = std::deque<SDL_FPoint>;

void addTrailPoint(Trail &trail, float x, float y, size_t maxLength) {
  trail.push_back({x, y});
  if (trail.size() > maxLength) {
    trail.pop_front(); // remove o mais antigo
  }
}

void drawTrail(SDL_Renderer *renderer, const Trail &trail, Uint8 r, Uint8 g,
               Uint8 b) {
  size_t n = trail.size();
  if (n < 2)
    return;

  for (size_t i = 0; i + 1 < n; ++i) {
    // t vai de 0 (ponto mais antigo) ate 1 (ponto mais recente)
    float t = static_cast<float>(i) / static_cast<float>(n - 1);
    Uint8 alpha = static_cast<Uint8>(t * 255.0f);
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
    SDL_RenderLine(renderer, trail[i].x, trail[i].y, trail[i + 1].x,
                   trail[i + 1].y);
  }
}

// Avanca posicao e velocidade por um passo de tempo dt, dada uma
// aceleracao (constante durante esse passo). Metodo: Euler semi-implicito.
//
// A diferenca para o Euler "ingenuo" (explicito): aqui a velocidade e
// atualizada PRIMEIRO, e e essa velocidade JA NOVA que move a posicao
// logo em seguida. Isso faz o metodo ser bem mais estavel em simulacoes
// longas (como uma orbita) -- o Euler explicito tende a "ganhar" energia
// artificialmente com o tempo, fazendo a orbita espiralar pra fora aos
// poucos mesmo sem nenhuma forca extra agindo.
void integrate(Vector2 &position, Vector2 &velocity, Vector2 acceleration,
               float dt) {
  velocity = velocity + acceleration * dt; // v = v + a*dt
  position = position + velocity * dt;     // x = x + v*dt (v ja atualizada)
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
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

  // cria janela
  SDL_Window *window =
      SDL_CreateWindow("Solar System", width, height, SDL_WINDOW_RESIZABLE);

  if (!window) {
    printf("Erro ao criar janela: %s", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  // cria renderizador com janela e o driver (nullptr, faz procurar o primeiro
  // da lista de drivers disponivel no seu SO) a funcao faz a logica de escolher
  // o melhor renderizador de cada plataforma
  SDL_Renderer *renderer = SDL_CreateRenderer(window, pickPreferredRenderer());
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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

  bool running = true;
  SDL_Event event;

  float sunX = (float)width / 2;
  float sunY = (float)height / 2;
  constexpr float sunRadius = 50.0f;

  // parametros da orbita do planeta
  float orbitRadius = 150.0f; // distancia do planeta ao sol, em pixels
  float planetRadius = 15.0f; // raio visual do planeta
  float angle = 0.0f;         // angulo atual na orbita, em radianos
  float orbitalPeriod = 4.0f; // segundos para completar uma volta
  float angularSpeed =
      2.0f * std::numbers::pi_v<float> / orbitalPeriod; // rad/s
  constexpr size_t maxTrailLength = 200; // 3.3s de rastro a 60 FPS
  Trail sunTrail;
  Trail planetTrail;
  constexpr float dt = 1.0f / 60.0f; // passo de tempo fixo (~60 FPS)

  // --- teste do motor: queda livre com quique ---
  // forca conhecida e constante (gravidade), sem nada de orbita ainda.
  // serve pra confirmar que integrate() esta correto antes de implementar
  // a lei da gravitacao universal, que e bem mais complexa (forca varia
  // com a distancia, e dois corpos se influenciam mutuamente).
  Vector2 ballPos = {200.0f, 100.0f};
  Vector2 ballVel = {180.0f, 0.0f};           // velocidade horizontal inicial
  constexpr Vector2 gravity = {0.0f, 700.0f}; // aceleracao constante (px/s^2)
  constexpr float ballRadius = 14.0f;
  constexpr float restitution = 0.75f; // fracao de velocidade mantida no quique

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

    // avanca a bolinha usando o motor de integracao generico
    integrate(ballPos, ballVel, gravity, dt);

    // colisao simples com o chao: inverte a velocidade vertical,
    // perdendo uma fracao de energia a cada quique (senao quicaria
    // pra sempre na mesma altura, o que nao e fisico)
    if (ballPos.y + ballRadius > height) {
      ballPos.y = height - ballRadius;
      ballVel.y = -ballVel.y * restitution;
    }
    // colisao simples com as paredes laterais
    if (ballPos.x - ballRadius < 0.0f) {
      ballPos.x = ballRadius;
      ballVel.x = -ballVel.x * restitution;
    } else if (ballPos.x + ballRadius > width) {
      ballPos.x = width - ballRadius;
      ballVel.x = -ballVel.x * restitution;
    }

    // renderização

    // limpa tela com uma cor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // preto
    SDL_RenderClear(renderer);

    // posicao do planeta na orbita, calculada a partir do angulo atual
    float planetX = sunX + orbitRadius * std::cos(angle);
    float planetY = sunY + orbitRadius * std::sin(angle);

    addTrailPoint(sunTrail, sunX, sunY, maxTrailLength);
    addTrailPoint(planetTrail, planetX, planetY, maxTrailLength);

    drawTrail(renderer, sunTrail, 255, 230, 50);
    drawTrail(renderer, planetTrail, 50, 150, 250);

    // alterar para cor do pincel e desenhar o retangulo com a cor desejada
    SDL_FColor yellow = {1.0f, 0.9f, 0.2f, 1.0f};
    drawCircle(renderer, sunX, sunY, sunRadius, 48, yellow);

    SDL_FColor blue = {50.0f / 255.0f, 150.0f / 255.0f, 250.0f / 255.0f, 1.0f};
    drawCircle(renderer, planetX, planetY, planetRadius, 32, blue);

    SDL_FColor green = {0.4f, 0.9f, 0.4f, 1.0f};
    drawCircle(renderer, ballPos.x, ballPos.y, ballRadius, 24, green);

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
