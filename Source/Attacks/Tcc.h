//
// O TCC do Hugo: o tema do orientador, a banca e as perguntas dela.
//

#pragma once

#include <array>
#include <string>

#include "../Dificuldade.h"
#include "../Golpe.h"
#include "../Variacao.h"

/**
 * CAMADA PURA. Coberto por test_tcc.cpp.
 *
 * O TCC E O DO ALUNO, NA AREA DO ORIENTADOR. Quem ele escolheu antes da batalha
 * (EscolhaDoOrientador) luta do lado dele com o golpe da materia (Golpe.h) - e
 * o Hugo usa a MESMA materia contra ele: no desenvolvimento, cada capitulo do
 * TCC e um padrao dela (Padrao), com os projeteis dela. Na defesa, a BANCA sao
 * dois outros professores (Banca), e cada pergunta de cada um e o padrao da
 * materia dele, saindo de onde ele esta sentado.
 *
 * OS PADROES SAO OS GOLPES DO ORIENTADOR DO OUTRO LADO, mais densos: o for do
 * Andre varre um arco, a pilha do Salles desempilha leques, o barramento do
 * Ricardo vira paredes com os bits 0 de fresta, a transacao do Thiago
 * converge no COMMIT, os ponteiros do Luizinho fecham uma pinca, a gramatica
 * do Vladimir abre uma arvore, o gradiente do Julio oscila ate convergir em
 * voce, os observadores da Maria Lucia sao notificados um depois do outro, o
 * fetch do Lucas volta de lado e a janela do Vitor cresce 1, 2, 4, 8.
 *
 * O ALVO E O ALUNO e o lugar e relativo a quem atira (o Hugo, ou o professor
 * da banca); o resto do voo e o de Golpe::Avancar.
 */
namespace Tcc {

    using Golpe::Materia;

    constexpr const char* kFaseDoTema = "StateTwo";
    constexpr const char* kFaseDaDefesa = "StateThree";
    constexpr const char* kFaseDasCorrecoes = "StateFinal";
    constexpr const char* kGatilhoDaQuestao = "questao";   ///< um tiro de um padrao
    constexpr const char* kGatilhoDoRascunho = "rascunho"; ///< o rascunho que escorre embaixo dele

    /// O RASCUNHO: enquanto o Hugo para para escrever um capitulo, as paginas
    /// do rascunho escorrem retas embaixo dele - parado, ele e facil de
    /// acertar, e ficar embaixo dele custa.
    constexpr float kEntreRascunhos = 0.1f;
    /// NA DEFESA E NAS CORRECOES ele nao para: as anotacoes escorrem dele o
    /// tempo todo, uma pagina a cada tanto (Degrau::entreAnotacoes), e caem
    /// onde ele ESTAVA - em quem vem atras dele, embaixo.

    /// @brief O padrao 'k' (0 e o primeiro) da materia 'm' contra o aluno.
    /// Generico e o do proprio Hugo: um leque de paginas.
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

    /// @brief A banca: dois professores diferentes um do outro e do orientador
    /// (Generico: sem orientador, qualquer um), sorteados.
    std::array<Materia, 2> Banca(Materia orientador, Variacao::Sorteio& s);

    /// As dez materias com professor, na ordem do enum.
    constexpr std::array<Materia, 10> kMaterias = {
        Materia::Andre, Materia::Salles, Materia::Ricardo, Materia::Thiago, Materia::Luizinho,
        Materia::Vladimir, Materia::Julio, Materia::MariaLucia, Materia::Lucas, Materia::Vitor};

    /// @brief Onde mirar o tiro que sai de 'de' a 'velocidade', no aluno em
    /// 'aluno' andando a 'andando' px/s: ONDE ELE VAI ESTAR, se continuar
    /// andando - uma fracao 'adiantamento' do caminho, e no maximo kAdiantaAte
    /// segundos a frente. Quem anda sempre do mesmo jeito e achado.
    Vector2 Mira(const Vector2& de, float velocidade, const Vector2& aluno, const Vector2& andando,
                 float adiantamento);
    constexpr float kAdiantaAte = 1.6f;
    constexpr float kAdiantamentoDaBanca = 0.5f;

    // ---- O RELOGIO ---------------------------------------------------------------

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
