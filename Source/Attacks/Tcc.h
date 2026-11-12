//
// O TCC do Hugo: o tema do orientador, a banca e as perguntas dela.
//

#pragma once

#include <array>
#include <string>

#include "../Regras/Dificuldade.h"
#include "../Regras/Golpe.h"
#include "../Regras/Variacao.h"

/// O TCC E o do aluno, na area do orientador.
namespace Tcc {

    using Golpe::Materia;

    constexpr const char* kFaseDoTema = "StateTwo";
    constexpr const char* kFaseDaDefesa = "StateThree";
    constexpr const char* kFaseDasCorrecoes = "StateFinal";
    constexpr const char* kGatilhoDaQuestao = "questao";   ///< um tiro de um padrao
    constexpr const char* kGatilhoDoRascunho = "rascunho"; ///< o rascunho que escorre embaixo dele

    /// O rascunho: enquanto o Hugo para para escrever um capitulo, as paginas do rascunho escorrem
    /// retas embaixo dele.
    constexpr float kEntreRascunhos = 0.1f;
    /// Na defesa e nas correcoes ele nao para.

    /// O padrao 'k' (0 e o primeiro) da materia 'm' contra o aluno. Generico e o do proprio Hugo:
    /// um leque de paginas.
    Golpe::Rodada Padrao(Materia m, int k, Variacao::Sorteio& s, Dificuldade::Nivel n);

    /// "Salles": o nome do professor como o placar o escreve.
    std::string Professor(Materia m);

    /// "Estruturas de Dados": a area do TCC com esse orientador.
    std::string Area(Materia m);

    /// "a pilha": o que o capitulo de cada padrao e.
    std::string Capitulo(Materia m);

    /// "E se a pilha estourar?": a pergunta que esse professor faz na banca.
    std::string Pergunta(Materia m);

    /// O nome da animacao do projetil "Materia" do Hugo: "Salles", ...
    std::string Animacao(Materia m);

    /// A banca: dois professores diferentes um do outro e do orientador (Generico: sem orientador,
    /// qualquer um), sorteados.
    std::array<Materia, 2> Banca(Materia orientador, Variacao::Sorteio& s);

    /// As dez materias com professor, na ordem do enum.
    constexpr std::array<Materia, 10> kMaterias = {
        Materia::Andre, Materia::Salles, Materia::Ricardo, Materia::Thiago, Materia::Luizinho,
        Materia::Vladimir, Materia::Julio, Materia::MariaLucia, Materia::Lucas, Materia::Vitor};

    /// Onde mirar o tiro que sai de 'de' a 'velocidade', no aluno em 'aluno' andando a 'andando'
    /// px/s: onde ele vai estar, se continuar andando.
    Vector2 Mira(const Vector2& de, float velocidade, const Vector2& aluno, const Vector2& andando,
                 float adiantamento);
    constexpr float kAdiantaAte = 1.6f;
    constexpr float kAdiantamentoDaBanca = 0.5f;

    // ---- O relogio ----

    constexpr float kPrimeiro = 0.8f;

    /// O que o Dificil muda: cada padrao vem mais cedo.
    struct Degrau {
        float entreTemas = 1.9f;        ///< um capitulo a cada, no desenvolvimento
        float entrePerguntas = 1.5f;    ///< uma pergunta a cada, alternando a banca
        float entreCorrecoes = 2.2f;    ///< um padrao a cada, nas correcoes
        float adiantamento = 1.0f;      ///< quanto do caminho do aluno a mira adianta
        int rascunhos = 5;              ///< as paginas que escorrem a cada capitulo
        float entreAnotacoes = 0.45f;   ///< na defesa e nas correcoes
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
