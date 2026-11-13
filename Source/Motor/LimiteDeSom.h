//
// O limite dos efeitos sonoros: quantas copias de um som tocam juntas, e de quanto em quanto.
//

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/// Um som que toca muitas vezes seguidas - o disparo do professor, o tiro do aluno - vira um
/// barulho continuo que cobre a musica: o disparo dura 0,7 s, e um chefe que solta uma gota a cada
/// 0,06 s encheria os canais todos. Cada grupo de sons tem um maximo de copias tocando juntas e um
/// intervalo minimo entre dois comecos; o que passa disso nao toca. A musica (o som em laco) nao
/// entra na conta.
namespace LimiteDeSom {
    struct Regra {
        int simultaneos = 3;           ///< copias do grupo tocando juntas, no maximo
        std::uint32_t intervaloMs = 30;   ///< o minimo entre dois comecos do grupo
    };

    /// O padrao de qualquer som: so nao deixa o mesmo som empilhar no mesmo instante.
    constexpr Regra kPadrao{3, 30};
    /// O disparo do professor: duas copias, no maximo, e ao menos 0,11 s entre elas.
    constexpr Regra kDisparo{2, 110};
    /// O tiro do aluno: duas copias (a dupla atira junto), ao menos 0,06 s entre elas.
    constexpr Regra kTiro{2, 60};

    class Controle {
    public:
        /// Se o 'grupo' pode comecar um som em 'agoraMs', com 'tocando' copias dele ainda tocando,
        /// pela 'regra'. Se puder, marca o comeco.
        bool PodeTocar(const std::string& grupo, std::uint32_t agoraMs, int tocando, const Regra& regra);

    private:
        std::unordered_map<std::string, std::uint32_t> mUltimoComeco;
    };
}
