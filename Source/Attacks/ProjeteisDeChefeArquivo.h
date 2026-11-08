//
// A ponte entre os tipos de projetil lidos de arquivo e o motor.
//

#pragma once

#include <string>

class Boss;

/// Registra no chefe todos os projeteis do seu conjunto em Assets/Attacks/projeteis.json. E a unica
/// maneira de um chefe ganhar projeteis.
bool RegistrarProjeteisDeArquivo(Boss* boss, const std::string& nomeDoConjunto);
