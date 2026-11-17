//
// A INF 216 do Lucas Ferreira: as quatro provas que mexem nos sistemas do proprio jogo.
//

#include "JogosDigitais.h"

#include <algorithm>
#include <cmath>

namespace {

    float Mistura(const float a, const float b, const float u) {
        return a + (b - a) * u;
    }

    /// O vai e vem que acelera e freia: comeca e termina parado.
    float Suave(const float u) {
        const float x = std::clamp(u, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    Vector2 EixoDe(const float angulo) {
        const float a = Math::ToRadians(angulo);
        return Vector2(std::cos(a), std::sin(a));
    }
}

// ---- Fase 1: a interface ----

namespace HudHostil {

    std::array<AnelDaAbertura, kAneisDaAbertura> AneisDaAbertura(const Dificuldade::Nivel n) {
        if (n == Dificuldade::Nivel::Dificil) {
            return {{{0.15f, 300.0f, 720.0f, 44, 4}, {0.85f, 320.0f, 770.0f, 46, 4}, {1.55f, 340.0f, 820.0f, 48, 4}}};
        }
        return {{{0.15f, 300.0f, 620.0f, 44, 5}, {0.95f, 320.0f, 660.0f, 46, 5}, {1.75f, 340.0f, 700.0f, 48, 5}}};
    }

    float AnguloNoAnel(const int i, const int tiros) {
        if (tiros <= 0) return 0.0f;
        return -180.0f + 360.0f * static_cast<float>(i) / static_cast<float>(tiros);
    }

    float VaoDaAbertura(Variacao::Sorteio& s, const float anterior, const float rumo) {
        // A distancia entre dois angulos, dando a volta.
        const auto longe = [](const float a, const float b) {
            float d = std::fmod(a - b + 540.0f, 360.0f);
            if (d < 0.0f) d += 360.0f;
            return std::fabs(d - 180.0f);
        };
        float v = rumo + s.Real(-kAberturaDoVao, kAberturaDoVao);
        if (anterior < -900.0f) return v;
        for (int k = 0; k < 12 && longe(v, anterior) < 50.0f; ++k) v = rumo + s.Real(-kAberturaDoVao, kAberturaDoVao);
        // Sem sorte: a ponta do leque que fica mais longe do anterior.
        if (longe(v, anterior) < 50.0f) v = rumo + (longe(rumo + 55.0f, anterior) > longe(rumo - 55.0f, anterior) ? 55.0f : -55.0f);
        return v;
    }

    bool NoVao(const int i, const int tiros, const float vao, const int largura) {
        if (tiros <= 0 || largura <= 0) return false;
        const float passo = 360.0f / static_cast<float>(tiros);
        const int centro = ((static_cast<int>(std::lround((vao + 180.0f) / passo)) % tiros) + tiros) % tiros;
        int d = ((i - centro) % tiros + tiros) % tiros;
        if (d > tiros / 2) d -= tiros;
        const int de = -(largura - 1) / 2;
        return d >= de && d < de + largura;
    }

    float Fechamento(const AnelDaAbertura& a) {
        return a.velocidade > 0.0f ? a.raio / a.velocidade : 0.0f;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.passadasDeitada = 4;
        d.passadasDePe = 5;
        d.entreLeques = 0.95f;
        d.tirosDoLeque = 5;
        d.velocidade = 270.0f;
        d.entreJanelas = 2.0f;
        return d;
    }

    namespace {
        constexpr float kComecoDeitada = kTremorDaBarra + kArrancadaDaBarra + kRespiroDaBarra;
        constexpr float kComecoDePe = kGiroDaBarra + kRespiroDaBarra;

        float Ida(const float de, const float ate, const int passadas) {
            const int n = std::max(1, passadas);
            return std::max(0.2f, (ate - de - static_cast<float>(n - 1) * kPausaDaBarra) / static_cast<float>(n));
        }
    }

    float IdaDeitada(const Degrau& g) {
        return Ida(kComecoDeitada, kDuracoes[kEtapaDaBarraDeitada] - kRespiroDaBarra, g.passadasDeitada);
    }

    float IdaDePe(const Degrau& g) {
        return Ida(kComecoDePe, kDuracoes[kEtapaDaBarraDePe] - kVoltaDaBarra - kRespiroDaBarra, g.passadasDePe);
    }

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
        if (t < 0.0f || ida <= 0.0f) return v;
        const float ciclo = ida + kPausaDaBarra;
        v.passada = static_cast<int>(std::floor(t / ciclo));
        const float dentro = t - static_cast<float>(v.passada) * ciclo;
        v.parada = dentro >= ida;
        const float u = std::min(1.0f, dentro / ida);
        v.onde = v.passada % 2 == 0 ? u : 1.0f - u;
        // No descanso ela ja mostra o lado da passada seguinte.
        v.espelhada = (v.passada + (v.parada ? 1 : 0)) % 2 == 1;
        return v;
    }

    PoseDaBarra Pose(const int etapa, const float te, const JogosDigitais::Campo& c, const Vector2 encaixe,
                     const float noEncaixe, const Degrau& g) {
        PoseDaBarra p;
        const float meio = c.x + c.w / 2.0f;
        const Vector2 alto(meio, c.y + kRecuoDaBarra);
        const Vector2 baixo(meio, c.y + c.h - kRecuoDaBarra);
        const Vector2 esquerda(c.x + kRecuoDaBarra, c.y + c.h / 2.0f);
        const Vector2 direita(c.x + c.w - kRecuoDaBarra, c.y + c.h / 2.0f);

        if (etapa == kEtapaDaBarraDeitada) {
            if (te < kTremorDaBarra) {
                // Ainda no rodape, tremendo cada vez mais.
                p.tremor = std::clamp(te / kTremorDaBarra, 0.0f, 1.0f);
                p.centro = encaixe + Vector2(std::sin(te * 61.0f) * 3.0f, std::sin(te * 47.0f) * 2.0f) * p.tremor;
                p.comprimento = noEncaixe;
                p.noEncaixe = true;
                return p;
            }
            if (te < kTremorDaBarra + kArrancadaDaBarra) {
                // Arrancada: sobe o campo inteiro ate o alto, esticando ate a largura dele.
                const float u = Suave((te - kTremorDaBarra) / kArrancadaDaBarra);
                p.centro = Vector2::Lerp(encaixe, alto, u);
                p.comprimento = Mistura(noEncaixe, c.w, u);
                return p;
            }
            // A varredura: a passada 0 desce do alto.
            const Varredura v = NaVarredura(te - kComecoDeitada, IdaDeitada(g));
            p.centro = Vector2::Lerp(alto, baixo, Suave(v.onde));
            p.comprimento = c.w;
            p.espelhada = v.espelhada;
            p.perigo = true;
            return p;
        }

        // De pe: comeca de onde a deitada parou, e a primeira passada enche do mesmo fim - o giro nao
        // vira o desenho do avesso.
        const PoseDaBarra fim = Pose(kEtapaDaBarraDeitada, kDuracoes[kEtapaDaBarraDeitada], c, encaixe, noEncaixe, g);
        const int deslocamento = fim.espelhada ? 1 : 0;
        if (te < kGiroDaBarra) {
            const float u = Suave(te / kGiroDaBarra);
            p.centro = Vector2::Lerp(fim.centro, esquerda, u);
            p.comprimento = Mistura(fim.comprimento, c.h, u);
            p.angulo = Mistura(0.0f, -90.0f, u);
            p.espelhada = fim.espelhada;
            p.eixo = EixoDe(p.angulo);
            return p;
        }
        const float idaDePe = IdaDePe(g);
        const float voltaEm = kDuracoes[kEtapaDaBarraDePe] - kVoltaDaBarra;
        const Varredura v = NaVarredura(std::min(te, voltaEm) - kComecoDePe, idaDePe);
        const Vector2 aqui = Vector2::Lerp(esquerda, direita, Suave(v.onde));
        const bool espelhada = (v.espelhada ? 1 : 0) != deslocamento ? true : false;
        if (te < voltaEm) {
            p.centro = aqui;
            p.comprimento = c.h;
            p.angulo = -90.0f;
            p.espelhada = espelhada;
            p.perigo = true;
            p.eixo = EixoDe(p.angulo);
            return p;
        }
        // A volta para o rodape. Espelhada, ela gira para o outro lado: a meia volta desfaz o espelho.
        const float u = Suave((te - voltaEm) / kVoltaDaBarra);
        p.centro = Vector2::Lerp(aqui, encaixe, u);
        p.comprimento = Mistura(c.h, noEncaixe, u);
        p.angulo = Mistura(-90.0f, espelhada ? -180.0f : 0.0f, u);
        p.espelhada = espelhada;
        p.noEncaixe = u >= 1.0f;
        p.eixo = EixoDe(p.angulo);
        return p;
    }

    Vector2 CentroDoSegmento(const PoseDaBarra& p, const int k, const int segmentos) {
        const float sobra = (p.comprimento - static_cast<float>(segmentos) * kSegmento) / 2.0f;
        const float ao = -p.comprimento / 2.0f + sobra + (static_cast<float>(k) + 0.5f) * kSegmento;
        return p.centro + p.eixo * ao;
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
}

// ---- Fase 2: a fisica ----

namespace Gravidade {

    using Plataforma::Lado;

    Desafio DesafioDo(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return Desafio::Pulos;
            case Lado::Esquerda: return Desafio::Lancas;
            case Lado::Cima:     return Desafio::Chuva;
            case Lado::Direita:  return Desafio::Esteira;
        }
        return Desafio::Pulos;
    }

    std::array<Lado, 4> Ordem(Variacao::Sorteio& s) {
        std::vector<Lado> resto = {Lado::Esquerda, Lado::Cima, Lado::Direita};
        s.Embaralhar(resto);
        return {Lado::Baixo, resto[0], resto[1], resto[2]};
    }

    Lado Oposto(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return Lado::Cima;
            case Lado::Esquerda: return Lado::Direita;
            case Lado::Cima:     return Lado::Baixo;
            case Lado::Direita:  return Lado::Esquerda;
        }
        return Lado::Cima;
    }

