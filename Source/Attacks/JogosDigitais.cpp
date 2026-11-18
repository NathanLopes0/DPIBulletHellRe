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

    float Sinal(const float x, const float seZero) {
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        return seZero;
    }
}

namespace JogosDigitais {

    Vector2 Rumo(const Vector2 eu, const Vector2 alvo) {
        Vector2 d = alvo - eu;
        if (d.Length() < 1e-3f) return Vector2::Zero;
        d.Normalize();
        return d;
    }

    Vector2 Fugir(const Vector2 eu, const Vector2 de, const Campo& c, const float margem) {
        Vector2 d = eu - de;
        if (d.Length() < 1e-3f) d = Vector2(1.0f, 0.0f);
        d.Normalize();
        const bool esquerda = eu.x <= c.x + margem, direita = eu.x >= c.x + c.w - margem;
        const bool cima = eu.y <= c.y + margem, baixo = eu.y >= c.y + c.h - margem;
        // Na parede, o que entraria nela sai: sobra o que escorrega.
        if ((esquerda && d.x < 0.0f) || (direita && d.x > 0.0f)) d.x = 0.0f;
        if ((cima && d.y < 0.0f) || (baixo && d.y > 0.0f)) d.y = 0.0f;
        if (d.Length() > 0.3f) {
            d.Normalize();
            return d;
        }
        // Sem para onde fugir de frente: anda pela parede, para o lado longe do perseguidor (ou, se
        // ele esta bem em frente, para o meio do campo); no canto, pela parede em que ele esta mais
        // longe.
        const float sx = Sinal(eu.x - de.x, c.x + c.w / 2.0f >= eu.x ? 1.0f : -1.0f);
        const float sy = Sinal(eu.y - de.y, c.y + c.h / 2.0f >= eu.y ? 1.0f : -1.0f);
        const Vector2 porX((sx > 0.0f ? !direita : !esquerda) ? sx : -sx, 0.0f);
        const Vector2 porY(0.0f, (sy > 0.0f ? !baixo : !cima) ? sy : -sy);
        const bool naParedeDosLados = esquerda || direita, naDeCimaOuDeBaixo = cima || baixo;
        if (naDeCimaOuDeBaixo && !naParedeDosLados) return porX;
        if (naParedeDosLados && !naDeCimaOuDeBaixo) return porY;
        return std::fabs(eu.x - de.x) > std::fabs(eu.y - de.y) ? porY : porX;
    }

    Vector2 Prender(const Vector2 p, const Campo& c, const float mx, const float my) {
        const float x0 = c.x + mx, x1 = c.x + c.w - mx, y0 = c.y + my, y1 = c.y + c.h - my;
        return Vector2(x0 <= x1 ? std::clamp(p.x, x0, x1) : c.x + c.w / 2.0f,
                       y0 <= y1 ? std::clamp(p.y, y0, y1) : c.y + c.h / 2.0f);
    }

    float DuracaoDa(const std::string& fase) {
        if (fase == RegrasTrocadas::kFase) return RegrasTrocadas::kDuracaoDaFase;
        if (fase == Lancamento::kFase) return Lancamento::kDuracaoDaFase;
        return kDuracaoDaFase;
    }
}

// ---- Fase 1: a interface ----

namespace HudHostil {

