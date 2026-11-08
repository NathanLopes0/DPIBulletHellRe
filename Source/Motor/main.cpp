#include "Game.h"
#include "Gabinete.h"

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

    Game game = Game(SCREEN_WIDTH, SCREEN_HEIGHT, Gabinete::Ler(argumentos));
    bool success = game.Initialize();
    if (success)
    {
        game.RunLoop();
    }
    game.Shutdown();
    return 0;
}
