//
// A tela que mostra as maiores notas de cada materia.
//

#include "TelaDeRanking.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>
#include <iomanip>
#include <sstream>

#include "../Motor/CaminhosArquivo.h"
#include "../Aluno/FichaArquivo.h"
#include "../Motor/Font.h"
#include "../Motor/Game.h"
#include "../Curso/MateriasArquivo.h"
#include "../Regras/Nota.h"
#include "../Aluno/Ranking.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
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

    /// AS ABAS DAS QUATRO LISTAS, logo abaixo do titulo. Em fracao da largura,
    /// para caberem na tela do gabinete, que e mais estreita que a do computador.
    constexpr float kAlturaDasAbas = 0.185f;
    constexpr float kEntreAbas = 0.21f;
    constexpr float kLarguraDaAba = 0.19f;
    constexpr int kAlturaDaAba = 40;

    /// A aba aberta tem a moldura e o nome em dourado, como a linha escolhida
    /// nas Opcoes; as outras ficam apagadas, mas legiveis.
    const SDL_Color kMolduraDaAba{255, 214, 90, 255};
    const SDL_Color kFundoDaAba{58, 52, 34, 190};
    const SDL_Color kMolduraApagada{235, 235, 235, 70};
    const SDL_Color kFundoApagado{34, 38, 48, 120};
    const Vector3 kNomeApagado(0.62f, 0.64f, 0.70f);

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

    // AS ABAS. Todas a vista, e nao so o nome da lista aberta: quem ve as
    // quatro sabe que elas existem e quantos toques faltam ate a que quer.
    for (int i = 0; i < Ranking::kQuantasListas; ++i) {
        const float x = largura / 2.0f + (static_cast<float>(i) - (Ranking::kQuantasListas - 1) / 2.0f)
                                         * largura * kEntreAbas;
        auto aba = std::make_unique<Actor>(this);
        aba->SetPosition(Vector2(x, altura * kAlturaDasAbas));
        mAbas.push_back(aba->AddComponent<DrawCaixaComponent>(static_cast<int>(largura * kLarguraDaAba),
                                                              kAlturaDaAba, kMolduraApagada, kFundoApagado,
                                                              2, 100));
        AddActor(std::move(aba));
        const auto larguraDoNome = static_cast<int>(largura * kLarguraDaAba) - 16;
        mNomesDasAbas.push_back(Texto(Ranking::NomeDaLista(Ranking::kListas[i]), x, altura * kAlturaDasAbas,
                                      22, larguraDoNome)->GetComponent<DrawTextComponent>());
    }

    // A EXPLICACAO DO GERAL desceu para baixo da lista: o lugar dela, embaixo
    // do titulo, e das abas agora.
    mSubtituloAtor = Texto(" ", largura / 2.0f, altura * 0.80f, 22, 1100);
    if (const auto dc = mSubtituloAtor->GetComponent<DrawTextComponent>()) {
        dc->SetColor(Color::LightBlue);   // e uma nota de rodape, nao um titulo
    }
    mRodapeAtor = Texto(" ", largura / 2.0f, altura * 0.93f, 22, 1000);

    // COMECA NA LISTA DA PARTIDA. Ver o comentario de mLista.
    mLista = Ranking::IndiceDaLista(
        mGame->QuantosJogadores() > 1 ? Progresso::Modo::Dupla : Progresso::Modo::Solo,
        mGame->NivelDeDificuldade());

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

    const Ranking::Lista& lista = ListaAtual();
    const bool duplas = EmDupla();

    // A ABA ABERTA ACENDE. E A UNICA PISTA DE QUAL LISTA ESTA NA TELA, entao
    // nao e enfeite: quatro listas de numeros parecidos sem rotulo fariam o
    // aluno comparar a propria nota do Normal com a de quem jogou no Dificil.
    for (size_t i = 0; i < mAbas.size(); ++i) {
        const bool aberta = static_cast<int>(i) == mLista;
        mAbas[i]->SetMoldura(aberta ? kMolduraDaAba : kMolduraApagada);
        mAbas[i]->SetFundo(aberta ? kFundoDaAba : kFundoApagado);
        mNomesDasAbas[i]->SetColor(aberta ? Color::Gold : kNomeApagado);
    }

    // O GERAL E A MEDIA SOBRE O CURSO INTEIRO, e por isso o titulo diz "media
    // do curso" em vez de "maiores notas": o numero nao e uma nota, e ler 34,55
    // como nota faria todo mundo achar que foi mal.
    if (mPedido.geral) {
        escrever(mTituloAtor, "MEDIA DO CURSO");
        escrever(mSubtituloAtor,
                 "Soma das melhores notas dividida pelas "
                 + std::to_string(materias.Quantas()) + " materias. Nao e uma nota.");
    }
    else {
        const Materias::Materia* m = materias.Por(mPedido.materia);
        escrever(mTituloAtor, "MAIORES NOTAS  -  " + (m ? m->nome : std::string("?")));
        escrever(mSubtituloAtor, "");
    }

    // AS DUAS FORMAS DE LISTA SAO LIDAS, e a que nao esta em uso sai vazia.
    // Montar so a de uma vez pediria dois lacos de desenho quase iguais - e
    // dois lacos quase iguais sao duas chances de um ganhar uma correcao e o
    // outro nao.
    const auto linhas = duplas
        ? std::vector<Ranking::Linha>{}
        : (mPedido.geral ? Ranking::Geral(mFichas, materias.Quantas(), lista.nivel, kQuantasLinhas)
                         : Ranking::DaMateria(mFichas, mPedido.materia, lista.nivel, kQuantasLinhas));

    const auto deDupla = !duplas
        ? std::vector<Ranking::LinhaDeDupla>{}
        : (mPedido.geral ? Ranking::DuplasGeral(mFichas, materias.Quantas(), lista.nivel, kQuantasLinhas)
                         : Ranking::DuplasDaMateria(mFichas, mPedido.materia, lista.nivel, kQuantasLinhas));

    const size_t quantasTem = duplas ? deDupla.size() : linhas.size();

    for (size_t i = 0; i < kQuantasLinhas; ++i) {

        const bool temLinha = i < quantasTem;

        // Posicao, nota e materias jogadas saem IGUAIS das duas formas, e sao
        // lidas aqui de uma vez para o desenho nao se repartir em dois.
        const int  posicao  = temLinha ? (duplas ? deDupla[i].posicao : linhas[i].posicao) : 0;
        const float nota    = temLinha ? (duplas ? deDupla[i].nota : linhas[i].nota) : 0.0f;
        const int  jogadas  = temLinha ? (duplas ? deDupla[i].materiasJogadas
                                                 : linhas[i].materiasJogadas) : 0;

        escrever(mPosicaoAtores[i], temLinha ? std::to_string(posicao) : "");
        escrever(mNotaAtores[i],    temLinha ? ComDuasCasas(nota) : "");

        // Uma das duas colunas de nome fica vazia. Ver mDuplaAtores.
        escrever(mMatriculaAtores[i], (temLinha && !duplas) ? linhas[i].matricula : "");
        escrever(mDuplaAtores[i],
                 (temLinha && duplas) ? deDupla[i].um + kEntreOsDois + deDupla[i].dois : "");

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

    // A LISTA VAZIA DIZ DE QUAL LISTA FALA. "Ninguem jogou esta materia ainda"
    // seria falso com outra lista cheia ao lado, e quem lesse acharia que a
    // lista nao funciona.
    escrever(mVazioAtor, quantasTem > 0 ? "" : "Ninguem tem nota nesta lista ainda.");

    // O MANCHE TROCA DE LISTA, dando a volta. O botao 3 tambem, sem aparecer
    // escrito: era ele que alternava solo e dupla, e quem se acostumou continua
    // chegando a todas as listas com ele.
    escrever(mRodapeAtor, Painel::Rodape({
        Painel::Manche(Painel::Jogador::Um, "trocar de lista"),
        {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"},
    }, mGame->JeitoDoRodape()));

    // A POSICAO DE QUEM ESTA JOGANDO, quando ela nao cabe no topo. Sem isto o
    // ranking so conversa com quem ja esta bem, que e quem menos precisa.
    // CADA LISTA RESPONDE A SUA PERGUNTA. Na de solo "onde estou"; na de duplas
    // "onde estamos", e o sujeito e a DUPLA - dizer ali a posicao de solo seria
    // um numero que nao pertence a nada na tela.
    int posicao = 0;
    std::string quemEsta = "Voce esta";

    if (duplas) {
        // A DUPLA E A DOS DOIS QUE ESTAO JOGANDO AGORA, e nao uma qualquer de
        // que o jogador 1 tenha feito parte: ele pode ter varias, e nenhuma
        // seria mais "a dele" que as outras. Jogando sozinho nao ha dupla de
        // quem perguntar, e o rodape fica vazio - como fica para o visitante,
        // que nao entra em dupla nenhuma.
        if (mGame->QuantosJogadores() > 1) {
            const std::string& um   = mGame->MatriculaDe(0);
            const std::string& dois = mGame->MatriculaDe(1);

            posicao = mPedido.geral
                ? Ranking::PosicaoDaDuplaNoGeral(mFichas, materias.Quantas(), lista.nivel, um, dois)
                : Ranking::PosicaoDaDupla(mFichas, mPedido.materia, lista.nivel, um, dois);

            quemEsta = "Voces estao";
        }
    }
    else {
        // DO JOGADOR 1: e ele quem abre esta tela, do menu ou da selecao de fases.
        const std::string& eu = mGame->MatriculaDe(0);
        if (!eu.empty()) {
            posicao = mPedido.geral
                ? Ranking::PosicaoNoGeral(mFichas, materias.Quantas(), lista.nivel, eu)
                : Ranking::PosicaoDe(mFichas, mPedido.materia, lista.nivel, eu);
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

    // O MANCHE PARA OS LADOS troca de lista, dando a volta; o botao 3 anda
    // para a seguinte (ver o rodape em Redesenhar).
    const bool esquerda = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Esquerda);
    const bool direita = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Direita);
    const bool alternar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Tres);

    int passo = 0;
    if (esquerda && !mEsquerdaAnterior) passo -= 1;
    if ((direita && !mDireitaAnterior) || (alternar && !mAlternarAnterior)) passo += 1;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;
    mAlternarAnterior = alternar;

    if (passo != 0) {
        mLista = Ranking::MoverLista(mLista, passo);

        // AS FICHAS NAO SAO RELIDAS: a lista muda, a turma nao. Reler aqui
        // custaria um passeio pelo disco a cada toque.
        Redesenhar();
    }
}