    NumeroDaAbertura NumeroEm(const float te, const Vector2 placa, const Vector2 professor, const float nota) {
        NumeroDaAbertura n;
        const Vector2 destino = professor + Vector2(0.0f, 120.0f);
        const auto suave = [](const float u) {
            const float k = std::clamp(u, 0.0f, 1.0f);
            return k * k * (3.0f - 2.0f * k);
        };
        // O arco: o meio do caminho sobe, como quem puxa o numero por um fio.
        const auto arco = [&](const Vector2 de, const Vector2 para, const float u) {
            const Vector2 meio = (de + para) * 0.5f + Vector2(0.0f, -160.0f);
            const float v = 1.0f - u;
            return de * (v * v) + meio * (2.0f * v * u) + para * (u * u);
        };
        const float t1 = kTremeNaPlaca, t2 = t1 + kVaiAteEle, t3 = t2 + kReescreve;
        if (te < t1) {
            n.momento = Momento::NaPlaca;
            n.onde = placa;
            n.valor = nota;
        } else if (te < t2) {
            const float u = suave((te - t1) / kVaiAteEle);
            n.momento = Momento::Indo;
            n.onde = arco(placa, destino, u);
            n.valor = nota;
            n.tamanho = 1.0f + (kTamanhoNoProfessor - 1.0f) * u;
        } else if (te < t3) {
            // Ele apaga digito a digito: a descida comeca devagar e acaba de vez.
            const float u = std::clamp((te - t2) / kReescreve, 0.0f, 1.0f);
            n.momento = Momento::Reescrevendo;
            n.onde = destino;
            n.valor = std::round(nota + (kNotaDaAbertura - nota) * (u * u));
            n.tamanho = kTamanhoNoProfessor;
        } else if (te < kNotaEditada) {
            const float u = suave((te - t3) / kVoltaAPlaca);
            n.momento = Momento::Voltando;
            n.onde = arco(destino, placa, u);
            n.valor = kNotaDaAbertura;
            n.tamanho = kTamanhoNoProfessor + (1.0f - kTamanhoNoProfessor) * u;
        } else {
            n.momento = Momento::Devolvido;
            n.onde = placa;
            n.valor = kNotaDaAbertura;
        }
        return n;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.passadasDeitada = 4;
        d.passadasDePe = 5;
        d.entreLeques = 0.95f;
        d.tirosDoLeque = 5;
        d.velocidade = 270.0f;
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

    std::vector<Caco> Cacos(const PoseDaBarra& p, const int segmentos, const float mostrada) {
        std::vector<Caco> cacos;
        const int n = std::max(1, segmentos);
        const int cheios = Cheios(mostrada, n);
        const Vector2 normal(-p.eixo.y, p.eixo.x);
        for (int k = 0; k < n; ++k) {
            const bool cheio = Cheio(k, n, mostrada, p.espelhada) && cheios > 0;
            const Vector2 centro = CentroDoSegmento(p, k, n);
            // Do meio da barra para as pontas: -1 a 1.
            const float aoLongo = n > 1 ? 2.0f * static_cast<float>(k) / static_cast<float>(n - 1) - 1.0f : 0.0f;
            for (int j = 0; j < kCacosPorSegmento; ++j) {
                const int i = k * kCacosPorSegmento + j;
                // Um passo fixo e primo entre os cacos: parece sorteado, e nao e.
                const float h = static_cast<float>((i * 37 + 11) % 23) / 22.0f;
                const float lado = (i % 2 == 0) ? 1.0f : -1.0f;
                Caco c;
                c.onde = centro + p.eixo * ((static_cast<float>(j) - 1.0f) * kSegmento / 3.0f);
                c.velocidade = normal * (lado * (90.0f + 150.0f * h)) + p.eixo * (aoLongo * 160.0f + (h - 0.5f) * 80.0f) +
                               Vector2(0.0f, -120.0f);
                c.giro = lado * (240.0f + 480.0f * h);
                c.lado = 6.0f + 6.0f * h;
                c.cheio = cheio;
                cacos.push_back(c);
            }
        }
        return cacos;
    }

    Vector2 OndeDoCaco(const Caco& c, const float t) {
        return c.onde + c.velocidade * t + Vector2(0.0f, 0.5f * kGravidadeDosCacos * t * t);
    }

    float NotaNaQuebra(const float antes, const float t) {
        if (QuebraAcabou(t)) return 0.0f;
        // Escorre devagar e despenca, e chega a quase nada antes do fim: a faixa, que anda atras da
        // nota, ja mostra zero quando a luta acaba.
        const float u = std::clamp(t / (kQuebraDaBarra * kEscorreAte), 0.0f, 1.0f);
        return std::max(0.01f, antes * (1.0f - u * u));
    }

}

// ---- Fase 1, a ultima etapa: a IA do inimigo ----

namespace IaDoInimigo {

    const char* Nome(const Estado e) {
        switch (e) {
            case Estado::Nascendo:  return "NASCENDO";
            case Estado::Patrulha:  return "PATRULHA";
            case Estado::Alerta:    return "ALERTA";
            case Estado::Persegue:  return "PERSEGUE";
            case Estado::Atordoado: return "ATORDOADO";
            case Estado::Fugindo:   return "FUGINDO";
            case Estado::Derrotado: return "DERROTADO";
        }
        return "";
    }