    float Comprimento(const Lado l, const JogosDigitais::Campo& c) {
        return (l == Lado::Baixo || l == Lado::Cima) ? c.w : c.h;
    }

    float Profundidade(const Lado l, const JogosDigitais::Campo& c, const float chao) {
        return ((l == Lado::Baixo || l == Lado::Cima) ? c.h : c.w) - 2.0f * chao;
    }

    Vector2 Ponto(const Lado l, const JogosDigitais::Campo& c, const float chao, const float u, const float h) {
        switch (l) {
            case Lado::Baixo:    return Vector2(c.x + u, c.y + c.h - chao - h);
            case Lado::Esquerda: return Vector2(c.x + chao + h, c.y + u);
            case Lado::Cima:     return Vector2(c.x + c.w - u, c.y + chao + h);
            case Lado::Direita:  return Vector2(c.x + c.w - chao - h, c.y + c.h - u);
        }
        return Vector2(c.x + u, c.y + c.h - chao - h);
    }

    Vector2 NoChao(const Lado l, const JogosDigitais::Campo& c, const float chao, const Vector2 p) {
        switch (l) {
            case Lado::Baixo:    return Vector2(p.x - c.x, c.y + c.h - chao - p.y);
            case Lado::Esquerda: return Vector2(p.y - c.y, p.x - c.x - chao);
            case Lado::Cima:     return Vector2(c.x + c.w - p.x, p.y - c.y - chao);
            case Lado::Direita:  return Vector2(c.y + c.h - p.y, c.x + c.w - chao - p.x);
        }
        return Vector2(p.x - c.x, c.y + c.h - chao - p.y);
    }

