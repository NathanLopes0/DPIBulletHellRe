//
// Os chefes do jogo: o nome de cada um e a fabrica que o monta.
//

#include "Chefes.h"

#include "FabricaDeChefe.h"
#include "../Bosses/Andre.h"
#include "../Bosses/Hugo.h"
#include "../Bosses/Julio.h"
#include "../Bosses/Lucas.h"
#include "../Bosses/Luizinho.h"
#include "../Bosses/MariaLucia.h"
#include "../Bosses/Ricardo.h"
#include "../Bosses/Salles.h"
#include "../Bosses/Thiago.h"
#include "../Bosses/Vitor.h"
#include "../Bosses/Vladimir.h"

namespace Chefes {

namespace {
    /// Uma linha por chefe: a classe, a pasta (que e tambem o nome) e o nome
    /// do arquivo do retrato em Assets/Teachers/DPIBH<arquivo>.png.
    template <typename Chefe>
    void Registrar(std::map<std::string, std::unique_ptr<IBossFactory>>& fabricas, Game* game,
                   const std::string& pasta, const std::string& arquivo) {
        fabricas[pasta] = std::make_unique<FabricaDeChefe<Chefe>>(game, pasta, "Teachers/DPIBH" + arquivo);
    }
}

std::map<std::string, std::unique_ptr<IBossFactory>> Fabricas(Game* game) {
    std::map<std::string, std::unique_ptr<IBossFactory>> f;
    Registrar<Salles>    (f, game, "salles",     "Salles");
    Registrar<Ricardo>   (f, game, "ricardo",    "Ricardo");
    Registrar<Andre>     (f, game, "andre",      "Andre");
    Registrar<Julio>     (f, game, "julio",      "Julio");
    Registrar<Thiago>    (f, game, "thiago",     "Thiago");
    Registrar<Vladimir>  (f, game, "vladimir",   "Vladimir");
    Registrar<Vitor>     (f, game, "vitor",      "Vitor");
    Registrar<MariaLucia>(f, game, "marialucia", "MariaLucia");
    Registrar<Lucas>     (f, game, "lucas",      "Lucas");
    Registrar<Luizinho>  (f, game, "luizinho",   "Luizinho");
    Registrar<Hugo>      (f, game, "hugo",       "Hugo");
    return f;
}

}