    namespace {
        std::vector<Vector2> Lugares(const char marca) {
            std::vector<Vector2> v;
            for (int j = 0; j < kLinhas; ++j) {
                for (int i = 0; i < kColunas; ++i) {
                    if (kCorpo[static_cast<size_t>(j)][i] != marca) continue;
                    v.emplace_back((static_cast<float>(i) - static_cast<float>(kColunas - 1) / 2.0f) * kBloco,
                                   (static_cast<float>(j) - static_cast<float>(kLinhas - 1) / 2.0f) * kBloco);
                }
            }
            return v;
        }

        /// O retangulo em que o centro dele pode estar.
        JogosDigitais::Campo Arena(const JogosDigitais::Campo& c) {
            return JogosDigitais::Campo{c.x, c.y + kTopoDaArena, c.w, c.h - kTopoDaArena};
        }
        constexpr float kFolgaDaArena = 6.0f;
    }

    std::vector<Vector2> LugaresDosBlocos() {
        return Lugares('#');
    }

    std::vector<Vector2> LugaresDosOlhos() {
        return Lugares('O');
    }

    bool NaBarra(const Vector2 lugar) {
        return lugar.y > kMeiaAltura - kBloco;
    }

    int Coluna(const Vector2 lugar) {
        const int c = static_cast<int>(std::lround(lugar.x / kBloco + static_cast<float>(kColunas - 1) / 2.0f));
        return std::clamp(c, 0, kColunas - 1);
    }

    float Onda(const int coluna, const float relogio) {
        // Um quarto de volta por coluna: os pes (nas colunas pares) ficam meia volta um do outro.
        return kOnda * std::sin(relogio * 9.0f + static_cast<float>(coluna) * 1.5707963f);
    }

    float Flutua(const float relogio) {
        return kFlutua * std::sin(relogio * 2.4f);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.persegue = 140.0f;
        d.aceleracao = 10.0f;
        d.maxima = 200.0f;
        d.foge = 210.0f;
        d.raioDeVisao = 280.0f;
        d.raioDePerder = 420.0f;
        d.susto = 3.6f;
        d.atordoado = 1.5f;
        d.primeiroPoder = 3.6f;
        d.entrePoderes = 3.0f;
        return d;
    }

    bool Passo(Ia& ia, const Percepcao& p, const float dt, const Degrau& g) {
        const Estado antes = ia.estado;
        const auto ir = [&](const Estado e) {
            ia.estado = e;
            ia.noEstado = 0.0f;
            ia.longe = 0.0f;
            ia.recuo = 0.0f;
        };
        // Sem corpo, nao ha inimigo. Depois os eventos: o power-up vence o olho, e o power-up de novo,
        // fugindo, renova o susto.
        const bool vivo = ia.estado != Estado::Nascendo && ia.estado != Estado::Derrotado;
        if (vivo && p.blocos <= 0) {
            ir(Estado::Derrotado);
            return true;
        }
        if (p.pegouOPoder && vivo) {
            if (ia.estado != Estado::Fugindo) ir(Estado::Fugindo);
            ia.susto = p.susto > 0.0f ? p.susto : g.susto;
            return ia.estado != antes;
        }
        if (p.olhoAtingido && (ia.estado == Estado::Patrulha || ia.estado == Estado::Alerta || ia.estado == Estado::Persegue)) {
            ir(Estado::Atordoado);
            return true;
        }
        if (p.acertouOAluno && (ia.estado == Estado::Alerta || ia.estado == Estado::Persegue)) {
            ir(Estado::Patrulha);
            ia.recuo = kRecuo;
            return true;
        }
        ia.noEstado += dt;
        switch (ia.estado) {
            case Estado::Nascendo:
                if (ia.noEstado >= kNascendo) ir(Estado::Patrulha);
                break;
            case Estado::Patrulha:
                ia.recuo = std::max(0.0f, ia.recuo - dt);
                if ((p.distancia <= g.raioDeVisao && ia.recuo <= 0.0f) || ia.noEstado >= kRondaCansa) ir(Estado::Alerta);
                break;
            case Estado::Alerta:
                if (ia.noEstado >= kAlerta) {
                    ir(Estado::Persegue);
                    ia.rapidez = g.persegue;
                }
                break;
            case Estado::Persegue:
                ia.rapidez = std::min(g.maxima, std::max(ia.rapidez, g.persegue) + g.aceleracao * dt);
                ia.longe = p.distancia > g.raioDePerder ? ia.longe + dt : 0.0f;
                if (ia.longe >= kParaPerder) ir(Estado::Patrulha);
                break;
            case Estado::Atordoado:
                if (ia.noEstado >= g.atordoado) {
                    ir(Estado::Persegue);
                    ia.rapidez = g.persegue;
                }
                break;
            case Estado::Fugindo:
                ia.susto = std::max(0.0f, ia.susto - dt);
                if (ia.susto <= 0.0f) {
                    ir(Estado::Persegue);
                    ia.rapidez = g.persegue;
                }
                break;
            case Estado::Derrotado:
                if (ia.noEstado >= kRespawn) ir(Estado::Nascendo);
                break;
        }
        return ia.estado != antes;
    }

