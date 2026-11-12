//
// A ponte entre as fases lidas de arquivo e o motor.
//

#pragma once

#include <string>

class Boss;
class FSMComponent;

/// Monta as fases de um chefe a partir de Assets/Attacks/fases.json. E a unica maneira de um chefe
/// ganhar fases.
bool ConfigurarFasesDeArquivo(Boss* boss, FSMComponent* fsm, const std::string& nomeDoConjunto);
