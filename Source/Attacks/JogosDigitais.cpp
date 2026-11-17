//
// A INF 216 do Lucas Ferreira: as quatro provas que mexem nos sistemas do proprio jogo.
//

#include "JogosDigitais.h"

#include <algorithm>
#include <cmath>

#include "../Regras/Nota.h"

namespace JogosDigitais {

    int Etapa(const std::array<float, 3>& d, const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < d.size(); ++k) {
            fim += d[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(d.size()) - 1;
    }

    float NaEtapa(const std::array<float, 3>& d, const float t) {
        float inicio = 0.0f;
        const int etapa = Etapa(d, t);
        for (int k = 0; k < etapa; ++k) inicio += d[static_cast<size_t>(k)];
        return std::max(0.0f, t - inicio);
    }
}

namespace HudHostil {

    int Cheios(const float nota, const int segmentos) {
        if (segmentos <= 0) return 0;
        const float fracao = std::clamp(nota, 0.0f, 100.0f) / 100.0f;
        return std::clamp(static_cast<int>(std::floor(fracao * static_cast<float>(segmentos) + 1e-3f)), 0, segmentos);
    }

    bool Cheio(const int k, const int segmentos, const float nota, const bool espelhada) {
        if (k < 0 || k >= segmentos) return false;
        const int cheios = Cheios(nota, segmentos);
        return espelhada ? k >= segmentos - cheios : k < cheios;
    }

    Varredura NaVarredura(const float t, const float ida) {
        Varredura v;
        if (t < kAvisoDaBarra || ida <= 0.0f) return v;
        const float ciclo = ida + kPausaDaBarra;
        const float s = t - kAvisoDaBarra;
        v.passada = static_cast<int>(std::floor(s / ciclo));
        const float dentro = s - static_cast<float>(v.passada) * ciclo;
        v.parada = dentro >= ida;
        const float u = std::min(1.0f, dentro / ida);
        v.onde = v.passada % 2 == 0 ? u : 1.0f - u;
        // No descanso ela ja mostra o lado da passada seguinte.
        v.espelhada = (v.passada + (v.parada ? 1 : 0)) % 2 == 1;
        return v;
    }

    const char* CaraDaBarra(const float nota) {
        if (nota < Nota::kNotaDeExame) return "Vermelha";
        if (nota < Nota::kNotaAprovacao) return "Amarela";
        return "Verde";
    }

    std::vector<Vector2> Moldura(const Vector2 centro) {
        std::vector<Vector2> pecas;
        const float meiaL = kLarguraDaJanela / 2.0f, meiaA = kAlturaDaJanela / 2.0f;
        const Vector2 x = BotaoDeFechar(centro);
        for (float dx = -meiaL; dx <= meiaL + 0.5f; dx += kPecaDaJanela) {
            const Vector2 cima(centro.x + dx, centro.y - meiaA);
            if (std::fabs(cima.x - x.x) > kPecaDaJanela / 2.0f) pecas.push_back(cima);
            pecas.emplace_back(centro.x + dx, centro.y + meiaA);
        }
        for (float dy = -meiaA + kPecaDaJanela; dy <= meiaA - kPecaDaJanela + 0.5f; dy += kPecaDaJanela) {
            pecas.emplace_back(centro.x - meiaL, centro.y + dy);
            pecas.emplace_back(centro.x + meiaL, centro.y + dy);
        }
        return pecas;
    }

    Vector2 BotaoDeFechar(const Vector2 centro) {
        return Vector2(centro.x + kLarguraDaJanela / 2.0f - kRecuoDoFechar, centro.y - kAlturaDaJanela / 2.0f);
    }

    Vector2 CentroDaJanela(const Vector2 aluno, const float esquerda, const float topo, const float direita, const float fundo) {
        const float meiaL = kLarguraDaJanela / 2.0f + 14.0f, meiaA = kAlturaDaJanela / 2.0f + 6.0f;
        return Vector2(std::clamp(aluno.x, esquerda + meiaL, std::max(esquerda + meiaL, direita - meiaL)),
                       std::clamp(aluno.y, topo + meiaA, std::max(topo + meiaA, fundo - meiaA)));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.idaDaBarra = 2.4f;
        d.entreLeques = 0.95f;
        d.tirosDoLeque = 5;
        d.velocidade = 270.0f;
        d.entreJanelas = 2.0f;
        return d;
    }
}

namespace RegrasTrocadas {

    Regra RegraEm(const float t) {
        if (t < kComecaAInversao) return Regra::Aviso;
        if (t < kVoltaEm) return Regra::Invertida;
        if (t < kVoltou) return Regra::Voltando;
        return Regra::Normal;
    }

    bool TiroEMoeda(const Regra r) {
        return r == Regra::Invertida || r == Regra::Voltando;
    }

    bool AcertoCobra(const Regra r) {
        return TiroEMoeda(r);
    }

    bool MoedaPisca(const Regra r, const float t) {
        return r == Regra::Voltando && static_cast<int>(std::floor(t / 0.12f)) % 2 == 0;
    }

