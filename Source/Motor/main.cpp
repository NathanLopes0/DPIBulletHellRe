#include "Game.h"
#include "Gabinete.h"
#include "../Simulacao/Simulacao.h"

#include <string>
#include <vector>

// Screen dimension constants.
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

int main(int argc, char** argv)
{
    // O jogo todo e posicionado neste tamanho, inclusive as telas que usam fracoes da altura.
    std::vector<std::string> argumentos;
    for (int i = 1; i < argc; ++i) {
        if (argv[i]) argumentos.emplace_back(argv[i]);
    }

    // Um parametro de simulacao que nao se le e dito e ignorado: a medida roda com o resto.
    std::vector<std::string> problemas;
    const Simulacao::Configuracao simulacao = Simulacao::Ler(argumentos, &problemas);
    for (const auto& p : problemas) SDL_Log("SIMULACAO: %s; ignorado.", p.c_str());

    Game game = Game(SCREEN_WIDTH, SCREEN_HEIGHT, Gabinete::Ler(argumentos), simulacao);
    bool success = game.Initialize();
    if (success)
    {
        game.RunLoop();
    }
    game.Shutdown();
    return 0;
}
