//
// Created by Claude on 17/11/2026.
//

#pragma once

#include <SDL.h>

#include <string>
#include <vector>

#include "MapaDoManche.h"

/// Os joysticks ligados a maquina - o encoder do gabinete, quando ele se apresenta como joystick.
/// Abre cada um que aparece (tambem os ligados com o jogo aberto), fecha o que sai, e a cada quadro
/// liga no teclado as teclas do painel que eles apertam (MapaDoManche).
///
/// O diagnostico vai para o log (no gabinete, Saves/jogo.log): o nome de cada joystick, quantos
/// eixos, direcionais e botoes ele tem, e cada botao, direcional e eixo que se mexe. E com ele que
/// se acerta o Assets/painel.json de um encoder novo.
class Manches {
public:
    /// Le a configuracao. Sem o arquivo, ou com erro nele, fica o padrao (MapaDoManche::Padrao).
    void Carregar(const std::string& caminho);

    /// Os eventos de joystick: chegada, saida e o diagnostico.
    void AoEvento(const SDL_Event& evento);

    /// Liga em 'teclado' as teclas que os joysticks apertam neste quadro - e as do START e do
    /// SELECT que chegaram por evento (MapaDoManche::DonoDoSistema), por uns quadros.
    void Apertar(Uint8* teclado, int quantasTeclas);

    [[nodiscard]] bool AlgumLigado() const { return !mAbertos.empty(); }

    /// Fecha todos. Antes do SDL_Quit.
    void Fechar();

private:
    struct Aberto {
        SDL_Joystick* joystick = nullptr;
        SDL_JoystickID id = -1;
        /// Para o diagnostico: de que lado da zona morta cada eixo estava (-1, 0 ou 1).
        std::vector<int> lados;
    };

    [[nodiscard]] int Indice(SDL_JoystickID id) const;
    [[nodiscard]] MapaDoManche::Leitura Ler(const Aberto& aberto) const;

    /// O START e o SELECT chegam tambem pelo evento do botao, de qualquer joystick: a tecla do dono
    /// fica apertada por kQuadrosDoPulso quadros, e quem nasceu com o dispositivo trocado nao perde
    /// o botao. As cenas leem a borda, entao o pulso junto com a leitura normal conta uma vez so.
    struct Pulso {
        SDL_Scancode tecla = SDL_SCANCODE_UNKNOWN;
        int quadros = 0;
    };
    static constexpr int kQuadrosDoPulso = 3;
    std::vector<Pulso> mPulsos;

    MapaDoManche::Configuracao mConfiguracao = MapaDoManche::Padrao();
    /// Na ordem em que chegaram: o "dispositivo" do painel.json e a posicao aqui.
    std::vector<Aberto> mAbertos;

    /// O diagnostico de cada botao, direcional e eixo serve para acertar o painel.json, e isso se faz
    /// nos primeiros toques: depois de kDiagnosticosNoMaximo linhas por sessao do jogo ele se cala, e
    /// o jogo.log nao cresce com cada partida de um gabinete que fica ligado o dia todo. A chegada e a
    /// saida de um joystick continuam no log.
    static constexpr int kDiagnosticosNoMaximo = 300;
    int mDiagnosticos = 0;
    /// Se este toque ainda vai para o log; no ultimo, avisa que o resto nao vai.
    bool Diagnosticar();
};
