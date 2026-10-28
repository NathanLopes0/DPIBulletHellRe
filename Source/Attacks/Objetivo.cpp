//
// O escudo do Hugo e o objetivo de cada etapa do TCC.
//

#include "Objetivo.h"

#include <algorithm>
#include <cmath>

namespace Objetivo {

    Etapa DaFase(const std::string& fase) {
        if (fase == "StateOne") return Etapa::Proposta;
        if (fase == "StateTwo") return Etapa::Desenvolvimento;
        if (fase == "StateThree") return Etapa::Defesa;
        if (fase == "StateFinal") return Etapa::Correcoes;
        return Etapa::Nenhuma;
    }

    namespace {
        /// 'n' x espalhados pela faixa, cada um sorteado na sua fatia - nunca
        /// dois em cima um do outro.
        std::vector<float> Espalhar(Variacao::Sorteio& s, const int n, const float largura) {
            std::vector<float> xs;
            const float fatia = (largura - 2.0f * kMargem) / static_cast<float>(std::max(1, n));
            for (int i = 0; i < n; ++i) {
                const float de = kMargem + fatia * static_cast<float>(i);
                xs.push_back(s.Real(de + fatia * 0.25f, de + fatia * 0.75f));
            }
            return xs;
        }
    }

    std::vector<Alvo> Gerar(const Etapa etapa, Variacao::Sorteio& s, const float largura,
                            const Dificuldade::Nivel n, const int certa) {
        std::vector<Alvo> alvos;
        const auto N = [n](const int normal, const int dificil) { return Dificuldade::Escolher(n, normal, dificil); };
        switch (etapa) {
            case Etapa::Proposta:
                for (const float x : Espalhar(s, N(2, 3), largura)) {
                    alvos.push_back({"Assinatura", Vector2(x, s.Real(kYDeCima, kYDeBaixo)), true});
                }
                break;
            case Etapa::Desenvolvimento:
                for (const float x : Espalhar(s, N(3, 4), largura)) {
                    alvos.push_back({"Bug", Vector2(x, s.Real(kYDeCima - 60.0f, kYDeBaixo)), true});
                }
                break;
            case Etapa::Defesa: {
                // AS TRES RESPOSTAS lado a lado, na ordem do placar.
                const auto xs = Espalhar(s, 3, largura);
                const char* letras[] = {"A", "B", "C"};
                for (int i = 0; i < 3; ++i) {
                    alvos.push_back({letras[i], Vector2(xs[static_cast<size_t>(i)], (kYDeCima + kYDeBaixo) / 2.0f), i == certa});
                }
                break;
            }
            case Etapa::Correcoes:
                for (const float x : Espalhar(s, N(2, 3), largura)) {
                    alvos.push_back({"Correcao", Vector2(x, s.Real(kYDeCima, kYDeBaixo)), true});
                }
                break;
            default:
                break;
        }
        return alvos;
    }

    Vector2 Balanco(const Etapa etapa, const Vector2& lugar, const int k, const float t) {
        const float fase = static_cast<float>(k) * 1.7f;
        switch (etapa) {
            case Etapa::Defesa:
                // AS RESPOSTAS vao e voltam de lado, cada uma no seu ritmo.
                return lugar + Vector2(70.0f * std::sin(t * 1.3f + fase), 12.0f * std::sin(t * 2.1f + fase));
            case Etapa::Correcoes:
                return lugar + Vector2(25.0f * std::sin(t * 0.8f + fase), 20.0f * std::sin(t * 1.1f + fase));
            default:
                return lugar + Vector2(30.0f * std::sin(t * 1.0f + fase), 18.0f * std::sin(t * 1.6f + fase));
        }
    }

    Vector2 Paragem(Variacao::Sorteio& s, const float largura) {
        return Vector2(s.Real(kMargem, largura - kMargem), s.Real(kYDeCima - 60.0f, kYDeBaixo));
    }

