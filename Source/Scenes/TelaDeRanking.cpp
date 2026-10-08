//
// A tela que mostra as maiores notas de cada materia.
//

#include "TelaDeRanking.h"
#include "../Painel.h"

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

    /// As tres colunas, em fracao da largura.
    constexpr float kColunaPosicao   = 0.26f;
    constexpr float kColunaMatricula = 0.38f;
    constexpr float kColunaJogadas   = 0.55f;   // so no geral
    constexpr float kColunaNota      = 0.72f;

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
    Texto(Painel::Rodape({{Painel::Botao::Dois, "voltar"}}),
          largura / 2.0f, altura * 0.93f, 22, 1000);

    // As linhas sao criadas uma vez e reescritas na troca de materia. Criar e
    // destruir ator a cada troca deixaria lixo na cena, que so some no fim dela.
    for (size_t i = 0; i < kQuantasLinhas; ++i) {
        const float y = altura * (kPrimeiraLinha + kEspacoEntreLinhas * static_cast<float>(i));
        mPosicaoAtores.push_back  (Texto(" ", largura * kColunaPosicao,   y, 30, 120, true));
        mMatriculaAtores.push_back(Texto(" ", largura * kColunaMatricula, y, 30, 260, true));
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

    const auto linhas = mPedido.geral
        ? Ranking::Geral(mFichas, materias.Quantas(), kQuantasLinhas)
        : Ranking::DaMateria(mFichas, mPedido.materia, kQuantasLinhas);

    for (size_t i = 0; i < kQuantasLinhas; ++i) {

        const bool temLinha = i < linhas.size();

        escrever(mPosicaoAtores[i],   temLinha ? std::to_string(linhas[i].posicao) : "");
        escrever(mMatriculaAtores[i], temLinha ? linhas[i].matricula : "");
        escrever(mNotaAtores[i],      temLinha ? ComDuasCasas(linhas[i].nota) : "");

        // So no geral: no ranking de uma materia a contagem seria sempre "1 de
        // 11" e nao diria nada.
        escrever(mJogadasAtores[i],
                 (temLinha && mPedido.geral)
                     ? std::to_string(linhas[i].materiasJogadas) + " de "
                       + std::to_string(materias.Quantas())
                     : "");

        // O DOURADO SEGUE A MESMA REGRA DA BATALHA. 99,99 e 100 sao resultados
        // diferentes, e e aqui que essa diferenca finalmente aparece para quem
        // joga - era disso que a curva da nota tratava desde o comeco.
        // O DOURADO SO VALE NO RANKING DE MATERIA. No geral o numero e uma media
        // sobre o curso inteiro, entao 100 exigiria nota cheia em TODAS as
        // materias - pintar por esse criterio seria uma cor que nunca acende.
        const bool cheia = temLinha && !mPedido.geral && Nota::ECheia(linhas[i].nota);
        const Vector3 cor = cheia ? Color::Gold : Color::White;
        pintar(mPosicaoAtores[i], cor);
        pintar(mMatriculaAtores[i], cor);
        pintar(mNotaAtores[i], cor);
        pintar(mJogadasAtores[i], Color::LightBlue);
    }

    escrever(mVazioAtor, linhas.empty()
        ? (mPedido.geral ? "Ninguem jogou ainda." : "Ninguem jogou esta materia ainda.")
        : "");

    // A POSICAO DE QUEM ESTA JOGANDO, quando ela nao cabe no topo. Sem isto o
    // ranking so conversa com quem ja esta bem, que e quem menos precisa.
    std::string rodape;
    const std::string& eu = mGame->MatriculaAtual();
    if (!eu.empty()) {
        const int posicao = mPedido.geral
            ? Ranking::PosicaoNoGeral(mFichas, materias.Quantas(), eu)
            : Ranking::PosicaoDe(mFichas, mPedido.materia, eu);

        if (posicao > static_cast<int>(kQuantasLinhas)) {
            rodape = "Voce esta em " + std::to_string(posicao) + "o";
        }
    }
    escrever(mSuaPosicaoAtor, rodape);
}

void TelaDeRanking::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        // VOLTA PARA ONDE VEIO, e nao sempre para o menu: aberta pela selecao de
        // fases, cair no menu faria o aluno refazer o caminho ate a materia.
        mGame->RequestSceneChange(mPedido.voltarPara);
        return;
    }
    mVoltarAnterior = voltar;
}

void TelaDeRanking::OnUpdate(const float deltaTime) {
}
