//
// Created by Claude on 17/11/2026.
//

#pragma once

#include <SDL_stdinc.h>

#include <array>
#include <string>
#include <vector>

#include "Painel.h"

/// O painel quando o encoder do gabinete se apresenta como JOYSTICK, e nao como teclado: cada
/// posicao do painel le um joystick, e o que ele aperta vira as teclas dela (Painel::Tecla). As
/// telas continuam lendo so o teclado - nenhuma sabe que o joystick existe.
///
/// A configuracao fica em Assets/painel.json, para o gabinete mudar sem compilar de novo. Quem
/// abre os joysticks e le o estado deles e Manches.h; aqui e so a conta, sem SDL.
namespace MapaDoManche {

/// Sem eixo, sem direcional ou sem botao.
constexpr int kNenhum = -1;

/// Como uma posicao do painel aparece no joystick.
struct Posicao {
    /// Qual joystick: 0 e o primeiro que o sistema mostrou. Um encoder so para as duas posicoes
    /// e o mesmo dispositivo nas duas, com eixos e botoes diferentes.
    int dispositivo = 0;
    int eixoX = 0;
    int eixoY = 1;
    bool inverterX = false;
    bool inverterY = false;
    /// O direcional digital (hat) que tambem move o manche.
    int hat = 0;
    /// Os botoes do joystick, na ordem de Painel::Botao.
    std::array<int, 3> botoes{0, 1, 2};
    /// Para o encoder que manda o manche como botoes: na ordem de Painel::Direcao.
    std::array<int, 4> direcoes{kNenhum, kNenhum, kNenhum, kNenhum};
};

struct Configuracao {
    /// A fracao do curso do eixo a partir da qual o manche conta como empurrado.
    float zonaMorta = 0.5f;
    /// Na ordem de Painel::Jogador.
    std::array<Posicao, 2> posicoes;
};

/// Sem arquivo: o primeiro joystick e o jogador 1, o segundo e o jogador 2, ambos com os eixos 0 e
/// 1, o direcional 0 e os botoes 0, 1 e 2.
Configuracao Padrao();

/// O texto de Assets/painel.json. O que falta ou nao se le fica no padrao, e e dito em 'problemas':
/// um erro no arquivo nao pode impedir o gabinete de abrir.
Configuracao Ler(const std::string& texto, std::vector<std::string>* problemas);

/// O estado de um joystick num quadro, como o SDL o da.
struct Leitura {
    std::vector<Sint16> eixos;
    std::vector<Uint8> hats;     ///< bits SDL_HAT_UP, _RIGHT, _DOWN e _LEFT
    std::vector<Uint8> botoes;
};

/// Se o joystick empurra o manche desta posicao para este lado.
[[nodiscard]] bool Empurrado(const Leitura& leitura, const Posicao& posicao, float zonaMorta, Painel::Direcao direcao);

/// Se o joystick aperta este botao desta posicao.
[[nodiscard]] bool Apertado(const Leitura& leitura, const Posicao& posicao, Painel::Botao botao);

/// Liga em 'teclado' as teclas de 'quem' que o joystick aperta. Nunca desliga nada: o teclado de
/// verdade continua valendo junto.
void Apertar(const Leitura& leitura, const Posicao& posicao, float zonaMorta, Painel::Jogador quem,
             Uint8* teclado, int quantasTeclas);

} // namespace MapaDoManche
