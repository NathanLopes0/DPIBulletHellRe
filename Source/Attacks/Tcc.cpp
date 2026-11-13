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
                // For (i = 0; i < n; i++): um tiro por volta, e o laco varre o arco.
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
                // Push, push, push... e o pop comeca pelo ultimo.
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
                // O barramento: paredes de oito fios, e so os bits 0 deixam passar.
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
                // Cada escrita espera na linha, e o commit manda a linha inteira em voce, junta.
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
                // Os ponteiros saem de lado e apontam para voce: uma pinca.
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
                // O gradiente: cada passo erra para um lado, e erra menos.
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
                // NotifyAll(): os observadores aparecem juntos e cada um, na vez dele, atira um leque.
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
                // O fetch vai ao servidor, de lado, e a resposta volta em voce.
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
                // A janela do TCP: rajadas de 1, 2, 4 e 8, cada uma mirada.
                const std::vector<int> janelas = {1, 2, 4, 8};
                for (size_t j = 0; j < janelas.size(); ++j) {
                    for (int i = 0; i < janelas[j]; ++i) {
                        tiros.push_back(Tiro(0.3f * static_cast<float>(j) + 0.04f * static_cast<float>(i),
                                             N(280.0f, 290.0f)));
                    }
                }
                break;
            }
            case Materia::Arroyo: {
                // T(n) = T(n/2) + n: leques de 8, 4, 2 e 1 tiros, cada um com a metade do anterior.
                for (int nivel = 0; nivel < 4; ++nivel) {
                    const int q = 8 >> nivel;
                    const float abertura = N(36.0f, 42.0f) / static_cast<float>(1 << nivel);
                    for (int i = 0; i < q; ++i) {
                        Golpe::Tiro t = Tiro(0.28f * static_cast<float>(nivel), N(255.0f, 275.0f));
                        t.desvio = q > 1 ? Passo(i, q, -abertura, abertura) : 0.0f;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Goulart: {
                // O Round Robin: tres processos revezando a CPU - uma rajada curta de cada um, e o
                // seguinte vem de outro lado.
                for (int q = 0; q < 6; ++q) {
                    const float lado = static_cast<float>(q % 3 - 1) * N(18.0f, 24.0f);
                    for (int i = 0; i < 3; ++i) {
                        Golpe::Tiro t = Tiro(0.22f * static_cast<float>(q) + 0.05f * static_cast<float>(i), N(260.0f, 280.0f));
                        t.desvio = lado;
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Daniel: {
                // As k-medias: tres grupos de tiros, cada um num rumo - o centroide -, com o
                // espalhamento do grupo em volta dele.
                for (int g = 0; g < 3; ++g) {
                    const float centro = static_cast<float>(g - 1) * N(22.0f, 28.0f);
                    for (int i = 0; i < 5; ++i) {
                        Golpe::Tiro t = Tiro(0.12f * static_cast<float>(g) + 0.03f * static_cast<float>(i), N(250.0f, 270.0f));
                        t.desvio = centro + Passo(i, 5, -6.0f, 6.0f);
                        tiros.push_back(t);
                    }
                }
                break;
            }
            case Materia::Generico:
            default: {
                // O Hugo sem orientador: um leque de paginas.
                const int q = N(7, 9);
                for (int i = 0; i < q; ++i) {
                    Golpe::Tiro t = Tiro(0.0f, N(240.0f, 270.0f));
                    t.desvio = Passo(i, q, -35.0f, 35.0f);
                    tiros.push_back(t);
                }
                break;
            }
        }
        // A rodada dura o que o ultimo tiro leva para partir.
        r.duracao = 0.1f;
        for (const Golpe::Tiro& t : tiros) r.duracao = std::max(r.duracao, t.quando + t.espera + 0.1f);
        return r;
    }

    namespace {
        /// Uma linha por materia: o professor, a area do TCC, o capitulo, a pergunta da banca e a
        /// animacao do projetil.
        struct Ficha {
            Materia materia;
            const char* professor;
            const char* area;
            const char* capitulo;
            const char* pergunta;
            const char* animacao;
        };
        const Ficha kFichas[] = {
            {Materia::Andre,      "André",       "Programação",
             "for (i = 0; i < n; i++)",  "Cadê o fluxograma?",                "Andre"},
            {Materia::Salles,     "Salles",      "Estruturas de Dados",
             "push, push, pop: a pilha", "E se a pilha estourar?",            "Salles"},
            {Materia::Ricardo,    "Ricardo",     "Organização de Computadores",
             "o barramento de 8 bits",   "Quantos ciclos de clock isso leva?", "Ricardo"},
            {Materia::Thiago,     "Thiago",      "Banco de Dados",
             "BEGIN ... COMMIT",         "Essa tabela está normalizada?",     "Thiago"},
            {Materia::Luizinho,   "Luizinho",    "Linguagens de Programação",
             "int *p = &voce;",          "Onde esse ponteiro é liberado?",    "Luizinho"},
            {Materia::Vladimir,   "Vladimir",    "Linguagens Formais e Autômatos",
             "S -> S S",                 "Isso é decidível?",                 "Vladimir"},
            {Materia::Julio,      "Júlio",       "Inteligência Artificial",
             "a descida do gradiente",   "Por que não usou um LLM?",          "Julio"},
            {Materia::MariaLucia, "Maria Lúcia", "Engenharia de Software",
             "notifyAll()",              "Cadê a documentação?",              "MariaLucia"},
            {Materia::Lucas,      "Lucas",       "Programação Web",
             "await fetch(\"/api\")",    "Funciona no Safari?",               "Lucas"},
            {Materia::Vitor,      "Vitor",       "Programação Concorrente e Distribuída",
             "a janela do TCP",          "E se der deadlock?",                "Vitor"},
            {Materia::Arroyo,     "Arroyo",      "Projeto e Análise de Algoritmos",
             "T(n) = 2T(n/2) + n",       "Qual é a complexidade disso?",      "Arroyo"},
            {Materia::Goulart,    "Goulart",     "Sistemas Operacionais",
             "Round Robin, q = 1,2 s",   "Quanto custa uma troca de contexto?", "Goulart"},
            {Materia::Daniel,     "Daniel",      "Ciência de Dados",
             "k-means, k = 3",           "Cadê o conjunto de teste?",           "Daniel"},
        };

        /// Sem orientador (o Generico), o proprio Hugo: tema livre.
        const Ficha kDoHugo{Materia::Generico, "Hugo", "tema livre", "mais uma página", "Pode repetir?", "Folha"};

        const Ficha& FichaDe(const Materia m) {
            for (const Ficha& f : kFichas) if (f.materia == m) return f;
            return kDoHugo;
        }
    }

    std::string Professor(const Materia m) { return FichaDe(m).professor; }
    std::string Area(const Materia m) { return FichaDe(m).area; }
    std::string Capitulo(const Materia m) { return FichaDe(m).capitulo; }
    std::string Pergunta(const Materia m) { return FichaDe(m).pergunta; }
    std::string Animacao(const Materia m) { return FichaDe(m).animacao; }

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