    Vector2 Lancar(const Vector2 de, const Vector2 alvo, const float voo, const float g) {
        const float t = std::max(voo, 0.05f);
        return Vector2((alvo.x - de.x) / t, (alvo.y - de.y) / t - 0.5f * g * t);
    }

    Vector2 NoArco(const Vector2 de, const Vector2 v, const float t, const float g) {
        return Vector2(de.x + v.x * t, de.y + v.y * t + 0.5f * g * t * t);
    }

    bool SaiuPorCima(const float y, const float topo) {
        return y < topo + kFaixaDeSaida;
    }

    bool SaiuPorBaixo(const float y, const float fundo) {
        return y > fundo + 6.0f;
    }

    Vector2 OndeVolta(const float x, const float fundo) {
        return Vector2(x, fundo + 10.0f);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreAneis = 0.8f;
        d.tirosDoAnel = 18;
        d.velocidade = 220.0f;
        d.entreLeques = 1.2f;
        d.tirosDoLeque = 7;
        d.entreArremessos = 0.8f;
        d.arremessos = 5;
        d.entreTiros = 0.65f;
        return d;
    }
}

namespace MotorQuebrado {

    int QuadroEm(const float t) {
        return static_cast<int>(std::floor(std::max(0.0f, t) / kQuadroDoLag));
    }

    Vector2 NoQuadro(const Vector2 p0, const Vector2 v, const int quadros) {
        return p0 + v * (kQuadroDoLag * static_cast<float>(quadros));
    }

    Vector2 Descolamento(const float t) {
        const float abre = std::clamp(t / kAbreODescolamento, 0.0f, 1.0f);
        const float a = Math::ToRadians(kGiroDoDescolamento * std::max(0.0f, t));
        return Vector2(std::cos(a), std::sin(a)) * (kDescolamento * abre);
    }

    Evento EventoEntre(const float antes, const float agora) {
        for (const float s : kSaves) if (antes < s && s <= agora) return Evento::Salvar;
        for (const float l : kLoads) if (antes < l && l <= agora) return Evento::Carregar;
        return Evento::Nada;
    }

    float RumoAoLoad(const float t) {
        for (size_t k = 0; k < kSaves.size(); ++k) {
            if (t >= kSaves[k] && t < kLoads[k]) return (t - kSaves[k]) / (kLoads[k] - kSaves[k]);
        }
        return -1.0f;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreAneis = 0.9f;
        d.tirosDoAnel = 16;
        d.velocidade = 205.0f;
        d.entreLeques = 1.2f;
        d.tirosDoLeque = 5;
        return d;
    }
}

namespace Lancamento {

    int TravadaEm(const float t) {
        for (size_t k = 0; k < kTravadas.size(); ++k) {
            if (t >= kTravadas[k] && t < kTravadas[k] + kEspera) return static_cast<int>(k);
        }
        return -1;
    }

    float FatorDaVolta(const Escolha e) {
        return e == Escolha::Nenhuma ? 1.6f : 1.0f;
    }

    int LadoDoEsperar(const int k) {
        return ((k % 2) + 2) % 2;
    }

    Marcha MarchaEm(const int passos, const float folga) {
        Marcha m;
        float lado = 1.0f;
        for (int i = 0; i < passos; ++i) {
            if (std::fabs(m.dx + lado * kPassoDaFrota) > folga) {
                ++m.descidas;
                lado = -lado;
            } else {
                m.dx += lado * kPassoDaFrota;
            }
        }
        return m;
    }

    Vector2 LugarNaFrota(const int fileira, const int coluna, const Vector2 centroDoTopo, const Marcha& m) {
        const float meio = static_cast<float>(kColunasDaFrota - 1) / 2.0f;
        return Vector2(centroDoTopo.x + (static_cast<float>(coluna) - meio) * kEspacoX + m.dx,
                       centroDoTopo.y + static_cast<float>(fileira) * kEspacoY + static_cast<float>(m.descidas) * kDescidaDaFrota);
    }

    int VaoDaLinha(const int blocos, Variacao::Sorteio& s) {
        const int meio = blocos / 2;
        const int fimDaEsquerda = meio - kFaixaDoTexto - kVaoDosCreditos;
        const int comecoDaDireita = meio + kFaixaDoTexto;
        const int fimDaDireita = blocos - kVaoDosCreditos - 1;
        const bool temEsquerda = fimDaEsquerda >= 1, temDireita = fimDaDireita >= comecoDaDireita;
        if (!temEsquerda && !temDireita) return 0;
        const bool esquerda = temEsquerda && (!temDireita || s.Chance(0.5f));
        return esquerda ? s.Inteiro(1, fimDaEsquerda) : s.Inteiro(comecoDaDireita, fimDaDireita);
    }

    bool NoVao(const int bloco, const int inicio) {
        return bloco >= inicio && bloco < inicio + kVaoDosCreditos;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreAneis = 0.75f;
        d.tirosDoAnel = 20;
        d.velocidade = 215.0f;
        d.entrePassos = 0.24f;
        d.entreQuedas = 0.4f;
        d.velocidadeDaQueda = 270.0f;
        d.entreLinhas = 1.4f;
        d.subida = 92.0f;
        return d;
    }
}