    bool Machuca(const Estado e) {
        return e == Estado::Patrulha || e == Estado::Alerta || e == Estado::Persegue || e == Estado::Atordoado;
    }

    float Rapidez(const Ia& ia, const Degrau& g) {
        switch (ia.estado) {
            case Estado::Patrulha: return g.ronda;
            case Estado::Persegue: return std::max(ia.rapidez, g.persegue);
            case Estado::Fugindo:  return g.foge;
            default:               return 0.0f;
        }
    }

    bool Pisca(const Ia& ia, const float relogio) {
        return ia.estado == Estado::Fugindo && ia.susto <= kPiscaNoFim && std::fmod(relogio, 0.2f) < 0.1f;
    }

    Vector2 NaArena(const Vector2 centro, const JogosDigitais::Campo& c) {
        return JogosDigitais::Prender(centro, Arena(c), kMeiaLargura + kFolgaDaArena, kMeiaAltura + kFolgaDaArena);
    }

    std::array<Vector2, 4> Ronda(const JogosDigitais::Campo& c) {
        const JogosDigitais::Campo a = Arena(c);
        const float x0 = a.x + kMeiaLargura + kFolgaDaArena, x1 = a.x + a.w - kMeiaLargura - kFolgaDaArena;
        const float y0 = a.y + kMeiaAltura + kFolgaDaArena, y1 = a.y + a.h - kMeiaAltura - kFolgaDaArena;
        return {Vector2(x0, y0), Vector2(x1, y0), Vector2(x1, y1), Vector2(x0, y1)};
    }

    int Ninho(const JogosDigitais::Campo& c, const Vector2 aluno) {
        const auto cantos = Ronda(c);
        int longe = 0;
        for (int k = 1; k < 4; ++k) {
            if ((cantos[static_cast<size_t>(k)] - aluno).Length() > (cantos[static_cast<size_t>(longe)] - aluno).Length()) longe = k;
        }
        return longe;
    }