    Questao Perguntar(const Golpe::Materia m, Variacao::Sorteio& s) {
        // A PRIMEIRA E A CERTA; o placar as embaralha.
        Questao q;
        std::array<std::string, 3> r;
        switch (m) {
            case Golpe::Materia::Andre:
                q.pergunta = "Cadê o fluxograma?";
                r = {"no apêndice A", "fluxo... o quê?", "no meu coração"}; break;
            case Golpe::Materia::Salles:
                q.pergunta = "E se a pilha estourar?";
                r = {"limito a recursão", "compro mais RAM", "reinicio o PC"}; break;
            case Golpe::Materia::Ricardo:
                q.pergunta = "Quantos ciclos de clock isso leva?";
                r = {"cinco, no pipeline", "uns três segundos", "depende do dia"}; break;
            case Golpe::Materia::Thiago:
                q.pergunta = "Essa tabela está normalizada?";
                r = {"está na 3FN", "está numa planilha", "está em CSV"}; break;
            case Golpe::Materia::Luizinho:
                q.pergunta = "Onde esse ponteiro é liberado?";
                r = {"no destrutor", "o sistema libera", "nunca, é global"}; break;
            case Golpe::Materia::Vladimir:
                q.pergunta = "Isso é decidível?";
                r = {"sim, é regular", "rodei e parou", "o chat disse que sim"}; break;
            case Golpe::Materia::Julio:
                q.pergunta = "Por que não usou um LLM?";
                r = {"tinha que ser explicável", "não conhecia", "usei: ele escreveu isto"}; break;
            case Golpe::Materia::MariaLucia:
                q.pergunta = "Cadê a documentação?";
                r = {"no README e no UML", "o código é a doc", "sai na versão 2"}; break;
            case Golpe::Materia::Lucas:
                q.pergunta = "Funciona no Safari?";
                r = {"testei em todos", "quem usa Safari?", "na minha máquina sim"}; break;
            case Golpe::Materia::Vitor:
                q.pergunta = "E se der deadlock?";
                r = {"ordeno as travas", "reinicio o servidor", "torço para não dar"}; break;
            default:
                q.pergunta = "Pode repetir?";
                r = {"posso", "não", "o quê?"}; break;
        }
        std::array<int, 3> ordem = {0, 1, 2};
        std::vector<int> v(ordem.begin(), ordem.end());
        s.Embaralhar(v);
        for (int i = 0; i < 3; ++i) {
            q.respostas[static_cast<size_t>(i)] = r[static_cast<size_t>(v[static_cast<size_t>(i)])];
            if (v[static_cast<size_t>(i)] == 0) q.certa = i;
        }
        return q;
    }

    float EntrePaginasSoltas(const Dificuldade::Nivel n) {
        return Dificuldade::Escolher(n, 0.24f, 0.24f);
    }

    float PorAlvo(const Dificuldade::Nivel n) { return Dificuldade::Escolher(n, 1.5f, 1.25f); }
    float PorJanela(const Dificuldade::Nivel n) { return Dificuldade::Escolher(n, 3.0f, 2.5f); }

    float Janela(const Etapa etapa, const Dificuldade::Nivel n) {
        // AS CORRECOES SAO A REPESCAGEM: a janela e mais longa.
        const float base = etapa == Etapa::Correcoes ? 9.0f : 8.0f;
        return Dificuldade::Escolher(n, base, base - 2.0f);
    }

    std::string Falta(const Etapa etapa, const int faltam) {
        const std::string n = std::to_string(faltam);
        switch (etapa) {
            case Etapa::Proposta:
                return faltam == 1 ? "falta 1 assinatura" : "faltam " + n + " assinaturas";
            case Etapa::Desenvolvimento:
                return faltam == 1 ? "falta 1 bug" : "faltam " + n + " bugs";
            case Etapa::Correcoes:
                return faltam == 1 ? "falta 1 correção" : "faltam " + n + " correções";
            case Etapa::Defesa:
                return "acerte a resposta certa";
            default:
                return "";
        }
    }

    // ---- AS OUTRAS MANEIRAS DE ABRIR -----------------------------------------------

    Tipo DeAndar(const Etapa etapa) {
        switch (etapa) {
            case Etapa::Proposta:        return Tipo::Reuniao;
            case Etapa::Desenvolvimento: return Tipo::Referencias;
            case Etapa::Defesa:          return Tipo::Arguicao;
            case Etapa::Correcoes:       return Tipo::Entrega;
            default:                     return Tipo::Derrubar;
        }
    }

