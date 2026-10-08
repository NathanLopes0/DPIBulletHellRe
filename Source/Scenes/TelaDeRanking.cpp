//
// A tela que mostra as maiores notas de cada materia.
//

#include "TelaDeRanking.h"

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
    constexpr float kColunaPosicao   = 0.30f;
    constexpr float kColunaMatricula = 0.45f;
    constexpr float kColunaNota      = 0.70f;

    constexpr float kPassoDoManche = 0.22f;

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

    const Materias::Lista& materias = Materias::Carregadas();

    // AS FICHAS SAO LIDAS UMA VEZ. Ver o comentario de classe: reler a cada
    // quadro custaria um passeio pelo disco inteiro por quadro.
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        mFichas.push_back(f);
    }

    mTituloAtor = Texto(" ", largura / 2.0f, altura * 0.13f, 44, 900);
    Texto("MANCHE  trocar de materia          ESC  voltar",
          largura / 2.0f, altura * 0.93f, 22, 1000);

    // As linhas sao criadas uma vez e reescritas na troca de materia. Criar e
    // destruir ator a cada troca deixaria lixo na cena, que so some no fim dela.
    for (size_t i = 0; i < kQuantasLinhas; ++i) {
        const float y = altura * (kPrimeiraLinha + kEspacoEntreLinhas * static_cast<float>(i));
        mPosicaoAtores.push_back  (Texto(" ", largura * kColunaPosicao,   y, 30, 120, true));
        mMatriculaAtores.push_back(Texto(" ", largura * kColunaMatricula, y, 30, 260, true));
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
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void TelaDeRanking::Redesenhar() {

    const Materias::Lista& materias = Materias::Carregadas();
    const Materias::Materia* m = materias.Por(mMateria);

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

    escrever(mTituloAtor, "MAIORES NOTAS  -  " + (m ? m->nome : std::string("?")));

    const auto linhas = Ranking::DaMateria(mFichas, mMateria, kQuantasLinhas);

    for (size_t i = 0; i < kQuantasLinhas; ++i) {

        const bool temLinha = i < linhas.size();

        escrever(mPosicaoAtores[i],   temLinha ? std::to_string(linhas[i].posicao) : "");
        escrever(mMatriculaAtores[i], temLinha ? linhas[i].matricula : "");
        escrever(mNotaAtores[i],      temLinha ? ComDuasCasas(linhas[i].nota) : "");

        // O DOURADO SEGUE A MESMA REGRA DA BATALHA. 99,99 e 100 sao resultados
        // diferentes, e e aqui que essa diferenca finalmente aparece para quem
        // joga - era disso que a curva da nota tratava desde o comeco.
        const Vector3 cor = (temLinha && Nota::ECheia(linhas[i].nota)) ? Color::Gold : Color::White;
        pintar(mPosicaoAtores[i], cor);
        pintar(mMatriculaAtores[i], cor);
        pintar(mNotaAtores[i], cor);
    }

    escrever(mVazioAtor, linhas.empty() ? "Ninguem jogou esta materia ainda." : "");

    // A POSICAO DE QUEM ESTA JOGANDO, quando ela nao cabe no topo. Sem isto o
    // ranking so conversa com quem ja esta bem, que e quem menos precisa.
    std::string rodape;
    const std::string& eu = mGame->MatriculaAtual();
    if (!eu.empty()) {
        if (const int posicao = Ranking::PosicaoDe(mFichas, mMateria, eu);
            posicao > static_cast<int>(kQuantasLinhas)) {
            rodape = "Voce esta em " + std::to_string(posicao) + "o";
        }
    }
    escrever(mSuaPosicaoAtor, rodape);
}

void TelaDeRanking::OnProcessInput(const Uint8* keyState) {

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mVoltarAnterior = voltar;

    if (mPassoTimer < kPassoDoManche) return;

    const int quantas = Materias::Carregadas().Quantas();
    if (quantas <= 0) return;

    const int antes = mMateria;

    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D] ||
        keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S]) {
        mMateria = (mMateria + 1) % quantas;
    }
    else if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A] ||
             keyState[SDL_SCANCODE_UP]   || keyState[SDL_SCANCODE_W]) {
        mMateria = (mMateria - 1 + quantas) % quantas;
    }

    if (mMateria != antes) {
        mPassoTimer = 0.0f;
        Redesenhar();
    }
}

void TelaDeRanking::OnUpdate(const float deltaTime) {
    if (mPassoTimer < kPassoDoManche) mPassoTimer += deltaTime;
}
