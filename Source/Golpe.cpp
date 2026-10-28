//
// O golpe do orientador no TCC: o da materia que ele da.
//

#include "Golpe.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace Golpe {

    Materia DoProfessor(const std::string& professor) {
        if (professor == "Andr\xc3\xa9" || professor == "Andre") return Materia::Andre;
        if (professor == "Salles") return Materia::Salles;
        if (professor == "Ricardo") return Materia::Ricardo;
        if (professor == "Thiago") return Materia::Thiago;
        if (professor == "Luizinho") return Materia::Luizinho;
        if (professor == "Vladimir") return Materia::Vladimir;
        if (professor == "J\xc3\xbalio" || professor == "Julio") return Materia::Julio;
        if (professor == "Maria L\xc3\xba" "cia" || professor == "Maria Lucia") return Materia::MariaLucia;
        if (professor == "Lucas") return Materia::Lucas;
        if (professor == "Vitor") return Materia::Vitor;
        return Materia::Generico;
    }

    Rodada DaVez(const Materia m, const int k, Variacao::Sorteio& s) {
        Rodada r;
        switch (m) {
            case Materia::Andre:
                // for (i = 0; i < 4; i++) - um tiro por volta do laco.
                r.duracao = 2.0f;
                for (int i = 0; i < 4; ++i) {
                    Tiro t;
                    t.quando = 0.1f * static_cast<float>(i);
                    r.tiros.push_back(t);
                }
                break;

            case Materia::Salles:
                // PUSH, PUSH, PUSH, PUSH - cada um em cima do outro -, e o POP
                // comeca pelo de cima: o ultimo empilhado sai primeiro.
                r.duracao = 2.0f;
                for (int i = 0; i < 4; ++i) {
                    Tiro t;
                    t.quando = 0.2f * static_cast<float>(i);
                    t.dy = -34.0f - 22.0f * static_cast<float>(i);
                    const float pop = 0.9f + 0.15f * static_cast<float>(3 - i);
                    t.espera = pop - t.quando;
                    r.tiros.push_back(t);
                }
                break;

            case Materia::Ricardo: {
                // O BARRAMENTO: oito fios, e so os bits 1 do byte atiram.
                r.duracao = 1.5f;
                const auto bits = Byte(s);
                for (int b = 0; b < 8; ++b) {
                    if (!bits[static_cast<size_t>(b)]) continue;
                    Tiro t;
                    t.dx = (static_cast<float>(b) - 3.5f) * 12.0f;
                    t.paralelo = true;
                    r.tiros.push_back(t);
                }
                break;
            }

            case Materia::Thiago:
                // CADA ESCRITA FICA NA LINHA, esperando o COMMIT.
                r.duracao = 3.0f;
                for (int i = 0; i < 6; ++i) {
                    Tiro t;
                    t.quando = 0.3f * static_cast<float>(i);
                    t.dx = -50.0f + 20.0f * static_cast<float>(i);
                    t.dy = -40.0f;
                    t.espera = 2.0f - t.quando;
                    r.tiros.push_back(t);
                }
                break;

            case Materia::Luizinho:
                // DOIS PONTEIROS, um de cada lado, apontando para o chefe.
                r.duracao = 1.0f;
                for (const float lado : {-60.0f, 60.0f}) {
                    Tiro t;
                    t.desvio = lado;
                    t.velocidade = 650.0f;
                    t.curva = 260.0f;
                    r.tiros.push_back(t);
                }
                break;

            case Materia::Vladimir: {
                // S -> S S, duas vezes: um tiro, dois, quatro.
                r.duracao = 2.0f;
                Tiro t;
                t.divide = 0.14f;
                t.geracoes = 1;
                r.tiros.push_back(t);
                break;
            }

            case Materia::Julio:
                // O GRADIENTE: cada passo erra para um lado, e erra menos.
                r.duracao = 1.5f;
                for (int i = 0; i < 3; ++i) {
                    Tiro t;
                    t.quando = 0.15f * static_cast<float>(i);
                    t.onda = (i % 2 == 0) ? 60.0f : -60.0f;
                    r.tiros.push_back(t);
                }
                break;

            case Materia::MariaLucia:
                // O OBSERVER nao tem relogio: quem dispara e o aluno.
                r.duracao = 1.0f;
                break;

            case Materia::Lucas:
                // O FETCH vai ao servidor, de lado, e a resposta volta no chefe.
                r.duracao = 1.5f;
                for (int i = 0; i < 3; ++i) {
                    Tiro t;
                    t.quando = 0.12f * static_cast<float>(i);
                    t.desvio = (i % 2 == 0) ? 50.0f : -50.0f;
                    t.reMira = 0.3f;
                    r.tiros.push_back(t);
                }
                break;

            case Materia::Vitor: {
                // A JANELA: uma rajada do tamanho dela.
                r.duracao = 2.2f;
                const int n = Janela(k);
                for (int i = 0; i < n; ++i) {
                    Tiro t;
                    t.quando = 0.08f * static_cast<float>(i);
                    r.tiros.push_back(t);
                }
                break;
            }

            case Materia::Generico:
            default:
                r.duracao = 1.5f;
                for (const float d : {-7.0f, 0.0f, 7.0f}) {
                    Tiro t;
                    t.desvio = d;
                    r.tiros.push_back(t);
                }
                break;
        }
        return r;
    }

    Vector2 Rumo(const Vector2& de, const Vector2& alvo, const float desvio) {
        Vector2 d = alvo - de;
        if (d.Length() < 1.0f) d = Vector2(0.0f, -1.0f);
        d.Normalize();
        const float a = Math::ToRadians(desvio);
        const float c = Math::Cos(a), sn = Math::Sin(a);
        return Vector2(d.x * c - d.y * sn, d.x * sn + d.y * c);
    }

    Vector2 Curvar(const Vector2& velocidade, const Vector2& paraOAlvo, const float grausPorSegundo, const float dt) {
        const float modulo = velocidade.Length();
        if (modulo < 1e-3f || paraOAlvo.Length() < 1e-3f) return velocidade;
        const float agora = Math::Atan2(velocidade.y, velocidade.x);
        const float quer = Math::Atan2(paraOAlvo.y, paraOAlvo.x);
        float falta = std::remainder(quer - agora, 2.0f * Math::Pi);
        const float passo = Math::ToRadians(grausPorSegundo) * dt;
        falta = std::clamp(falta, -passo, passo);
        const float a = agora + falta;
        return Vector2(Math::Cos(a), Math::Sin(a)) * modulo;
    }

    float Oscilacao(const float t, const float amplitude) {
        if (t <= 0.0f) return 0.0f;
        return amplitude * std::exp(-t / kTempoDaOnda) * std::sin(2.0f * Math::Pi * kVoltasDaOnda * t);
    }

    int Notificacoes(const int doAluno, const int dela) {
        return std::max(0, doAluno / kNotificacoes - dela);
    }

    int Janela(const int k) {
        if (k <= 0) return 1;
        if (k == 1) return 2;
        if (k == 2) return 4;
        if (k == 3) return 5;
        if (k == 4) return 6;
        return 3 + (k - 5) % 4;   // a perda: metade, e o dente recomeca
    }

    std::array<bool, 8> Byte(Variacao::Sorteio& s) {
        std::vector<int> fios(8);
        std::iota(fios.begin(), fios.end(), 0);
        s.Embaralhar(fios);
        std::array<bool, 8> bits{};
        for (int i = 0; i < kBitsUm; ++i) bits[static_cast<size_t>(fios[static_cast<size_t>(i)])] = true;
        return bits;
    }
}
