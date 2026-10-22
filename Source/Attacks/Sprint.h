//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#pragma once

#include <vector>

#include "../Math.h"

/**
 * CAMADA PURA.
 *
 * Na fase 2 da Maria Lucia (Metodos Ageis) cada etapa e uma SPRINT, com o
 * tempo fixo de um timebox. No comeco dela um CURSOR de mouse pega a pilha de
 * historias do professor e a ARRASTA para o quadro, no meio do campo - e o
 * arrastar-e-soltar da IHC, a outra materia que ela da. Ao soltar, as historias
 * se espalham pelas vagas do quadro, e ficam la ate a REVISAO, no fim da
 * sprint: o que o aluno nao entregou (derrubou a tiro) cai em cima dele.
 *
 * O CURSOR ANDA COMO UMA MAO, e as duas contas sao da IHC:
 *
 *   - QUANTO ele leva e a LEI DE FITTS: T = a + b log2(D/L + 1), com D a
 *     distancia e L a largura do alvo. O alvo e o quadro inteiro, entao a
 *     sprint de oito historias, de quadro largo, e arrastada mais depressa que
 *     a de uma so;
 *   - COMO ele vai e o perfil de MINIMO SOLAVANCO (Flash e Hogan, 1985), o do
 *     braco humano apontando: sai devagar, corre no meio e chega devagar.
 *
 * Quem faz isso no jogo e a SprintAttack, com os dois behaviors da sprint
 * (Behaviors.h); aqui ficam so as contas, cobertas por test_sprint.cpp.
 */
namespace Sprint {

    /// O quadro: uma fileira de vagas abaixo de onde o professor anda e acima
    /// de onde o aluno vive.
    ///
    /// A ALTURA E A DIFICULDADE DA FASE, medida: com o quadro no meio do campo
    /// (450) o codigo das historias chegava ao aluno tao tarde que o piloto
    /// que persegue a professora levou 1 acerto na fase inteira, e todos os
    /// outros vieram da propria professora. A 520, 150 px acima do aluno, sao
    /// as historias que acertam - e o piloto passa nas quatro cadencias de
    /// codigo que foram medidas, enquanto a 500 reprovava em tres.
    struct Quadro {
        float centroX = 600.0f;
        float altura = 520.0f;
        float espaco = 110.0f;   ///< entre o centro de duas vagas
        float vaga = 40.0f;      ///< a largura de uma historia, para a lei de Fitts
    };

    /// As constantes da lei de Fitts, em segundos e em segundos por bit.
    struct Fitts {
        float a = 0.35f;
        float b = 0.18f;
    };

    /// A lei de Fitts: a + b log2(distancia / largura + 1). Largura que nao e
    /// positiva conta como um pixel - o alvo mais dificil que existe.
    float TempoDeFitts(float distancia, float largura, const Fitts& f = {});

    /// O minimo solavanco: 10u^3 - 15u^4 + 6u^5, de 0 a 1, com u preso a [0, 1].
    float SemSolavanco(float u);

    /// A vaga 'i' de 'n', centradas no quadro.
    Vector2 Vaga(int i, int n, const Quadro& q = {});

    /// Onde a historia 'i' fica na pilha, em relacao a ponta do cursor. A ponta
    /// segura a pilha pelo canto de baixo e da direita: assim a seta, que sai
    /// da ponta para baixo e para a direita, fica fora dos cartoes.
    Vector2 NaPilha(int i);

    /// Onde fica o centro do desenho do cursor quando a ponta esta em 'ponta'.
    /// A ponta da seta esta no pixel (6, 4) do quadro de 32.
    Vector2 CentroDoCursor(Vector2 ponta, float escala);

    /// Tudo o que uma sprint vai fazer, decidido no disparo.
    struct Plano {
        Vector2 origem;                 ///< onde o cursor pega a pilha: o professor
        Vector2 soltura;                ///< onde solta: o meio do quadro
        std::vector<Vector2> vagas;
        float arraste = 0.0f;           ///< quanto o arraste leva (Fitts)
        float espalha = 0.0f;           ///< quanto as historias levam ate as vagas
        float revisao = 0.0f;           ///< quando o que sobrou cai
    };

    /// Quanto as historias levam da pilha ate as vagas, depois de soltas.
    constexpr float kEspalha = 0.35f;

    /// O clique da revisao comeca este tanto antes dela: e o aviso.
    constexpr float kAvisoDaRevisao = 0.4f;

    /// A revisao de um ataque que nao escreveu "revisao" no bloco "sprint".
    constexpr float kRevisaoPadrao = 2.5f;

    /// O POSTO das historias paradas no quadro (BossProjectile::DefinirPosto):
    /// e dele que a PontosAttack atira, e a historia em andamento solta o
    /// codigo dela. Tem de ser um dos nomes de PostoExiste (FasesDeAtaque.h).
    constexpr const char* kPostoDoQuadro = "quadro";

    /// O plano de uma sprint de 'n' historias saindo de 'origem'. A revisao
    /// nunca vem antes de as historias chegarem as vagas: se 'revisao' for
    /// menor que o arraste mais o espalhar, ela vira exatamente isso.
    Plano Planejar(Vector2 origem, int n, float revisao, const Quadro& q = {}, const Fitts& f = {});

    /// Onde a ponta do cursor esta 't' segundos depois do disparo.
    Vector2 PontaDoCursor(const Plano& p, float t);

    /// Onde a historia 'i' esta 't' segundos depois do disparo, ate a revisao.
    /// Depois dela quem a move e a queda (o behavior), e isto devolve a vaga.
    Vector2 PosicaoDaHistoria(const Plano& p, int i, float t);

    /// O que o cursor esta fazendo: a cara dele e quando ele some.
    enum class Gesto { Arrastando, Soltando, Esperando, Clicando, Acabou };
    Gesto GestoDoCursor(const Plano& p, float t);
}
