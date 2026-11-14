//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#include "Poder.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../Attacks/Agrupamento.h"

namespace Poder {

    Descricao Da(const Golpe::Materia m) {
        Descricao d;
        switch (m) {
            case Golpe::Materia::Andre:
                d = {Tipo::Matriz, "for (i) for (j)", "laços aninhados: 5 linhas de 8 tiros",
                     static_cast<float>(kLinhasDaMatriz) * kEntreLinhas + 0.6f, 0.0f,
                     kLinhasDaMatriz * kColunasDaMatriz, kEntreCelulas};
                break;
            case Golpe::Materia::Salles:
                d = {Tipo::Pilha, "push() e pop()", "a capivara empilha os tiros e devolve", 5.0f, 95.0f, 0,
                     kEntrePops};
                break;
            case Golpe::Materia::Ricardo:
                d = {Tipo::Congelar, "HLT", "o clock parou: o Hugo congela", 2.8f, 0.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Thiago:
                d = {Tipo::Transacao, "BEGIN", "se a nota cair até o fim, ROLLBACK", 6.0f, 0.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Luizinho:
                d = {Tipo::TryCatch, "try { } catch", "o acerto não tira nota: vira um leque", 3.0f, 0.0f, 5,
                     0.0f};
                break;
            case Golpe::Materia::Vladimir:
                d = {Tipo::Converter, "o autômato rejeita", "os tiros em volta viram seus", 0.0f, 240.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Julio:
                d = {Tipo::Agentes, "agentes autônomos", "os tiros caçam o alvo", 5.0f, 70.0f, 3, 0.0f};
                break;
            case Golpe::Materia::MariaLucia:
                d = {Tipo::BalaDePrata, "a bala de prata", "o escudo do Hugo quebrou", 0.0f, 0.0f, 1, 0.0f};
                break;
            case Golpe::Materia::Lucas:
                d = {Tipo::PromiseAll, "Promise.all()", "doze promessas, juntas", 0.7f, 60.0f, 12, 0.0f};
                break;
            case Golpe::Materia::Vitor:
                d = {Tipo::Mutex, "P(s)", "os tiros fazem fila: passa um por vez", 5.0f, 0.0f, 0,
                     kEntrePassagens};
                break;
            case Golpe::Materia::Arroyo:
                d = {Tipo::BuscaBinaria, "busca bin\xc3\xa1ria", "metade dos tiros some, de novo e de novo",
                     static_cast<float>(kPassosDaBusca) * kEntrePassosDaBusca + 0.3f, 0.0f, kPassosDaBusca,
                     kEntrePassosDaBusca};
                break;
            case Golpe::Materia::Goulart:
                d = {Tipo::Rodizio, "Round Robin", "os tiros do Hugo andam um quarto por vez", 5.0f, 0.0f,
                     kGruposDoRodizio, kQuantumDoRodizio};
                break;
            case Golpe::Materia::Michel:
                // \xc3\xa2 e o a circunflexo, \xc3\xa7 o c cedilha.
                d = {Tipo::CameraEscura, "c\xc3\xa2mera escura", "o tiro que passa pelo furo volta de cabe\xc3\xa7" "a para baixo",
                     4.5f, kDistanciaDaParede, 0, 0.0f};
                break;
            case Golpe::Materia::Sabrina:
                // \xc3\xb3 e o o agudo.
                d = {Tipo::Pcr, "PCR", "seus tiros se copiam: 1, 2, 4, 8",
                     static_cast<float>(kCiclosDaPcr) * kEntreCiclosDaPcr + 0.2f, 0.0f, kTetoDaPcr, kEntreCiclosDaPcr};
                break;
            case Golpe::Materia::Daniel:
                // \xc3\xa9 e o e agudo, \xc3\xb3 o o agudo.
                d = {Tipo::KMedias, "k-m\xc3\xa9" "dias", "os tiros do Hugo viram 4 centr\xc3\xb3" "ides", 0.0f, 0.0f,
                     kGruposDasKMedias, 0.0f};
                break;
            default:
                break;
        }
        return d;
    }

    float Fracao(const int carga) {
        return std::clamp(static_cast<float>(carga) / static_cast<float>(kCarga), 0.0f, 1.0f);
    }

    bool Ataca(const Tipo t) {
        // Os agentes nao: com o Hugo blindado, os tiros cacam os alvos do objetivo.
        return t == Tipo::Matriz || t == Tipo::Converter || t == Tipo::PromiseAll || t == Tipo::BalaDePrata ||
               t == Tipo::Pcr;
    }

    Vector2 Agente(const Vector2& centro, const int k, const int n, const float t, const float raio) {
        const float a = 2.0f * Math::Pi * (static_cast<float>(k) / static_cast<float>(std::max(1, n)) + 0.5f * t);
        return centro + Vector2(Math::Cos(a), Math::Sin(a)) * raio;
    }

    Celula CelulaDaMatriz(const int k) {
        Celula c;
        const int n = std::max(0, k);
        c.i = n / kColunasDaMatriz;
        c.j = n % kColunasDaMatriz;
        c.quando = static_cast<float>(c.i) * kEntreLinhas + static_cast<float>(c.j) * kEntreCelulas;
        c.dx = (static_cast<float>(c.j) - static_cast<float>(kColunasDaMatriz - 1) / 2.0f) * kEntreColunas;
        return c;
    }

    float DesvioNoLeque(const int k, const int n, const float abertura) {
        if (n <= 1) return 0.0f;
        return -abertura / 2.0f + abertura * static_cast<float>(k) / static_cast<float>(n - 1);
    }

    bool NaPorta(const float y, const float vy, const float porta) {
        return vy > 0.0f && y >= porta - kFolgaDaPorta && y <= porta + kFolgaDaPorta / 2.0f;
    }

    Vector2 Guiar(const Vector2& velocidade, const Vector2& de, const Vector2& alvo, const float giro, const float dt) {
        const float rapidez = velocidade.Length();
        const Vector2 falta = alvo - de;
        if (rapidez <= 0.0f || falta.LengthSq() <= 1.0f) return velocidade;
        const float agora = std::atan2(velocidade.y, velocidade.x);
        float quer = std::atan2(falta.y, falta.x) - agora;
        while (quer > Math::Pi) quer -= 2.0f * Math::Pi;
        while (quer < -Math::Pi) quer += 2.0f * Math::Pi;
        const float passo = std::clamp(quer, -giro * dt, giro * dt);
        const float novo = agora + passo;
        return Vector2(std::cos(novo), std::sin(novo)) * rapidez;
    }

    int QuadroDaMateria(const Golpe::Materia m) {
        // Os quadros 3 a 12 da folha, na ordem do enum (Generico e o 0); o Arroyo, que entrou depois,
        // no fim dela (gerar_hugo.py poe as linhas novas sempre no fim).
        if (m == Golpe::Materia::Generico) return -1;
        if (m == Golpe::Materia::Arroyo) return kQuadroDoArroyo;
        if (m == Golpe::Materia::Goulart) return kQuadroDoGoulart;
        if (m == Golpe::Materia::Daniel) return kQuadroDoDaniel;
        if (m == Golpe::Materia::Michel) return kQuadroDoMichel;
        if (m == Golpe::Materia::Sabrina) return kQuadroDaSabrina;
        return 2 + static_cast<int>(m);
    }

    int CopiasDoCiclo(const int emVoo, const int feitas) {
        return std::clamp(std::min(emVoo, kTetoDaPcr - feitas), 0, kTetoDaPcr);
    }

    bool CruzouAParede(const float y, const float vy, const float parede) {
        return vy > 0.0f && y >= parede && y <= parede + kFaixaDaParede;
    }

    Vector2 PeloFuro(const Vector2& p, const Vector2& furo) {
        return furo * 2.0f - p;
    }

    std::vector<Vector2> KMedias(const std::vector<Vector2>& pontos, const int k, std::vector<int>& grupos) {
        grupos.assign(pontos.size(), -1);
        if (pontos.empty() || k <= 0) return {};
        // As sementes: de ponta a ponta, pelo x - espalhadas, e sem sorteio.
        std::vector<size_t> ordem(pontos.size());
        for (size_t i = 0; i < ordem.size(); ++i) ordem[i] = i;
        std::sort(ordem.begin(), ordem.end(), [&](const size_t a, const size_t b) { return pontos[a].x < pontos[b].x; });
        const int n = std::min(k, static_cast<int>(pontos.size()));
        std::vector<Vector2> centros;
        for (int j = 0; j < n; ++j) {
            centros.push_back(pontos[ordem[static_cast<size_t>((2 * j + 1) * static_cast<int>(pontos.size()) / (2 * n))]]);
        }
        for (int it = 0; it < kIteracoesDasKMedias; ++it) {
            grupos = Agrupamento::Atribuir(pontos, centros);
            centros = Agrupamento::Atualizar(pontos, grupos, centros, {}, 0.0f);
        }
        grupos = Agrupamento::Atribuir(pontos, centros);
        return centros;
    }

    int LevaNoPasso(const int n) {
        return n <= 0 ? 0 : (n + 1) / 2;
    }

    int GrupoDaVez(const float t) {
        if (t <= 0.0f) return 0;
        return static_cast<int>(t / kQuantumDoRodizio) % kGruposDoRodizio;
    }

    bool NoAlcance(const Vector2& centro, const Vector2& tiro, const float raio) {
        return (tiro - centro).LengthSq() <= raio * raio;
    }
}

// A previa: o poder em miniatura.

namespace Poder {

    namespace {
        constexpr float kHugoX = 0.5f;
        constexpr float kHugoY = 0.10f;
        constexpr float kAlunoX = 0.5f;
        constexpr float kAlunoY = 0.88f;

        /// O campo da previa e duas vezes mais largo que alto: um circulo tem o raio em y dobrado.
        constexpr float kAspecto = 2.0f;

        float Fracao01(const float v) { return v - std::floor(v); }

        /// Os tiros do Hugo, chovendo: 'n' deles, cada um na sua coluna e no seu compasso.
        std::vector<Ponto> Chuva(const float t, const int n) {
            std::vector<Ponto> chuva;
            for (int k = 0; k < n; ++k) {
                const float x = 0.10f + 0.80f * Fracao01(static_cast<float>(k) * 0.618f + 0.11f);
                const float y = 0.16f + 0.84f * Fracao01(t * 0.28f + static_cast<float>(k) * 0.37f);
                chuva.push_back({Marca::TiroDoHugo, x, y});
            }
            return chuva;
        }

        Ponto Entre(const Marca m, const float x0, const float y0, const float x1, const float y1, const float u) {
            return {m, x0 + (x1 - x0) * u, y0 + (y1 - y0) * u};
        }

        /// Os tiros de sempre do aluno, subindo ate (x, y).
        void Atirar(Quadro& q, const float t, const float x, const float y, const float ate = 1.0f) {
            for (int k = 0; k < 4; ++k) {
                const float u = Fracao01(t * 0.9f + static_cast<float>(k) / 4.0f);
                if (u <= ate) q.pontos.push_back(Entre(Marca::TiroSeu, kAlunoX, kAlunoY - 0.06f, x, y, u));
            }
        }

        void Anel(Quadro& q, const Marca cor, const float x, const float y, const float raio) {
            constexpr int kLados = 14;
            for (int k = 0; k < kLados; ++k) {
                const float a0 = 2.0f * Math::Pi * static_cast<float>(k) / kLados;
                const float a1 = 2.0f * Math::Pi * static_cast<float>(k + 1) / kLados;
                q.tracos.push_back({cor, x + std::cos(a0) * raio, y + std::sin(a0) * raio * kAspecto,
                                    x + std::cos(a1) * raio, y + std::sin(a1) * raio * kAspecto});
            }
        }

        void Caixa(Quadro& q, const Marca cor, const float x0, const float y0, const float x1, const float y1) {
            q.tracos.push_back({cor, x0, y0, x1, y0});
            q.tracos.push_back({cor, x1, y0, x1, y1});
            q.tracos.push_back({cor, x1, y1, x0, y1});
            q.tracos.push_back({cor, x0, y1, x0, y0});
        }

        /// Um ponto na curva de (x0, y0) a (x2, y2) puxada por (x1, y1).
        Ponto NaCurva(const Marca m, const float x0, const float y0, const float x1, const float y1,
                      const float x2, const float y2, const float u) {
            const float a = (1.0f - u) * (1.0f - u), b = 2.0f * u * (1.0f - u), c = u * u;
            return {m, a * x0 + b * x1 + c * x2, a * y0 + b * y1 + c * y2};
        }
    }

    Quadro Previa(const Tipo tipo, const float tempo) {
        Quadro q;
        const float t = std::fmod(std::max(0.0f, tempo), kCicloDaPrevia);
        float hugoX = kHugoX;

        switch (tipo) {
            case Tipo::Matriz: {
                constexpr float kSai = 0.5f;
                // Na miniatura o laco anda devagar, para se ver cada volta.
                constexpr float kPorLinha = 0.30f, kPorCelula = 0.025f;
                for (int k = 0; k < kLinhasDaMatriz * kColunasDaMatriz; ++k) {
                    const Celula c = CelulaDaMatriz(k);
                    const float idade = t - kSai - static_cast<float>(c.i) * kPorLinha - static_cast<float>(c.j) * kPorCelula;
                    const float y = 0.80f - idade * 0.26f;
                    if (idade < 0.0f || y < kHugoY + 0.04f) continue;
                    q.pontos.push_back({Marca::TiroSeu, kAlunoX + c.dx / kEntreColunas * 0.045f, y});
                }
                const int i = std::clamp(static_cast<int>((t - kSai) / kPorLinha), 0, kLinhasDaMatriz - 1);
                if (t < kSai) q.legenda = "for (i = 0; i < 5; i++)";
                else if (t < kSai + static_cast<float>(kLinhasDaMatriz) * kPorLinha) {
                    q.legenda = "i = " + std::to_string(i) + ":  for (j = 0; j < 8; j++)";
                } else q.legenda = "5 linhas x 8 colunas = 40 tiros";
                break;
            }
            case Tipo::Pilha: {
                constexpr float kDevolve = 2.7f;
                constexpr int kCabem = 8;
                const bool empilhando = t < kDevolve;
                const int n = empilhando ? std::min(kCabem, static_cast<int>(t / 0.3f))
                                         : std::max(0, kCabem - static_cast<int>((t - kDevolve) / 0.13f));
                for (const Ponto& p : Chuva(t, 9)) {
                    // O que chega perto da capivara e comido: entra na pilha.
                    if (p.y > 0.70f && std::fabs(p.x - kAlunoX) < 0.24f) continue;
                    q.pontos.push_back(p);
                }
                for (int b = 0; b < n; ++b) {
                    const float base = 0.97f - static_cast<float>(b) * 0.075f;
                    Caixa(q, Marca::Estrutura, 0.61f, base - 0.055f, 0.68f, base);
                }
                for (int p = 0; p < kCabem - n && !empilhando; ++p) {
                    const float u = (t - kDevolve - static_cast<float>(p) * 0.13f) / 0.55f;
                    if (u >= 0.0f && u <= 1.0f) q.pontos.push_back(Entre(Marca::TiroSeu, 0.56f, 0.90f, hugoX, kHugoY, u));
                }
                q.pontos.push_back({Marca::Figura, kAlunoX, 0.97f});
                q.legenda = (empilhando ? "push(tiro)   pilha: " : "pop() no Hugo   pilha: ") + std::to_string(n);
                break;
            }
            case Tipo::Congelar: {
                constexpr float kPara = 1.2f, kVolta = 3.4f;
                const bool parado = t >= kPara && t < kVolta;
                const float doHugo = t < kPara ? t : (parado ? kPara : kPara + (t - kVolta));
                for (const Ponto& p : Chuva(doHugo, 10)) q.pontos.push_back(p);
                Atirar(q, t, hugoX, kHugoY);
                if (parado) Caixa(q, Marca::Estrutura, hugoX - 0.06f, kHugoY - 0.09f, hugoX + 0.06f, kHugoY + 0.09f);
                q.legenda = parado ? "HLT: o clock parou" : "clock: o Hugo atira";
                break;
            }
            case Tipo::Transacao: {
                constexpr float kBegin = 0.6f, kFim = 3.3f, kSalva = 0.62f;
                const float acertos[] = {1.3f, 1.9f, 2.5f};
                float nota = kSalva;
                for (const float a : acertos) {
                    if (t >= a && t < kFim) nota -= 0.09f;
                    if (t >= a - 0.25f && t < a) {
                        q.pontos.push_back({Marca::TiroDoHugo, kAlunoX, kAlunoY - 0.30f * (a - t) / 0.25f - 0.04f});
                    }
                }
                for (const Ponto& p : Chuva(t, 6)) if (std::fabs(p.x - kAlunoX) > 0.08f) q.pontos.push_back(p);
                // A barra da nota, embaixo: o trilho, o quanto vale e a marca do BEGIN.
                q.tracos.push_back({Marca::Apagado, 0.10f, 0.985f, 0.90f, 0.985f});
                q.tracos.push_back({Marca::TiroSeu, 0.10f, 0.985f, 0.10f + 0.80f * nota, 0.985f});
                if (t >= kBegin) {
                    const float x = 0.10f + 0.80f * kSalva;
                    q.tracos.push_back({Marca::Figura, x, 0.93f, x, 1.0f});
                    if (t < kFim) Caixa(q, Marca::Estrutura, kAlunoX - 0.07f, kAlunoY - 0.13f, kAlunoX + 0.07f, kAlunoY + 0.07f);
                }
                if (t < kBegin) q.legenda = "a nota, antes";
                else if (t < acertos[0]) q.legenda = "BEGIN: a nota está salva";
                else if (t < kFim) q.legenda = "apanhando: a nota cai";
                else q.legenda = "ROLLBACK: a nota voltou";
                break;
            }
            case Tipo::TryCatch: {
                constexpr float kAbre = 0.5f, kFecha = 3.9f;
                const float capturas[] = {1.2f, 2.5f};
                const bool dentro = t >= kAbre && t < kFecha;
                for (const Ponto& p : Chuva(t, 6)) if (std::fabs(p.x - kAlunoX) > 0.10f) q.pontos.push_back(p);
                q.legenda = t < kAbre ? "fora do try: acerto tira nota" : (dentro ? "try {" : "}");
                for (const float c : capturas) {
                    if (t >= c - 0.3f && t < c) {
                        q.pontos.push_back({Marca::TiroDoHugo, kAlunoX, kAlunoY - 0.05f - 0.30f * (c - t) / 0.3f});
                    }
                    const float u = (t - c) / 0.65f;
                    if (u < 0.0f || u > 1.0f) continue;
                    for (int k = 0; k < 5; ++k) {
                        Ponto p = Entre(Marca::TiroSeu, kAlunoX, kAlunoY - 0.06f, hugoX, kHugoY, u);
                        p.x += DesvioNoLeque(k, 5, kAberturaDoFinally) / kAberturaDoFinally * 0.22f * u * (1.0f - u * 0.6f);
                        q.pontos.push_back(p);
                    }
                    q.legenda = u < 0.45f ? "catch (acerto): não tira nota" : "finally: um leque no Hugo";
                }
                if (dentro) {
                    // As chaves do bloco, em volta do aluno.
                    for (const float lado : {-1.0f, 1.0f}) {
                        const float x = kAlunoX + lado * 0.085f;
                        q.tracos.push_back({Marca::Estrutura, x, kAlunoY - 0.13f, x, kAlunoY + 0.08f});
                        q.tracos.push_back({Marca::Estrutura, x, kAlunoY - 0.13f, x - lado * 0.02f, kAlunoY - 0.13f});
                        q.tracos.push_back({Marca::Estrutura, x, kAlunoY + 0.08f, x - lado * 0.02f, kAlunoY + 0.08f});
                    }
                }
                break;
            }
            case Tipo::BuscaBinaria: {
                // Dezesseis tiros do Hugo; a cada passo some a metade mais perto do aluno.
                constexpr float kComeca = 1.1f, kPorPasso = 0.55f;
                std::vector<Ponto> chuva = Chuva(std::min(t, kComeca), 16);
                std::sort(chuva.begin(), chuva.end(), [](const Ponto& a, const Ponto& b) {
                    const auto d = [](const Ponto& p) {
                        const float dx = p.x - kAlunoX, dy = (p.y - kAlunoY) / kAspecto;
                        return dx * dx + dy * dy;
                    };
                    return d(a) < d(b);
                });
                const int passos = t < kComeca ? 0 : std::min(4, 1 + static_cast<int>((t - kComeca) / kPorPasso));
                size_t ficam = chuva.size();
                for (int k = 0; k < passos; ++k) ficam -= static_cast<size_t>(LevaNoPasso(static_cast<int>(ficam)));
                for (size_t k = chuva.size() - ficam; k < chuva.size(); ++k) q.pontos.push_back(chuva[k]);
                if (passos > 0 && t - kComeca - static_cast<float>(passos - 1) * kPorPasso < 0.3f) {
                    Anel(q, Marca::Estrutura, kAlunoX, kAlunoY, 0.12f + 0.05f * static_cast<float>(passos));
                }
                q.legenda = passos == 0 ? "busca bin\xc3\xa1ria: 16 tiros"
                                        : "passo " + std::to_string(passos) + ": sobram " + std::to_string(ficam) +
                                              (passos == 4 ? "  (log\xe2\x82\x82 16 = 4)" : "");
                break;
            }
            case Tipo::Rodizio: {
                // Doze tiros do Hugo caindo; o Round Robin os divide em quatro grupos, e so o da vez
                // anda - os outros esperam, parados.
                constexpr float kComeca = 0.8f;
                constexpr int kTiros = 12;
                const float desde = std::max(0.0f, t - kComeca);
                const int quanta = static_cast<int>(desde / kQuantumDoRodizio);
                const int daVez = GrupoDaVez(desde);
                for (int k = 0; k < kTiros; ++k) {
                    const int grupo = k % kGruposDoRodizio;
                    // O quanto este tiro ja andou: tudo antes do rodizio, e depois so os quanta dele.
                    int dele = quanta / kGruposDoRodizio + (grupo < quanta % kGruposDoRodizio ? 1 : 0);
                    float andou = std::min(t, kComeca) + kQuantumDoRodizio * static_cast<float>(dele);
                    if (t > kComeca && grupo == daVez) andou += desde - kQuantumDoRodizio * static_cast<float>(quanta);
                    q.pontos.push_back(Chuva(andou, kTiros)[static_cast<size_t>(k)]);
                }
                q.legenda = t < kComeca ? "Round Robin: 12 tiros, 4 grupos"
                                        : "quantum: o grupo " + std::to_string(daVez + 1) + " de 4 anda";
                break;
            }
            case Tipo::CameraEscura: {
                // A parede com o furo, em cima do aluno: o tiro do Hugo que a cruza sai do outro lado
                // do furo, de cabeca para baixo, e volta subindo.
                constexpr float kParede = 0.64f, kFuro = 0.03f;
                q.tracos.push_back({Marca::Estrutura, 0.04f, kParede, kAlunoX - kFuro, kParede});
                q.tracos.push_back({Marca::Estrutura, kAlunoX + kFuro, kParede, 0.96f, kParede});
                for (const Ponto& p : Chuva(t, 10)) {
                    if (p.y < kParede) {
                        q.pontos.push_back(p);
                        continue;
                    }
                    const Vector2 pelo = PeloFuro(Vector2(p.x, p.y), Vector2(kAlunoX, kParede));
                    if (pelo.y > kHugoY + 0.04f) q.pontos.push_back({Marca::TiroDoHugo, pelo.x, pelo.y});
                    // O raio de luz do lugar de cada um ao furo.
                    q.tracos.push_back({Marca::Apagado, pelo.x, pelo.y, kAlunoX, kParede});
                }
                Atirar(q, t, hugoX, kHugoY);
                // \xc3\xa2 e o a circunflexo, \xc3\xa7 o c cedilha.
                q.legenda = Fracao01(t / 3.0f) < 0.5f ? "c\xc3\xa2mera escura: o furo em cima de voc\xc3\xaa"
                                                      : "o tiro passa e volta de cabe\xc3\xa7" "a para baixo";
                break;
            }
            case Tipo::KMedias: {
                // Dezesseis tiros do Hugo caindo; as k-medias os juntam em quatro grupos - uma linha de
                // cada tiro ate o centroide dele -, e cada grupo vira um tiro so, que segue caindo.
                constexpr float kAgrupa = 1.2f, kJunta = 1.9f;
                constexpr int kTiros = 16;
                const std::vector<Ponto> agora = Chuva(t, kTiros);
                std::vector<Vector2> naHora;
                for (const Ponto& p : Chuva(std::min(t, kAgrupa), kTiros)) naHora.emplace_back(p.x, p.y);
                std::vector<int> grupos;
                const std::vector<Vector2> centros = KMedias(naHora, kGruposDasKMedias, grupos);
                if (t < kJunta) {
                    for (const Ponto& p : agora) q.pontos.push_back(p);
                    if (t >= kAgrupa) {
                        for (size_t k = 0; k < agora.size(); ++k) {
                            const Vector2 c = centros[static_cast<size_t>(grupos[k])];
                            const float desce = agora[k].y - naHora[k].y;
                            q.tracos.push_back({Marca::Estrutura, agora[k].x, agora[k].y, c.x, c.y + desce});
                        }
                    }
                } else {
                    for (size_t g = 0; g < centros.size(); ++g) {
                        // O centroide desce com o grupo dele.
                        float desce = 0.0f;
                        int n = 0;
                        for (size_t k = 0; k < agora.size(); ++k) {
                            if (grupos[k] != static_cast<int>(g)) continue;
                            desce += agora[k].y - naHora[k].y;
                            ++n;
                        }
                        q.pontos.push_back({Marca::TiroDoHugo, centros[g].x, centros[g].y + (n > 0 ? desce / static_cast<float>(n) : 0.0f)});
                        Anel(q, Marca::Estrutura, centros[g].x, centros[g].y + (n > 0 ? desce / static_cast<float>(n) : 0.0f), 0.035f);
                    }
                }
                // \xc3\xa9 e o e agudo, \xc3\xa7\xc3\xa3 o "cao", \xc3\xb3 o o agudo.
                q.legenda = t < kAgrupa ? "k-m\xc3\xa9" "dias: 16 tiros"
                                        : (t < kJunta ? "atribui\xc3\xa7\xc3\xa3o: cada tiro ao centr\xc3\xb3" "ide"
                                                      : "16 tiros viram 4");
                break;
            }
            case Tipo::Converter: {
                constexpr float kRejeita = 1.4f, kRaio = 0.17f;
                const std::vector<Ponto> naHora = Chuva(std::min(t, kRejeita), 10);
                const std::vector<Ponto> agora = Chuva(t, 10);
                for (size_t k = 0; k < naHora.size(); ++k) {
                    const float dx = naHora[k].x - kAlunoX, dy = (naHora[k].y - kAlunoY) / kAspecto;
                    const bool perto = dx * dx + dy * dy <= kRaio * kRaio;
                    if (t < kRejeita || !perto) {
                        q.pontos.push_back(agora[k]);
                        continue;
                    }
                    const float u = (t - kRejeita) / 0.9f;
                    if (u <= 1.0f) q.pontos.push_back(Entre(Marca::TiroSeu, naHora[k].x, naHora[k].y, hugoX, kHugoY, u));
                }
                if (t >= kRejeita && t < kRejeita + 0.45f) {
                    Anel(q, Marca::Estrutura, kAlunoX, kAlunoY, kRaio * (t - kRejeita) / 0.45f);
                }
                q.legenda = t < kRejeita ? "o autômato lê os tiros em volta" : "rejeita: eles viram seus";
                break;
            }
            case Tipo::Agentes: {
                hugoX = 0.80f;
                for (int k = 0; k < 3; ++k) {
                    const float a = 2.0f * Math::Pi * (static_cast<float>(k) / 3.0f + 0.4f * t);
                    q.pontos.push_back({Marca::Figura, kAlunoX + std::cos(a) * 0.07f,
                                        kAlunoY - 0.03f + std::sin(a) * 0.07f * kAspecto * 0.6f});
                }
                for (int k = 0; k < 5; ++k) {
                    const float u = Fracao01(t * 0.6f + static_cast<float>(k) / 5.0f);
                    q.pontos.push_back(NaCurva(Marca::TiroSeu, kAlunoX, kAlunoY - 0.06f, kAlunoX, 0.30f, hugoX, kHugoY, u));
                }
                q.legenda = "os seus tiros caçam o alvo";
                break;
            }
            case Tipo::BalaDePrata: {
                constexpr float kSai = 1.4f, kQuebra = 2.4f;
                const bool blindado = t < kQuebra;
                if (blindado) Anel(q, Marca::Estrutura, hugoX, kHugoY + 0.02f, 0.075f);
                Atirar(q, t, hugoX, kHugoY, blindado ? 0.78f : 1.0f);
                if (t >= kSai && t < kQuebra) {
                    q.pontos.push_back(NaCurva(Marca::Figura, 0.40f, 0.80f, 0.12f, 0.40f, hugoX, kHugoY,
                                               (t - kSai) / (kQuebra - kSai)));
                }
                if (t < kSai) q.legenda = "o Hugo blindado: nada passa";
                else if (t < kQuebra) q.legenda = "a bala de prata";
                else q.legenda = "o escudo quebrou";
                break;
            }
            case Tipo::PromiseAll: {
                constexpr float kResolve = 1.2f, kChega = 2.2f, kVolta = 3.5f;
                constexpr float kOrientadorX = 0.36f, kOrientadorY = 0.78f;
                for (int k = 0; k < 12; ++k) {
                    const float a = 2.0f * Math::Pi * static_cast<float>(k) / 12.0f;
                    const float x = kOrientadorX + std::cos(a) * 0.06f, y = kOrientadorY + std::sin(a) * 0.06f * kAspecto;
                    if (t < kResolve) q.pontos.push_back({Marca::Apagado, x, y});
                    else if (t < kChega) q.pontos.push_back(Entre(Marca::TiroSeu, x, y, hugoX, kHugoY, (t - kResolve) / (kChega - kResolve)));
                    else if (t < kVolta && k % 2 == 0) {
                        q.pontos.push_back(Entre(Marca::Figura, hugoX, kHugoY, x, y, (t - kChega) / (kVolta - kChega)));
                    }
                }
                q.pontos.push_back({Marca::Figura, kOrientadorX, kOrientadorY});
                if (t < kResolve) q.legenda = "12 promessas, pendentes";
                else if (t < kChega) q.legenda = "resolvem juntas, no Hugo";
                else q.legenda = "e voltam, desfazendo tiros";
                break;
            }
            case Tipo::Mutex: {
                constexpr float kFecha = 0.5f, kAbre = 3.9f, kPorta = 0.52f;
                const bool fechada = t >= kFecha && t < kAbre;
                for (Ponto p : Chuva(t, 10)) {
                    if (fechada && p.y > kPorta - 0.03f) p.y = kPorta - 0.03f;   // esperando na porta
                    q.pontos.push_back(p);
                }
                if (fechada) {
                    q.tracos.push_back({Marca::Estrutura, 0.02f, kPorta, 0.98f, kPorta});
                    q.tracos.push_back({Marca::Estrutura, 0.02f, kPorta + 0.025f, 0.98f, kPorta + 0.025f});
                    // Um de cada vez.
                    const float volta = (t - kFecha) / 0.7f;
                    const float x = 0.18f + 0.64f * Fracao01(std::floor(volta) * 0.37f);
                    q.pontos.push_back({Marca::TiroDoHugo, x, kPorta + 0.03f + (1.0f - kPorta - 0.03f) * Fracao01(volta)});
                }
                if (t < kFecha) q.legenda = "sem trava: todos de uma vez";
                else if (fechada) q.legenda = "P(s): fila na porta, passa um por vez";
                else q.legenda = "V(s): quem esperava morreu";
                break;
            }
            case Tipo::Pcr: {
                // O tiro sobe e, a cada ciclo, cada tiro vira dois: 1, 2, 4, 8 - todos no Hugo.
                constexpr float kSai = 0.6f, kCiclo = 0.75f;
                for (const Ponto& p : Chuva(t, 8)) q.pontos.push_back(p);
                if (t < kSai) {
                    Atirar(q, t, hugoX, kHugoY);
                    // \xc3\xa9 e o e agudo.
                    q.legenda = "PCR: o seu tiro \xc3\xa9 o molde";
                    break;
                }
                const float voo = t - kSai;
                const int ciclos = std::min(kCiclosDaPcr, static_cast<int>(voo / kCiclo));
                const int n = 1 << ciclos;
                const float y = kAlunoY - 0.06f - voo * 0.2f;
                // \xc3\xb3 e o o agudo.
                if (y > kHugoY + 0.04f) {
                    for (int k = 0; k < n; ++k) {
                        const float x = kAlunoX + (static_cast<float>(k) - 0.5f * static_cast<float>(n - 1)) * 0.04f;
                        q.pontos.push_back({Marca::TiroSeu, x, y});
                    }
                    q.legenda = ciclos == 0 ? std::string("o molde sobe")
                                            : "ciclo " + std::to_string(ciclos) + ": " + std::to_string(n) + " c\xc3\xb3pias";
                } else {
                    q.legenda = std::to_string(n) + " c\xc3\xb3pias no Hugo";
                }
                break;
            }
            case Tipo::Nenhum:
                break;
        }

        // O que sai do campo nao se desenha: o traco todo de fora some, o que cruza a borda para nela.
        auto fora = [](const float v) { return v < 0.0f || v > 1.0f; };
        q.tracos.erase(std::remove_if(q.tracos.begin(), q.tracos.end(), [&fora](const Traco& r) {
            return (fora(r.x0) && fora(r.x1)) || (fora(r.y0) && fora(r.y1));
        }), q.tracos.end());
        for (Traco& r : q.tracos) {
            r.x0 = std::clamp(r.x0, 0.0f, 1.0f);
            r.x1 = std::clamp(r.x1, 0.0f, 1.0f);
            r.y0 = std::clamp(r.y0, 0.0f, 1.0f);
            r.y1 = std::clamp(r.y1, 0.0f, 1.0f);
        }

        q.pontos.push_back({Marca::Hugo, hugoX, kHugoY});
        q.pontos.push_back({Marca::Aluno, kAlunoX, kAlunoY});
        return q;
    }
}
