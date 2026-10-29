//
// A tela que mostra as maiores notas de cada materia.
//

#include "TelaDeRanking.h"
#include "../Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>
#include <iomanip>
#include <sstream>

#include "../CaminhosArquivo.h"
#include "../FichaArquivo.h"
#include "../Font.h"
#include "../Game.h"
#include "../MateriasArquivo.h"
#include "../Nota.h"
#include "../Ranking.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// Quantas posicoes cabem na tela. O resto da lista existe, so nao e exibido:
    /// quem nao esta aqui ve a propria posicao no rodape.
    constexpr size_t kQuantasLinhas = 8;

    constexpr float kPrimeiraLinha      = 0.275f;
    constexpr float kEspacoEntreLinhas  = 0.067f;

    /// As colunas, em fracao da largura.
    constexpr float kColunaPosicao   = 0.26f;
    constexpr float kColunaMatricula = 0.38f;
    constexpr float kColunaJogadas   = 0.55f;   // so no geral
    constexpr float kColunaNota      = 0.72f;

    /// A coluna da dupla ocupa o lugar da matricula, e e mais larga: cabem duas
    /// matriculas e o "+" entre elas.
    constexpr float kColunaDupla  = 0.355f;
    constexpr int   kLarguraDupla = 300;
    constexpr int   kTamanhoDupla = 26;

    /// " e ", E NAO " + ". A Zelda.ttf nao tem o glifo "+" e desenha um quadrado
    /// vazio no lugar - o mesmo tropeco do "_" na tela de matricula e do ";" na
    /// criacao de personagem. Medido na foto: "89384 + 90112" saiu com um
    /// retangulo no meio. E em portugues "89384 e 90112" e como se le uma dupla
    /// em voz alta, entao a troca nao e so um remendo.
    const std::string kEntreOsDois = " e ";

    std::string ComDuasCasas(const float nota) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(2) << nota;
        return s.str();
    }
}

TelaDeRanking::TelaDeRanking(Game* game)
    : Scene(game, SceneType::Ranking),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void TelaDeRanking::Load() {

    CriarFundoDoCorredor(*this);

    const auto altura  = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    mPedido = mGame->RankingPedido();

    const Materias::Lista& materias = Materias::Carregadas();

    // AS FICHAS SAO LIDAS UMA VEZ. Ver o comentario de classe: reler a cada
    // quadro custaria um passeio pelo disco inteiro por quadro.
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        mFichas.push_back(f);
    }

    mTituloAtor    = Texto(" ", largura / 2.0f, altura * 0.11f, 44, 900);
    mSubtituloAtor = Texto(" ", largura / 2.0f, altura * 0.185f, 22, 1100);
    if (const auto dc = mSubtituloAtor->GetComponent<DrawTextComponent>()) {
        dc->SetColor(Color::LightBlue);   // e uma nota de rodape, nao um titulo
    }
    mRodapeAtor = Texto(" ", largura / 2.0f, altura * 0.93f, 22, 1000);

    // COMECA NO MODO EM QUE SE ESTA JOGANDO. Ver o comentario de mDuplas.
    mDuplas = mGame->QuantosJogadores() > 1;

    // As linhas sao criadas uma vez e reescritas na troca de materia. Criar e
    // destruir ator a cada troca deixaria lixo na cena, que so some no fim dela.
    for (size_t i = 0; i < kQuantasLinhas; ++i) {
        const float y = altura * (kPrimeiraLinha + kEspacoEntreLinhas * static_cast<float>(i));
        mPosicaoAtores.push_back  (Texto(" ", largura * kColunaPosicao,   y, 30, 120, true));
        mMatriculaAtores.push_back(Texto(" ", largura * kColunaMatricula, y, 30, 260, true));
        mDuplaAtores.push_back    (Texto(" ", largura * kColunaDupla, y,
                                         kTamanhoDupla, kLarguraDupla, true));
        mJogadasAtores.push_back  (Texto(" ", largura * kColunaJogadas,  y, 24, 220, true));
        mNotaAtores.push_back     (Texto(" ", largura * kColunaNota,      y, 30, 220, true));
    }

    // Duas mensagens que ocupam o lugar da lista quando ela nao serve.
    mVazioAtor = Texto(" ", largura / 2.0f, altura * 0.45f, 28, 1000);
    mSuaPosicaoAtor = Texto(" ", largura / 2.0f, altura * 0.86f, 26, 1000);

    Redesenhar();
}

