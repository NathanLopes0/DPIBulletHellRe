//
// A ponte entre as regras lidas de arquivo e o motor.
//

#pragma once

#include <functional>
#include <string>

#include "RegrasDeAtaque.h"
#include "../Actors/Projectile.h"

/// Devolve um configurator que aplica um conjunto de regras de Assets/Attacks/<chefe>/regras.json.
std::function<void(Projectile*, int)> ConfiguratorDeArquivo(const std::string& chefe,
                                                            const std::string& nomeDoConjunto);

/// Devolve um configurator que aplica uma lista de regras ja lida.
std::function<void(Projectile*, int)> ConfiguratorDeRegras(const std::vector<Regra>& regras);
