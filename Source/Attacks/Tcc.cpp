//
// O TCC do Hugo: o tema do orientador, a banca e as perguntas dela.
//

#include "Tcc.h"

#include <algorithm>
#include <vector>

namespace Tcc {

    namespace {
        Golpe::Tiro Tiro(const float quando, const float velocidade) {
            Golpe::Tiro t;
            t.quando = quando;
            t.velocidade = velocidade;
            return t;
        }

        /// 'n' valores igualmente espacados de 'de' a 'ate' (um so: o meio).
        float Passo(const int i, const int n, const float de, const float ate) {
            if (n <= 1) return (de + ate) / 2.0f;
            return de + (ate - de) * static_cast<float>(i) / static_cast<float>(n - 1);
        }
    }

    Golpe::Rodada Padrao(const Materia m, const int k, Variacao::Sorteio& s, const Dificuldade::Nivel n) {
        const auto N = [n](const auto normal, const auto dificil) { return Dificuldade::Escolher(n, normal, dificil); };
        Golpe::Rodada r;
        auto& tiros = r.tiros;
        switch (m) {
            case Materia::Andre: {
                // for (i = 0; i < n; i++): um tiro por volta, e o laco varre o arco.
                const int q = N(10, 14);
                const float arco = N(45.0f, 65.0f);
                for (int i = 0; i < q; ++i) {
                    Golpe::Tiro t = Tiro(static_cast<float>(i) * N(0.06f, 0.05f), N(260.0f, 290.0f));
                    t.desvio = Passo(i, q, -arco, arco) * ((k % 2 == 0) ? 1.0f : -1.0f);
                    tiros.push_back(t);
                }
                break;
            }
            case Materia::Salles: {
                // PUSH, PUSH, PUSH... e o POP comeca pelo ultimo: cada nivel e
                // um leque de tres, mirado quando sai.
                const int niveis = N(6, 7);
                const float pop = static_cast<float>(niveis - 1) * 0.08f + 0.25f;
                for (int i = 0; i < niveis; ++i) {
                    for (const float d : {-14.0f, 0.0f, 14.0f}) {
                        Golpe::Tiro t = Tiro(static_cast<float>(i) * 0.08f, N(250.0f, 280.0f));
                        t.dy = 34.0f + 26.0f * static_cast<float>(i);
                        t.desvio = d;
                        t.espera = pop + static_cast<float>(niveis - 1 - i) * 0.12f - t.quando;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Ricardo: {
                // O BARRAMENTO: paredes de oito fios, e so os bits 0 deixam passar.
                const int bytes = 3;
                for (int b = 0; b < bytes; ++b) {
                    const auto bits = Golpe::Byte(s, 4);
                    for (int f = 0; f < 8; ++f) {
                        if (!bits[static_cast<size_t>(f)]) continue;
                        Golpe::Tiro t = Tiro(static_cast<float>(b) * N(0.4f, 0.36f), N(240.0f, 255.0f));
                        t.dx = (static_cast<float>(f) - 3.5f) * 44.0f;
                        t.paralelo = true;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Thiago: {
                // CADA ESCRITA ESPERA NA LINHA, e o COMMIT manda a linha inteira
                // em voce, junta - menos as escritas que o ROLLBACK desfez, que
                // sao as frestas.
                const int escritas = N(9, 10);
                const float intervalo = N(0.09f, 0.08f);
                const float commit = static_cast<float>(escritas - 1) * intervalo + 0.35f;
                std::vector<int> desfeitas;
                for (int i = 0; i < escritas; ++i) desfeitas.push_back(i);
                s.Embaralhar(desfeitas);
                desfeitas.resize(static_cast<size_t>(N(3, 3)));
                for (int i = 0; i < escritas; ++i) {
                    if (std::find(desfeitas.begin(), desfeitas.end(), i) != desfeitas.end()) continue;
                    Golpe::Tiro t = Tiro(static_cast<float>(i) * intervalo, N(250.0f, 270.0f));
                    t.dx = Passo(i, escritas, -200.0f, 200.0f);
                    t.dy = 50.0f;
                    t.espera = commit - t.quando;
                    t.paralelo = true;
                    tiros.push_back(t);
                }
                break;
            }
            case Materia::Luizinho: {
                // OS PONTEIROS saem de lado e apontam para voce: uma pinca.
                // A CURVA E LARGA (uns 140 px de raio): fechada, os ponteiros
                // giravam em volta do Hugo e desciam num bolo.
                std::vector<float> lados = {40.0f, 70.0f, 100.0f};
                if (n == Dificuldade::Nivel::Dificil) lados.push_back(130.0f);
                for (const float l : lados) {
                    for (const float sinal : {-1.0f, 1.0f}) {
                        Golpe::Tiro t = Tiro(0.0f, N(260.0f, 275.0f));
                        t.desvio = sinal * l;
                        t.curva = N(105.0f, 115.0f);
                        t.curvaAte = 2.0f;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Vladimir: {
                // S -> S S, duas vezes: cada semente vira uma arvore de oito.
                const int sementes = 2;
                for (int i = 0; i < sementes; ++i) {
                    Golpe::Tiro t = Tiro(0.0f, N(230.0f, 260.0f));
                    t.desvio = Passo(i, sementes, -22.0f, 22.0f);
                    t.divide = 0.3f;
                    t.geracoes = 2;
                    t.abertura = N(22.0f, 20.0f);
                    t.herdaRumo = true;
                    tiros.push_back(t);
                }
                break;
            }
            case Materia::Julio: {
                // O GRADIENTE: cada passo erra para um lado, e erra menos.
                const int q = N(6, 7);
                for (int i = 0; i < q; ++i) {
                    Golpe::Tiro t = Tiro(static_cast<float>(i) * 0.1f, N(260.0f, 275.0f));
                    t.onda = (i % 2 == 0) ? 90.0f : -90.0f;
                    t.desvio = Passo(i, q, -6.0f, 6.0f);
                    tiros.push_back(t);
                }
                break;
            }
            case Materia::MariaLucia: {
                // notifyAll(): os observadores aparecem juntos e cada um, na
                // vez dele, atira um leque.
                const int olhos = N(4, 5);
                for (int i = 0; i < olhos; ++i) {
                    for (const float d : {-10.0f, 0.0f, 10.0f}) {
                        Golpe::Tiro t = Tiro(0.0f, N(250.0f, 265.0f));
                        t.dx = Passo(i, olhos, -220.0f, 220.0f);
                        t.dy = 40.0f;
                        t.desvio = d;
                        t.espera = 0.6f + 0.12f * static_cast<float>(i);
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Lucas: {
                // O FETCH vai ao servidor, de lado, e a resposta volta em voce.
                // Cada resposta chega numa hora: a de mais longe, depois.
                std::vector<float> lados = {40.0f, 60.0f, 80.0f};
                if (n == Dificuldade::Nivel::Dificil) lados.push_back(100.0f);
                for (size_t i = 0; i < lados.size(); ++i) {
                    for (const float sinal : {-1.0f, 1.0f}) {
                        Golpe::Tiro t = Tiro(0.0f, N(300.0f, 315.0f));
                        t.desvio = sinal * lados[i];
                        t.reMira = N(0.3f, 0.28f) + 0.14f * static_cast<float>(i) + (sinal > 0.0f ? 0.07f : 0.0f);
                        t.curva = 70.0f;      // e a resposta ainda procura quem pediu
                        t.curvaAte = 1.6f;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Vitor: {
                // A JANELA DO TCP: rajadas de 1, 2, 4 e 8, cada uma mirada.
                const std::vector<int> janelas = {1, 2, 4, 8};
                for (size_t j = 0; j < janelas.size(); ++j) {
                    for (int i = 0; i < janelas[j]; ++i) {
                        tiros.push_back(Tiro(0.3f * static_cast<float>(j) + 0.04f * static_cast<float>(i),
                                             N(280.0f, 290.0f)));
                    }
                }
                break;
            }
            case Materia::Generico:
            default: {
                // O HUGO SEM ORIENTADOR: um leque de paginas.
                const int q = N(7, 9);
                for (int i = 0; i < q; ++i) {
                    Golpe::Tiro t = Tiro(0.0f, N(240.0f, 270.0f));
                    t.desvio = Passo(i, q, -35.0f, 35.0f);
                    tiros.push_back(t);
                }
                break;
            }
        }
        // A RODADA DURA o que o ultimo tiro leva para partir.
        r.duracao = 0.1f;
        for (const Golpe::Tiro& t : tiros) r.duracao = std::max(r.duracao, t.quando + t.espera + 0.1f);
        return r;
    }

    std::string Professor(const Materia m) {
        switch (m) {
            case Materia::Andre:      return "André";
            case Materia::Salles:     return "Salles";
            case Materia::Ricardo:    return "Ricardo";
            case Materia::Thiago:     return "Thiago";
            case Materia::Luizinho:   return "Luizinho";
            case Materia::Vladimir:   return "Vladimir";
            case Materia::Julio:      return "Júlio";
            case Materia::MariaLucia: return "Maria Lúcia";
            case Materia::Lucas:      return "Lucas";
            case Materia::Vitor:      return "Vitor";
            default:                  return "Hugo";
        }
    }

    std::string Area(const Materia m) {
        switch (m) {
            case Materia::Andre:      return "Programação";
            case Materia::Salles:     return "Estruturas de Dados";
            case Materia::Ricardo:    return "Organização de Computadores";
            case Materia::Thiago:     return "Banco de Dados";
            case Materia::Luizinho:   return "Linguagens de Programação";
            case Materia::Vladimir:   return "Linguagens Formais e Autômatos";
            case Materia::Julio:      return "Inteligência Artificial";
            case Materia::MariaLucia: return "Engenharia de Software";
            case Materia::Lucas:      return "Programação Web";
            case Materia::Vitor:      return "Programação Concorrente e Distribuída";
            default:                  return "tema livre";
        }
    }

    std::string Capitulo(const Materia m) {
        switch (m) {
            case Materia::Andre:      return "for (i = 0; i < n; i++)";
            case Materia::Salles:     return "push, push, pop: a pilha";
            case Materia::Ricardo:    return "o barramento de 8 bits";
            case Materia::Thiago:     return "BEGIN ... COMMIT";
            case Materia::Luizinho:   return "int *p = &voce;";
            case Materia::Vladimir:   return "S -> S S";
            case Materia::Julio:      return "a descida do gradiente";
            case Materia::MariaLucia: return "notifyAll()";
            case Materia::Lucas:      return "await fetch(\"/api\")";
            case Materia::Vitor:      return "a janela do TCP";
            default:                  return "mais uma página";
        }
    }

    std::string Pergunta(const Materia m) {
        switch (m) {
            case Materia::Andre:      return "Cadê o fluxograma?";
            case Materia::Salles:     return "E se a pilha estourar?";
            case Materia::Ricardo:    return "Quantos ciclos de clock isso leva?";
            case Materia::Thiago:     return "Essa tabela está normalizada?";
            case Materia::Luizinho:   return "Onde esse ponteiro é liberado?";
            case Materia::Vladimir:   return "Isso é decidível?";
            case Materia::Julio:      return "Por que não usou um LLM?";
            case Materia::MariaLucia: return "Cadê a documentação?";
            case Materia::Lucas:      return "Funciona no Safari?";
            case Materia::Vitor:      return "E se der deadlock?";
            default:                  return "Pode repetir?";
        }
    }

    std::string Animacao(const Materia m) {
        switch (m) {
            case Materia::Andre:      return "Andre";
            case Materia::Salles:     return "Salles";
            case Materia::Ricardo:    return "Ricardo";
            case Materia::Thiago:     return "Thiago";
            case Materia::Luizinho:   return "Luizinho";
            case Materia::Vladimir:   return "Vladimir";
            case Materia::Julio:      return "Julio";
            case Materia::MariaLucia: return "MariaLucia";
            case Materia::Lucas:      return "Lucas";
            case Materia::Vitor:      return "Vitor";
            default:                  return "Folha";
        }
    }

    std::array<Materia, 2> Banca(const Materia orientador, Variacao::Sorteio& s) {
        std::vector<Materia> candidatos;
        for (const Materia m : kMaterias) if (m != orientador) candidatos.push_back(m);
        s.Embaralhar(candidatos);
        return {candidatos[0], candidatos[1]};
    }

    Vector2 Mira(const Vector2& de, const float velocidade, const Vector2& aluno, const Vector2& andando,
                 const float adiantamento) {
        if (velocidade <= 0.0f) return aluno;
        const float voo = std::min((aluno - de).Length() / velocidade, kAdiantaAte);
        return aluno + andando * (voo * adiantamento);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreTemas = Dificuldade::Escolher(n, 1.8f, 1.5f);
        d.entrePerguntas = Dificuldade::Escolher(n, 1.4f, 1.2f);
        d.rascunhos = Dificuldade::Escolher(n, 5, 5);
        d.entreAnotacoes = Dificuldade::Escolher(n, 0.6f, 0.45f);
        d.entreCorrecoes = Dificuldade::Escolher(n, 2.4f, 2.1f);
        return d;
    }
}
