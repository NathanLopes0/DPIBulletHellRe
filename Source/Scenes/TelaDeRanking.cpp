//
// A tela que mostra as maiores notas de cada materia.
//

#include "TelaDeRanking.h"
#include "TextoDeCena.h"
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

    /// " e ", e nao " + ".
    const std::string kEntreOsDois = " e ";

    /// As abas das quatro listas, logo abaixo do titulo. Em fracao da largura, para caberem na tela
    /// do gabinete, que e mais estreita que a do computador.
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

    // As fichas sao lidas uma vez.
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        mFichas.push_back(f);
    }

    mTituloAtor    = Texto(" ", largura / 2.0f, altura * 0.11f, 44, 900);

    // As abas.
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

    mSubtituloAtor = Texto(" ", largura / 2.0f, altura * 0.80f, 22, 1100);
    if (const auto dc = mSubtituloAtor->GetComponent<DrawTextComponent>()) {
        dc->SetColor(Color::LightBlue);   // e uma nota de rodape, nao um titulo
    }
    mRodapeAtor = Texto(" ", largura / 2.0f, altura * 0.93f, 22, 1000);

    // Comeca na lista da partida.
    mLista = Ranking::IndiceDaLista(
        mGame->QuantosJogadores() > 1 ? Progresso::Modo::Dupla : Progresso::Modo::Solo,
        mGame->NivelDeDificuldade());

    // As linhas sao criadas uma vez e reescritas na troca de materia.
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
    // Uma coluna alinhada a esquerda e desenhada a partir do x dado, e nao centrada nele.
    const Vector2 onde(aEsquerda ? x + static_cast<float>(larguraMaxima) / 2.0f : x, y);
    return CriarTexto(*this, mFonte.get(), conteudo, onde, tamanho, larguraMaxima, tamanho + 8)->GetOwner();
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

    // A aba aberta acende.
    for (size_t i = 0; i < mAbas.size(); ++i) {
        const bool aberta = static_cast<int>(i) == mLista;
        mAbas[i]->SetMoldura(aberta ? kMolduraDaAba : kMolduraApagada);
        mAbas[i]->SetFundo(aberta ? kFundoDaAba : kFundoApagado);
        mNomesDasAbas[i]->SetColor(aberta ? Color::Gold : kNomeApagado);
    }

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

    // As duas formas de lista sao lidas, e a que nao esta em uso sai vazia.
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

        const int  posicao  = temLinha ? (duplas ? deDupla[i].posicao : linhas[i].posicao) : 0;
        const float nota    = temLinha ? (duplas ? deDupla[i].nota : linhas[i].nota) : 0.0f;
        const int  jogadas  = temLinha ? (duplas ? deDupla[i].materiasJogadas
                                                 : linhas[i].materiasJogadas) : 0;

        escrever(mPosicaoAtores[i], temLinha ? std::to_string(posicao) : "");
        escrever(mNotaAtores[i],    temLinha ? ComDuasCasas(nota) : "");

        // Uma das duas colunas de nome fica vazia.
        escrever(mMatriculaAtores[i], (temLinha && !duplas) ? linhas[i].matricula : "");
        escrever(mDuplaAtores[i],
                 (temLinha && duplas) ? deDupla[i].um + kEntreOsDois + deDupla[i].dois : "");

        // So no geral: no ranking de uma materia a contagem seria sempre "1 de
        // 11" e nao diria nada.
        escrever(mJogadasAtores[i],
                 (temLinha && mPedido.geral)
                     ? std::to_string(jogadas) + " de " + std::to_string(materias.Quantas())
                     : "");

        // O dourado so vale no ranking de materia.
        const bool cheia = temLinha && !mPedido.geral && Nota::ECheia(nota);
        const Vector3 cor = cheia ? Color::Gold : Color::White;
        pintar(mPosicaoAtores[i], cor);
        pintar(mMatriculaAtores[i], cor);
        pintar(mDuplaAtores[i], cor);
        pintar(mNotaAtores[i], cor);
        pintar(mJogadasAtores[i], Color::LightBlue);
    }

    // A lista vazia diz de qual lista fala.
    escrever(mVazioAtor, quantasTem > 0 ? "" : "Ninguem tem nota nesta lista ainda.");

    // O manche troca de lista, dando a volta.
    escrever(mRodapeAtor, Painel::Rodape({
        Painel::Manche(Painel::Jogador::Um, "trocar de lista"),
        {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"},
    }, mGame->JeitoDoRodape()));

    // A posicao de quem esta jogando, quando ela nao cabe no topo.
    int posicao = 0;
    std::string quemEsta = "Voce esta";

    if (duplas) {
        // Jogando sozinho nao ha dupla de quem perguntar, e o rodape fica vazio.
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
        // Do jogador 1: e ele quem abre esta tela, do menu ou da selecao de fases.
        const std::string& eu = mGame->MatriculaDe(0);
        if (!eu.empty()) {
            posicao = mPedido.geral
                ? Ranking::PosicaoNoGeral(mFichas, materias.Quantas(), lista.nivel, eu)
                : Ranking::PosicaoDe(mFichas, mPedido.materia, lista.nivel, eu);
        }
    }

    escrever(mSuaPosicaoAtor,
             posicao > static_cast<int>(kQuantasLinhas)
                 ? quemEsta + " em " + std::to_string(posicao) + "o"
                 : "");
}

void TelaDeRanking::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        // Volta para onde veio, e nao sempre para o menu.
        mGame->RequestSceneChange(mPedido.voltarPara);
        return;
    }
    mVoltarAnterior = voltar;

    // O manche para os lados troca de lista, dando a volta; o botao 3 anda para a seguinte.
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

        // As fichas nao sao relidas: a lista muda, a turma nao.
        Redesenhar();
    }
}
