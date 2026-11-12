//
// A fase 1 do Hugo: a proposta, e o cronograma dela.
//

#pragma once

#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// Toda proposta de TCC tem um cronograma, e o Hugo o desenha no alto do campo: um grafico de
/// Gantt, kLinhas linhas, e cada tarefa.
namespace Cronograma {

    constexpr const char* kFaseDaProposta = "StateOne";
    constexpr const char* kPostoDoCronograma = "cronograma";
    constexpr const char* kGatilhoDaTarefa = "tarefa";     ///< uma pagina de uma barra

    constexpr int kLinhas = 4;
    constexpr float kYDaPrimeiraLinha = 205.0f;
    constexpr float kEntreLinhas = 45.0f;
    constexpr float kMargem = 80.0f;          ///< o grafico vai de kMargem a largura - kMargem
    constexpr float kMenor = 180.0f;          ///< a barra mais curta
    constexpr float kMaior = 360.0f;
    constexpr float kFolgaNaLinha = 40.0f;    ///< entre duas barras da mesma linha
    constexpr float kSeparacao = 110.0f;      ///< entre os prazos de duas barras quaisquer
    constexpr int kSemanas = 16;              ///< o grafico inteiro, em semanas

    struct Tarefa {
        int linha = 0;
        float de = 0.0f;
        float ate = 0.0f;
        std::string nome;
    };

    /// O que o Dificil muda: mais tarefas, o hoje mais rapido, mais paginas por
    /// barra e a queda mais rapida.
    struct Degrau {
        int tarefas = 5;
        float varredura = 4.2f;       ///< segundos para o hoje atravessar o grafico
        float entrePaginas = 40.0f;
        float queda = 250.0f;         ///< px/s
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// Um cronograma novo numa janela de 'largura', em ordem de prazo.
    std::vector<Tarefa> Gerar(Variacao::Sorteio& s, float largura, const Degrau& d);

    [[nodiscard]] float YDaLinha(int linha);

    /// Onde o hoje esta 't' segundos depois de comecar a varrer.
    float XDoHoje(float t, float largura, const Degrau& d);

    /// O x de cada pagina da barra.
    std::vector<float> Paginas(const Tarefa& t, const Degrau& d);

    /// A semana em que 'x' cai no grafico, de 1 a kSemanas.
    int Semana(float x, float largura);

    /// "Revisao bibliografica\nvenceu na semana 7": o placar de uma queda.
    std::string Placar(const Tarefa& t, float largura);

    constexpr float kPlanejando = 0.7f;        ///< o grafico aparece antes de o hoje andar
    constexpr float kEntreCronogramas = 0.5f;
}