    float AlturaDaPeca(const int j, const float pes) {
        return -pes + kPeca / 2.0f + kPeca * static_cast<float>(j);
    }

    bool Pega(const float h, const float hPeca, const float raio, const float raioDoAluno) {
        return std::fabs(h - hPeca) < raio + raioDoAluno;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entrePilhasDe = 0.7f;
        d.entrePilhasAte = 1.05f;
        d.pilhaDe = 350.0f;
        d.pilhaAte = 420.0f;
        d.entreLancasDe = 0.5f;
        d.entreLancasAte = 0.72f;
        d.lanca = 620.0f;
        d.entreQuedasDe = 0.45f;
        d.entreQuedasAte = 0.7f;
        d.queda = 420.0f;
        d.fagulha = 360.0f;
        d.esteira = 165.0f;
        d.entreBuracosDe = 0.75f;
        d.entreBuracosAte = 1.15f;
        d.entreLeques = 1.7f;
        d.tirosDoLeque = 5;
        d.velocidade = 240.0f;
        return d;
    }

    std::vector<Pilha> Pulos(Variacao::Sorteio& s, const Degrau& g) {
        std::vector<Pilha> pilhas;
        float t = kComecaODesafio;
        while (t < kAcabaODesafio) {
            Pilha p;
            p.quando = t;
            const float r = s.Real(0.0f, 1.0f);
            p.altura = r < 0.5f ? 2 : (r < 0.8f ? 3 : 5);
            p.largura = s.Chance(0.25f) ? 2 : 1;
            p.sentido = s.Chance(0.6f) ? -1.0f : 1.0f;
            p.velocidade = s.Real(g.pilhaDe, g.pilhaAte);
            pilhas.push_back(p);
            float vao = s.Real(g.entrePilhasDe, g.entrePilhasAte);
            // Depois da alta ou da larga, o pulo longo precisa de tempo para descer.
            if (p.altura >= 5 || p.largura >= 2) vao = std::max(vao, 1.15f);
            t += vao;
        }
        return pilhas;
    }