Actor* TelaDeRanking::Texto(const std::string& conteudo, const float x, const float y,
                            const int tamanho, const int larguraMaxima, const bool aEsquerda) {

    auto ator = std::make_unique<Actor>(this);

    // Uma coluna alinhada a esquerda e desenhada a partir do x dado, e nao
    // centrada nele: com tudo centrado, os numeros dancam de lado conforme o
    // tamanho muda e a coluna deixa de parecer uma coluna.
    ator->SetPosition(Vector2(aEsquerda ? x + static_cast<float>(larguraMaxima) / 2.0f : x, y));

    auto dc = ator->AddComponent<DrawTextComponent>(conteudo, mFonte.get(),
                                                    larguraMaxima, tamanho + 8, tamanho, 255);

    // A QUEBRA ACOMPANHA A LARGURA DECLARADA. O padrao de 500 px quebrava a
    // frase explicativa em duas linhas, e com o ajuste ao texto ligado as duas
    // ainda eram encolhidas para caber na altura de uma - saia ilegivel.
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void TelaDeRanking::Redesenhar() {

    const Materias::Lista& materias = Materias::Carregadas();

    auto escrever = [](Actor* ator, const std::string& texto) {
        if (!ator) return;
        if (const auto dc = ator->GetComponent<DrawTextComponent>()) {
            // Espaco, e nao vazio: texto vazio nao gera textura.
            dc->SetText(texto.empty() ? " " : texto);
        }
    };
    auto pintar = [](Actor* ator, const Vector3& cor) {
        if (!ator) return;
        if (const auto dc = ator->GetComponent<DrawTextComponent>()) dc->SetColor(cor);
    };

    // O GERAL E A MEDIA SOBRE O CURSO INTEIRO, e por isso o titulo diz "media
    // do curso" em vez de "maiores notas": o numero nao e uma nota, e ler 34,55
    // como nota faria todo mundo achar que foi mal.
    // O SUFIXO DO TITULO E A UNICA PISTA DE QUAL LISTA ESTA ABERTA, entao ele
    // nao e enfeite: duas listas de numeros parecidos sem rotulo fariam o aluno
    // comparar a propria nota de solo com a de uma dupla.
    const std::string emDupla = mDuplas ? "  EM DUPLA" : "";

    if (mPedido.geral) {
        escrever(mTituloAtor, "MEDIA DO CURSO" + emDupla);
        escrever(mSubtituloAtor,
                 "Soma das melhores notas dividida pelas "
                 + std::to_string(materias.Quantas()) + " materias. Nao e uma nota.");
    }
    else {
        const Materias::Materia* m = materias.Por(mPedido.materia);
        escrever(mTituloAtor, "MAIORES NOTAS" + emDupla + "  -  " + (m ? m->nome : std::string("?")));
        escrever(mSubtituloAtor, "");
    }

    // AS DUAS LISTAS SAO LIDAS, e a que nao esta em uso sai vazia. Montar so a
    // de uma vez pediria dois lacos de desenho quase iguais - e dois lacos
    // quase iguais sao duas chances de um ganhar uma correcao e o outro nao.
    const auto linhas = mDuplas
        ? std::vector<Ranking::Linha>{}
        : (mPedido.geral ? Ranking::Geral(mFichas, materias.Quantas(), kQuantasLinhas)
                         : Ranking::DaMateria(mFichas, mPedido.materia, kQuantasLinhas));

    const auto duplas = !mDuplas
        ? std::vector<Ranking::LinhaDeDupla>{}
        : (mPedido.geral ? Ranking::DuplasGeral(mFichas, materias.Quantas(), kQuantasLinhas)
                         : Ranking::DuplasDaMateria(mFichas, mPedido.materia, kQuantasLinhas));

    const size_t quantasTem = mDuplas ? duplas.size() : linhas.size();

    for (size_t i = 0; i < kQuantasLinhas; ++i) {

        const bool temLinha = i < quantasTem;

        // Posicao, nota e materias jogadas saem IGUAIS das duas listas, e sao
        // lidas aqui de uma vez para o desenho nao se repartir em dois.
        const int  posicao  = temLinha ? (mDuplas ? duplas[i].posicao : linhas[i].posicao) : 0;
        const float nota    = temLinha ? (mDuplas ? duplas[i].nota : linhas[i].nota) : 0.0f;
        const int  jogadas  = temLinha ? (mDuplas ? duplas[i].materiasJogadas
                                                  : linhas[i].materiasJogadas) : 0;

        escrever(mPosicaoAtores[i], temLinha ? std::to_string(posicao) : "");
        escrever(mNotaAtores[i],    temLinha ? ComDuasCasas(nota) : "");

        // Uma das duas colunas de nome fica vazia. Ver mDuplaAtores.
        escrever(mMatriculaAtores[i], (temLinha && !mDuplas) ? linhas[i].matricula : "");
        escrever(mDuplaAtores[i],
                 (temLinha && mDuplas) ? duplas[i].um + kEntreOsDois + duplas[i].dois : "");

        // So no geral: no ranking de uma materia a contagem seria sempre "1 de
        // 11" e nao diria nada.
        escrever(mJogadasAtores[i],
                 (temLinha && mPedido.geral)
                     ? std::to_string(jogadas) + " de " + std::to_string(materias.Quantas())
                     : "");

        // O DOURADO SEGUE A MESMA REGRA DA BATALHA. 99,99 e 100 sao resultados
        // diferentes, e e aqui que essa diferenca finalmente aparece para quem
        // joga - era disso que a curva da nota tratava desde o comeco.
        // O DOURADO SO VALE NO RANKING DE MATERIA. No geral o numero e uma media
        // sobre o curso inteiro, entao 100 exigiria nota cheia em TODAS as
        // materias - pintar por esse criterio seria uma cor que nunca acende.
        const bool cheia = temLinha && !mPedido.geral && Nota::ECheia(nota);
        const Vector3 cor = cheia ? Color::Gold : Color::White;
        pintar(mPosicaoAtores[i], cor);
        pintar(mMatriculaAtores[i], cor);
        pintar(mDuplaAtores[i], cor);
        pintar(mNotaAtores[i], cor);
        pintar(mJogadasAtores[i], Color::LightBlue);
    }

    // A LISTA DE DUPLAS VAZIA DIZ OUTRA COISA. "Ninguem jogou esta materia
    // ainda" seria falso com o ranking de solo cheio ao lado, e quem lesse
    // acharia que a lista nao funciona.
    escrever(mVazioAtor, quantasTem > 0 ? ""
        : mDuplas
            ? (mPedido.geral ? "Ninguem jogou em dupla ainda."
                             : "Ninguem jogou esta materia em dupla ainda.")
            : (mPedido.geral ? "Ninguem jogou ainda." : "Ninguem jogou esta materia ainda."));

    // O RODAPE ANUNCIA PARA ONDE O BOTAO LEVA, e nao onde se esta: "ver duplas"
    // enquanto o solo esta aberto. Anunciar a lista atual faria o aluno apertar
    // esperando continuar onde estava.
    escrever(mRodapeAtor, Painel::Rodape({
        {Painel::Jogador::Um, Painel::Botao::Tres, mDuplas ? "ver solo" : "ver duplas"},
        {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"},
    }, mGame->JeitoDoRodape()));

    // A POSICAO DE QUEM ESTA JOGANDO, quando ela nao cabe no topo. Sem isto o
    // ranking so conversa com quem ja esta bem, que e quem menos precisa.
    // CADA LISTA RESPONDE A SUA PERGUNTA. Na de solo "onde estou"; na de duplas
    // "onde estamos", e o sujeito e a DUPLA - dizer ali a posicao de solo seria
    // um numero que nao pertence a nada na tela.
    int posicao = 0;
    std::string quemEsta = "Voce esta";

    if (mDuplas) {
        // A DUPLA E A DOS DOIS QUE ESTAO JOGANDO AGORA, e nao uma qualquer de
        // que o jogador 1 tenha feito parte: ele pode ter varias, e nenhuma
        // seria mais "a dele" que as outras. Jogando sozinho nao ha dupla de
        // quem perguntar, e o rodape fica vazio - como fica para o visitante,
        // que nao entra em dupla nenhuma.
        if (mGame->QuantosJogadores() > 1) {
            const std::string& um   = mGame->MatriculaDe(0);
            const std::string& dois = mGame->MatriculaDe(1);

            posicao = mPedido.geral
                ? Ranking::PosicaoDaDuplaNoGeral(mFichas, materias.Quantas(), um, dois)
                : Ranking::PosicaoDaDupla(mFichas, mPedido.materia, um, dois);

            quemEsta = "Voces estao";
        }
    }
    else {
        // DO JOGADOR 1: e ele quem abre esta tela, do menu ou da selecao de fases.
        const std::string& eu = mGame->MatriculaDe(0);
        if (!eu.empty()) {
            posicao = mPedido.geral
                ? Ranking::PosicaoNoGeral(mFichas, materias.Quantas(), eu)
                : Ranking::PosicaoDe(mFichas, mPedido.materia, eu);
        }
    }

    // SO QUANDO NAO CABE NO TOPO: quem ja esta na lista esta vendo a propria
    // posicao, e repeti-la embaixo nao acrescenta nada.
    escrever(mSuaPosicaoAtor,
             posicao > static_cast<int>(kQuantasLinhas)
                 ? quemEsta + " em " + std::to_string(posicao) + "o"
                 : "");
}

void TelaDeRanking::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        // VOLTA PARA ONDE VEIO, e nao sempre para o menu: aberta pela selecao de
        // fases, cair no menu faria o aluno refazer o caminho ate a materia.
        mGame->RequestSceneChange(mPedido.voltarPara);
        return;
    }
    mVoltarAnterior = voltar;

    // O MESMO BOTAO VAI E VOLTA. Dois botoes - um para cada lista - gastariam o
    // ultimo botao livre do painel e exigiriam saber em qual se esta para
    // escolher qual apertar.
    const bool alternar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Tres);
    if (alternar && !mAlternarAnterior) {
        mDuplas = !mDuplas;

        // AS FICHAS NAO SAO RELIDAS: a lista muda, a turma nao. Reler aqui
        // custaria um passeio pelo disco a cada aperto.
        Redesenhar();
    }
    mAlternarAnterior = alternar;
}