    Tipo DaVez(const Etapa etapa, const int vez, const int primeira) {
        return (vez + primeira) % 2 == 0 ? Tipo::Derrubar : DeAndar(etapa);
    }

    float TempoDeReuniao(const Dificuldade::Nivel n) { return Dificuldade::Escolher(n, 2.5f, 3.0f); }

    Vector2 LugarDaReuniao(Variacao::Sorteio& s, const float largura, const float xDoAluno) {
        // A METADE OPOSTA a do aluno: a reuniao e sempre uma travessia.
        const float meio = largura / 2.0f;
        const float borda = kMargem + kRaioDaReuniao;
        const float x = xDoAluno < meio ? s.Real(meio + 60.0f, largura - borda) : s.Real(borda, meio - 60.0f);
        return Vector2(x, s.Real(kChaoDeCima + 20.0f, kChaoDeBaixo - 20.0f));
    }

    std::vector<Vector2> Referencias(Variacao::Sorteio& s, const float largura, const Dificuldade::Nivel n) {
        std::vector<Vector2> livros;
        for (const float x : Espalhar(s, Dificuldade::Escolher(n, 3, 4), largura)) {
            livros.emplace_back(x, s.Real(kChaoDeCima, kChaoDeBaixo));
        }
        return livros;
    }

    int Raspadas(const Dificuldade::Nivel n) { return Dificuldade::Escolher(n, 6, 8); }

    bool Raspou(const float distancia, const float raios) {
        return distancia > raios && distancia <= raios + kFolgaDaRaspada;
    }

    Entrega Entregar(Variacao::Sorteio& s, const float largura) {
        const bool daEsquerda = s.Chance(0.5f);
        const float canto = kMargem * 0.75f;
        Entrega e;
        e.secretaria = Vector2(daEsquerda ? largura - canto : canto, kChaoDeBaixo - 10.0f);
        const float x = daEsquerda ? s.Real(kMargem, largura * 0.35f) : s.Real(largura * 0.65f, largura - kMargem);
        e.versao = Vector2(x, s.Real(kChaoDeCima - 60.0f, kChaoDeCima + 40.0f));
        return e;
    }

    int Placas(const Tipo tipo, const Dificuldade::Nivel n, const int alvosCertos) {
        switch (tipo) {
            case Tipo::Reuniao:     return 5;
            case Tipo::Referencias: return Dificuldade::Escolher(n, 3, 4);
            case Tipo::Arguicao:    return Raspadas(n);
            case Tipo::Entrega:     return 2;
            default:                return std::max(1, alvosCertos);
        }
    }

    int PlacasDePe(const int placas, const float progresso) {
        const float falta = 1.0f - std::clamp(progresso, 0.0f, 1.0f);
        // O EPSILON: 1/3 de 3 placas nao pode deixar 2,0000001 de pe.
        return std::clamp(static_cast<int>(std::ceil(static_cast<float>(placas) * falta - 1e-4f)), 0, placas);
    }

    float PorPasso(const Tipo tipo, const Dificuldade::Nivel n) {
        switch (tipo) {
            case Tipo::Arguicao: return PorAlvo(n) * 0.5f;
            case Tipo::Reuniao:  return PorAlvo(n) * 2.0f;
            default:             return PorAlvo(n);
        }
    }

    std::string Pede(const Tipo tipo, const int faltam) {
        const std::string n = std::to_string(faltam);
        switch (tipo) {
            case Tipo::Reuniao:
                return "reunião: fique com o orientador";
            case Tipo::Referencias:
                return faltam == 1 ? "falta 1 referência" : "faltam " + n + " referências";
            case Tipo::Arguicao:
                return faltam == 1 ? "raspe em mais 1 pergunta" : "raspe em mais " + n + " perguntas";
            case Tipo::Entrega:
                return faltam > 0 ? "pegue a versão final" : "leve-a à secretaria";
            default:
                return "";
        }
    }
}