    std::vector<Lanca> Lancas(Variacao::Sorteio& s, const Degrau& g) {
        std::vector<Lanca> lancas;
        float t = kComecaODesafio + kAvisoDaLanca;
        while (t < kAcabaODesafio) {
            Lanca l;
            l.quando = t;
            l.sentido = s.Chance(0.5f) ? 1.0f : -1.0f;
            l.h = s.Chance(0.6f) ? kLancaBaixa : kLancaAlta;
            l.velocidade = g.lanca * s.Real(0.92f, 1.08f);
            lancas.push_back(l);
            // As vezes a dupla: a mesma ponta, a outra altura, logo atras.
            if (s.Chance(0.25f)) {
                Lanca d = l;
                d.quando = t + 0.32f;
                d.h = l.h == kLancaBaixa ? kLancaAlta : kLancaBaixa;
                lancas.push_back(d);
                t += 0.32f;
            }
            t += s.Real(g.entreLancasDe, g.entreLancasAte);
        }
        return lancas;
    }

    std::vector<Queda> Chuva(Variacao::Sorteio& s, const Degrau& g, const float comprimento) {
        std::vector<Queda> quedas;
        float t = kComecaODesafio;
        float antes = -1000.0f;
        const float de = 110.0f, ate = std::max(de + 1.0f, comprimento - 110.0f);
        // A ultima cai a tempo de estilhacar antes do fim do lado.
        while (t < kAcabaODesafio - 0.8f) {
            Queda q;
            q.quando = t;
            q.moeda = s.Chance(0.3f);
            q.u = s.Real(de, ate);
            for (int k = 0; k < 8 && std::fabs(q.u - antes) < 180.0f; ++k) q.u = s.Real(de, ate);
            antes = q.u;
            quedas.push_back(q);
            t += s.Real(g.entreQuedasDe, g.entreQuedasAte);
        }
        return quedas;
    }

    std::vector<Buraco> Buracos(Variacao::Sorteio& s, const Degrau& g) {
        std::vector<Buraco> buracos;
        float t = kComecaODesafio;
        while (t < kAcabaODesafio - 1.0f) {
            Buraco b;
            b.quando = t;
            b.espinhos = s.Inteiro(2, 4);
            buracos.push_back(b);
            t += s.Real(g.entreBuracosDe, g.entreBuracosAte);
        }
        return buracos;
    }
}

// ---- Fase 3: as regras ----

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

    bool SaiuPorCima(const float y, const float topo) {
        return y < topo + kFaixaDeSaida;
    }

    bool SaiuPorBaixo(const float y, const float fundo) {
        return y > fundo + 6.0f;
    }

    Vector2 OndeVolta(const float x, const float fundo) {
        return Vector2(x, fundo + 10.0f);
    }

    int QuadroEm(const float t) {
        return static_cast<int>(std::floor(std::max(0.0f, t) / kQuadroDoLag));
    }

    Vector2 NoQuadro(const Vector2 p0, const Vector2 v, const int quadros) {
        return p0 + v * (kQuadroDoLag * static_cast<float>(quadros));
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
        d.entreAneis = 0.8f;
        d.tirosDoAnel = 18;
        d.velocidade = 220.0f;
        d.entreLeques = 1.2f;
        d.tirosDoLeque = 7;
        d.entreTiros = 0.65f;
        d.entreAneisDoMotor = 0.9f;
        d.tirosDoAnelDoMotor = 16;
        d.velocidadeDoMotor = 205.0f;
        d.entreLequesDoMotor = 1.2f;
        d.tirosDoLequeDoMotor = 5;
        return d;
    }
}

// ---- A repescagem: o lancamento ----

namespace Lancamento {

    Vector2 Descolamento(const float t) {
        const float abre = std::clamp(t / kAbreODescolamento, 0.0f, 1.0f);
        const float a = Math::ToRadians(kGiroDoDescolamento * std::max(0.0f, t));
        return Vector2(std::cos(a), std::sin(a)) * (kDescolamento * abre);
    }

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
        d.entreLinhas = 0.95f;
        d.subida = 92.0f;
        d.entreAneisDaHitbox = 0.9f;
        d.tirosDoAnelDaHitbox = 16;
        d.velocidadeDaHitbox = 205.0f;
        d.entreLequesDaHitbox = 1.2f;
        d.tirosDoLequeDaHitbox = 5;
        return d;
    }
}
