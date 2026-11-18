//
// Created by Claude on 17/11/2026.
//

#include "Manches.h"

#include <algorithm>
#include <fstream>
#include <sstream>

void Manches::Carregar(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        SDL_Log("MANCHE: sem %s; o primeiro joystick e o jogador 1 e o segundo, o jogador 2.", caminho.c_str());
        mConfiguracao = MapaDoManche::Padrao();
        return;
    }
    std::stringstream texto;
    texto << arquivo.rdbuf();
    std::vector<std::string> problemas;
    mConfiguracao = MapaDoManche::Ler(texto.str(), &problemas);
    for (const std::string& p : problemas) SDL_Log("MANCHE: %s: %s; fica o padrao.", caminho.c_str(), p.c_str());
}

int Manches::Indice(const SDL_JoystickID id) const {
    for (size_t i = 0; i < mAbertos.size(); ++i) {
        if (mAbertos[i].id == id) return static_cast<int>(i);
    }
    return -1;
}

void Manches::AoEvento(const SDL_Event& evento) {
    switch (evento.type) {
        case SDL_JOYDEVICEADDED: {
            SDL_Joystick* j = SDL_JoystickOpen(evento.jdevice.which);
            if (!j) {
                SDL_Log("MANCHE: nao consegui abrir o joystick %d: %s", evento.jdevice.which, SDL_GetError());
                return;
            }
            const SDL_JoystickID id = SDL_JoystickInstanceID(j);
            if (Indice(id) >= 0) return;
            Aberto a;
            a.joystick = j;
            a.id = id;
            a.lados.assign(static_cast<size_t>(std::max(0, SDL_JoystickNumAxes(j))), 0);
            mAbertos.push_back(a);
            const char* nome = SDL_JoystickName(j);
            SDL_Log("MANCHE: joystick %d ligado: \"%s\" (%d eixos, %d direcionais, %d botoes)",
                    static_cast<int>(mAbertos.size()) - 1, nome ? nome : "sem nome", SDL_JoystickNumAxes(j),
                    SDL_JoystickNumHats(j), SDL_JoystickNumButtons(j));
            return;
        }
        case SDL_JOYDEVICEREMOVED: {
            const int i = Indice(evento.jdevice.which);
            if (i < 0) return;
            SDL_JoystickClose(mAbertos[static_cast<size_t>(i)].joystick);
            mAbertos.erase(mAbertos.begin() + i);
            SDL_Log("MANCHE: joystick %d desligado", i);
            return;
        }
        case SDL_JOYBUTTONDOWN: {
            const int dispositivo = Indice(evento.jbutton.which);
            const int botao = evento.jbutton.button;
            const bool diagnostico = Diagnosticar();
            if (diagnostico) SDL_Log("MANCHE: joystick %d, botao %d", dispositivo, botao);
            for (const Painel::Sistema s : Painel::kSistemas) {
                const int dono = MapaDoManche::DonoDoSistema(mConfiguracao, dispositivo, botao, s);
                if (dono < 0) continue;
                mPulsos.push_back(Pulso{Painel::Tecla(Painel::kJogadores[dono], s), kQuadrosDoPulso});
                if (diagnostico) {
                    SDL_Log("MANCHE: o botao %d do joystick %d e o %s do jogador %d", botao, dispositivo,
                            Painel::Nome(s), dono + 1);
                }
            }
            return;
        }
        case SDL_JOYHATMOTION: {
            if (evento.jhat.value != SDL_HAT_CENTERED && Diagnosticar()) {
                SDL_Log("MANCHE: joystick %d, direcional %d = %s%s%s%s", Indice(evento.jhat.which), evento.jhat.hat,
                        (evento.jhat.value & SDL_HAT_UP) ? "cima " : "", (evento.jhat.value & SDL_HAT_DOWN) ? "baixo " : "",
                        (evento.jhat.value & SDL_HAT_LEFT) ? "esquerda " : "", (evento.jhat.value & SDL_HAT_RIGHT) ? "direita " : "");
            }
            return;
        }
        case SDL_JOYAXISMOTION: {
            // So quando o eixo atravessa a zona morta: um eixo analogico mandaria centenas por segundo.
            const int i = Indice(evento.jaxis.which);
            if (i < 0) return;
            std::vector<int>& lados = mAbertos[static_cast<size_t>(i)].lados;
            if (evento.jaxis.axis >= lados.size()) return;
            const float v = static_cast<float>(evento.jaxis.value) / 32767.0f;
            const int lado = v >= mConfiguracao.zonaMorta ? 1 : (v <= -mConfiguracao.zonaMorta ? -1 : 0);
            if (lado == lados[evento.jaxis.axis]) return;
            lados[evento.jaxis.axis] = lado;
            if (lado != 0 && Diagnosticar()) SDL_Log("MANCHE: joystick %d, eixo %d %s", i, evento.jaxis.axis, lado > 0 ? "+" : "-");
            return;
        }
        default:
            return;
    }
}

bool Manches::Diagnosticar() {
    if (mDiagnosticos >= kDiagnosticosNoMaximo) return false;
    ++mDiagnosticos;
    if (mDiagnosticos == kDiagnosticosNoMaximo) {
        SDL_Log("MANCHE: %d toques no log; o diagnostico para aqui ate o jogo abrir de novo.", kDiagnosticosNoMaximo);
    }
    return true;
}

MapaDoManche::Leitura Manches::Ler(const Aberto& aberto) const {
    MapaDoManche::Leitura l;
    SDL_Joystick* j = aberto.joystick;
    for (int e = 0; e < SDL_JoystickNumAxes(j); ++e) l.eixos.push_back(SDL_JoystickGetAxis(j, e));
    for (int h = 0; h < SDL_JoystickNumHats(j); ++h) l.hats.push_back(SDL_JoystickGetHat(j, h));
    for (int b = 0; b < SDL_JoystickNumButtons(j); ++b) l.botoes.push_back(SDL_JoystickGetButton(j, b));
    return l;
}

void Manches::Apertar(Uint8* teclado, const int quantasTeclas) {
    for (Pulso& p : mPulsos) {
        if (teclado && p.tecla > SDL_SCANCODE_UNKNOWN && p.tecla < quantasTeclas) teclado[p.tecla] = 1;
        --p.quadros;
    }
    mPulsos.erase(std::remove_if(mPulsos.begin(), mPulsos.end(), [](const Pulso& p) { return p.quadros <= 0; }),
                  mPulsos.end());
    if (mAbertos.empty()) return;
    for (size_t p = 0; p < mConfiguracao.posicoes.size(); ++p) {
        const MapaDoManche::Posicao& posicao = mConfiguracao.posicoes[p];
        if (posicao.dispositivo < 0 || static_cast<size_t>(posicao.dispositivo) >= mAbertos.size()) continue;
        MapaDoManche::Apertar(Ler(mAbertos[static_cast<size_t>(posicao.dispositivo)]), posicao, mConfiguracao.zonaMorta,
                              Painel::kJogadores[p], teclado, quantasTeclas);
    }
}

void Manches::Fechar() {
    for (const Aberto& a : mAbertos) SDL_JoystickClose(a.joystick);
    mAbertos.clear();
}