    Vector2 LugarDoPoder(Variacao::Sorteio& s, const JogosDigitais::Campo& c, const Vector2 inimigo) {
        const JogosDigitais::Campo a = Arena(c);
        const Vector2 meio(a.x + a.w / 2.0f, a.y + a.h / 2.0f);
        // O espelho dele pelo meio da arena, com um pouco de sorteio.
        const float dx = s.Real(-90.0f, 90.0f), dy = s.Real(-60.0f, 60.0f);
        const Vector2 lugar = JogosDigitais::Prender(meio * 2.0f - inimigo + Vector2(dx, dy), a, 40.0f, 30.0f);
        if ((lugar - inimigo).Length() >= 320.0f) return lugar;
        // Perto demais (ele estava no meio): o canto mais longe dele.
        const auto cantos = Ronda(c);
        Vector2 melhor = cantos[0];
        for (const Vector2& k : cantos) {
            if ((k - inimigo).Length() > (melhor - inimigo).Length()) melhor = k;
        }
        return melhor;
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

    float FpsAlvo(const float focados, const float teto) {
        const float f = std::clamp(focados, 0.0f, 1.0f);
        const float alto = std::max(teto, kFpsMinimo);
        return std::exp(std::log(alto) * (1.0f - f) + std::log(kFpsMinimo) * f);
    }

    float FocadosComFolga(Folga& f, const float agora, const float dt) {
        if (agora >= f.focados) {
            f.focados = agora;
            f.ha = 0.0f;
        } else {
            f.ha += dt;
            if (f.ha >= kFolgaAoSoltar) {
                f.focados = agora;
                f.ha = 0.0f;
            }
        }
        return f.focados;
    }

    bool Impulsiona(Arranque& a, const bool focadoAgora, const float dt) {
        if (focadoAgora) {
            a.focado += dt;
            return false;
        }
        const bool soltou = a.focado >= kFocoParaImpulso;
        a.focado = 0.0f;
        return soltou;
    }

    float PassoDoFps(const float fps, const float alvo, const float teto, const float dt) {
        const float alto = std::max(teto, kFpsMinimo + 1.0f);
        const float menor = std::log(kFpsMinimo), maior = std::log(alto), vao = maior - menor;
        float l = std::log(std::clamp(fps, kFpsMinimo, alto));
        const float quer = std::log(std::clamp(alvo, kFpsMinimo, alto));
        if (quer < l) l = std::max(quer, l - vao / kQuedaDoFps * dt);
        else l = std::min(quer, l + vao / kSubidaDoFps * dt);
        return std::exp(l);
    }

    float SemDeltaTime(const float fps) {
        return fps / kFpsDeReferencia;
    }

    float NoGrafico(const float fps, const float teto) {
        const float menor = std::log(kFpsMinimo), maior = std::log(std::max(teto, kFpsMinimo + 1.0f));
        return std::clamp((std::log(std::max(fps, 1.0f)) - menor) / (maior - menor), 0.0f, 1.0f);
    }

    std::array<bool, 7> Segmentos(const char c) {
        //                        a      b      c      d      e      f      g
        switch (c) {
            case '0': return {true,  true,  true,  true,  true,  true,  false};
            case '1': return {false, true,  true,  false, false, false, false};
            case '2': return {true,  true,  false, true,  true,  false, true};
            case '3': return {true,  true,  true,  true,  false, false, true};
            case '4': return {false, true,  true,  false, false, true,  true};
            case '5':
            case 'S': return {true,  false, true,  true,  false, true,  true};
            case '6': return {true,  false, true,  true,  true,  true,  true};
            case '7': return {true,  true,  true,  false, false, false, false};
            case '8': return {true,  true,  true,  true,  true,  true,  true};
            case '9': return {true,  true,  true,  true,  false, true,  true};
            case 'F': return {true,  false, false, false, true,  true,  true};
            case 'P': return {true,  true,  false, false, true,  true,  true};
            default:  return {false, false, false, false, false, false, false};
        }
    }

    float VaoDaParede(Variacao::Sorteio& s, const float anterior, const float de, const float ate, const float longe,
                      const float perto) {
        const float x0 = de + kVaoDaParede / 2.0f, x1 = ate - kVaoDaParede / 2.0f;
        if (x1 <= x0) return (de + ate) / 2.0f;
        if (anterior < 0.0f) return s.Real(x0, x1);
        // Um lado ou o outro do anterior, a uma distancia entre 'longe' e 'perto' - o lado que nao
        // cabe no campo cede para o outro.
        const float minimo = std::max(0.0f, longe), maximo = std::max(minimo, perto);
        const bool cabeADireita = anterior + minimo <= x1, cabeAEsquerda = anterior - minimo >= x0;
        float sentido = s.Real(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
        if (sentido > 0.0f && !cabeADireita) sentido = -1.0f;
        if (sentido < 0.0f && !cabeAEsquerda) sentido = 1.0f;
        const float x = anterior + sentido * s.Real(minimo, maximo);
        return std::clamp(x, x0, x1);
    }

    float VaoAlcancavel(const float velocidade, const float entreParedes, const float longe) {
        return std::max(longe + 40.0f, 0.85f * velocidade * entreParedes);
    }

    Humor HumorDoSave(const float salva, const float agora) {
        if (salva > agora + kEmpate) return Humor::Foge;
        if (salva < agora - kEmpate) return Humor::Persegue;
        return Humor::Parado;
    }

    float RapidezDoSave(const float salva, const float agora) {
        if (HumorDoSave(salva, agora) == Humor::Parado) return 0.0f;
        return std::min(kSaveRapido, kSaveDevagar + kSavePorPonto * std::fabs(salva - agora));
    }

    bool Encostou(const Vector2 save, const Vector2 aluno, const float raioDoAluno) {
        return (save - aluno).Length() <= kRaioDoSave + raioDoAluno;
    }

    bool Encurralado(const Vector2 eu, const Vector2 aluno, const JogosDigitais::Campo& area) {
        const bool nosLados = eu.x <= area.x + kNoCanto || eu.x >= area.x + area.w - kNoCanto;
        const bool emCimaOuEmbaixo = eu.y <= area.y + kNoCanto || eu.y >= area.y + area.h - kNoCanto;
        return nosLados && emCimaOuEmbaixo && (eu - aluno).Length() <= kPertoDoCanto;
    }

    Vector2 PassoDoSave(Fuga& f, const Humor h, const Vector2 eu, const Vector2 aluno, const JogosDigitais::Campo& area,
                        const float rapidez, const int tiros, const float dt) {
        f.recarga = std::max(0.0f, f.recarga - dt);
        f.imune = std::max(0.0f, f.imune - dt);
        // O tiro: o drive engasga - cada tiro alonga o engasgo, ate kEngasgoMaximo desde que ele
        // comecou -, a nao ser logo depois de um engasgo.
        if (tiros > 0 && f.imune <= 0.0f) {
            f.engasgo = std::min(kEngasgoMaximo - f.engasgado, f.engasgo + kEngasgo * static_cast<float>(tiros));
        }
        if (f.engasgo > 0.0f) {
            f.engasgo -= dt;
            f.engasgado += dt;
            if (f.engasgo <= 0.0f) {
                f.engasgo = 0.0f;
                f.engasgado = 0.0f;
                f.imune = kImuneAoTiro;
            }
            return Vector2::Zero;
        }
        if (h == Humor::Foge) {
            if (f.travado > 0.0f) {
                f.travado = std::max(0.0f, f.travado - dt);
                if (f.travado <= 0.0f) f.recarga = kRecargaDoCanto;
                return Vector2::Zero;
            }
            if (f.recarga <= 0.0f && Encurralado(eu, aluno, area)) {
                f.travado = kEncurralado;
                return Vector2::Zero;
            }
            return JogosDigitais::Fugir(eu, aluno, area, 12.0f) * (rapidez * dt);
        }
        f.travado = 0.0f;
        if (h == Humor::Persegue) return JogosDigitais::Rumo(eu, aluno) * (rapidez * dt);
        return Vector2::Zero;
    }

    bool Treme(const Fuga& f) {
        return f.travado > 0.0f || f.engasgo > 0.0f;
    }

    bool HoraDeSalvar(const float te, const float ultimoLoad) {
        if (te < kPrimeiroSave || te > kUltimoSave) return false;
        return ultimoLoad < 0.0f || te >= ultimoLoad + kEntreSaves;
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
        d.fpsMaximo = 240.0f;
        d.entreEspirais = 0.1f;
        d.giroDaEspiral = 15.0f;
        d.velocidadeVermelha = 130.0f;
        d.entreParedes = 1.9f;
        d.velocidadeAzul = 235.0f;
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

    float EntrePassos(const float base, const int vivos, const int total, const int onda) {
        const float u = total > 0 ? std::clamp(static_cast<float>(vivos) / static_cast<float>(total), 0.0f, 1.0f) : 1.0f;
        return base * (0.4f + 0.6f * u) * std::pow(0.8f, static_cast<float>(std::max(0, onda)));
    }

    int LugarDoHotfix(const std::vector<int>& vazios, Variacao::Sorteio& s) {
        if (vazios.empty()) return -1;
        return vazios[static_cast<size_t>(s.Inteiro(0, static_cast<int>(vazios.size()) - 1))];
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
        d.entreQuedas = 0.3f;
        d.velocidadeDaQueda = 270.0f;
        d.entreLinhas = 0.95f;
        d.subida = 92.0f;
        d.hotfix = 0.85f;
        d.janela = 2.8f;
        d.entreLequesDaFrota = 1.25f;
        d.tirosDoLequeDaFrota = 5;
        d.entreAneisDaHitbox = 0.9f;
        d.tirosDoAnelDaHitbox = 16;
        d.velocidadeDaHitbox = 205.0f;
        d.entreLequesDaHitbox = 1.2f;
        d.tirosDoLequeDaHitbox = 5;
        return d;
    }
}
