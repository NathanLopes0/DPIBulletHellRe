//
// A tela que o gabinete mostra quando ninguem esta jogando.
//

#include "Atracao.h"

#include <SDL_scancode.h>
#include <iomanip>
#include <sstream>

#include "../CaminhosArquivo.h"
#include "../Exportacao.h"
#include "../FichaArquivo.h"
#include "../Font.h"
#include "../Game.h"
#include "../Materias.h"
#include "../MateriasArquivo.h"
#include "../Matricula.h"
#include "../Painel.h"
#include "../Ranking.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    constexpr int kOrdemDoFundo = 40;
    constexpr int kOrdemDoVeu = 45;

    /// O mesmo tratamento da selecao de fases: a foto do corredor bem escura,
    /// mais um veu por cima, para o texto branco nao cair em cima de um tijolo
    /// claro e sumir.
    constexpr SDL_Color kSemBorda = {0, 0, 0, 0};
    constexpr SDL_Color kVeu = {6, 8, 18, 205};

    std::string ComDuasCasas(const float nota) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(2) << nota;
        return s.str();
    }
}

Atracao::Atracao(Game* game)
    : Scene(game, SceneType::Atracao)
    , mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void Atracao::Load() {

    CriarFundo();

    mCartazes.push_back(CriarChamada());

    // O PAINEL DOS MELHORES SO EXISTE SE HOUVER QUEM MOSTRAR. Numa maquina
    // recem-instalada nao ha ficha nenhuma, e uma tabela vazia alternando com a
    // chamada pareceria defeito.
    Cartaz melhores = CriarMelhores();
    if (!melhores.empty()) mCartazes.push_back(std::move(melhores));

    MostrarSomente(0);
}

void Atracao::CriarFundo() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    auto fundo = std::make_unique<Actor>(this);
    fundo->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    const auto foto = fundo->AddComponent<DrawSpriteComponent>(
        Caminhos::Asset("MainMenuBackground.png"), kOrdemDoFundo);
    foto->SetColor(66, 70, 92);
    AddActor(std::move(fundo));

    auto veu = std::make_unique<Actor>(this);
    veu->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    veu->AddComponent<DrawCaixaComponent>(
        static_cast<int>(largura), static_cast<int>(altura),
        kSemBorda, kVeu, 0, kOrdemDoVeu);
    AddActor(std::move(veu));
}

Atracao::Cartaz Atracao::CriarChamada() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    Cartaz cartaz;

    // A MESMA ARTE DE TITULO DO MENU, e nao uma segunda: se fossem duas, a
    // atracao continuaria anunciando o titulo antigo no dia em que ele mudasse.
    // CENTRADA NA TELA, e nao mais em cima: a arte tem a altura da tela inteira,
    // e subir o centro cortava o alto das letras do DPI.
    auto titulo = std::make_unique<Actor>(this);
    titulo->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    cartaz.push_back(titulo->AddComponent<DrawSpriteComponent>(
        Caminhos::Asset("DPIBHTitleMainMenu.png"), 75));
    AddActor(std::move(titulo));

    // A arte do titulo ocupa ate y = 607 de 800, entao as duas linhas vivem na
    // faixa de baixo - o mesmo raciocinio das opcoes do menu.
    cartaz.push_back(Texto("Voce e o aluno. Os professores sao os chefes.",
                           largura / 2.0f, altura * 0.86f, 28, 1000, Color::White));

    cartaz.push_back(Texto(Painel::Rodape({{Painel::Botao::Um, "jogar"}},
                                          mGame->JeitoDoRodape()),
                           largura / 2.0f, altura * 0.945f, 32, 700, Color::Gold));

    return cartaz;
}

Atracao::Cartaz Atracao::CriarMelhores() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    const Materias::Lista& materias = Materias::Carregadas();

    std::vector<Exportacao::FichaDeAluno> fichas;
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        fichas.push_back(f);
    }

    // A MESMA CONTA DO RANKING DO MENU, e pelo mesmo caminho: soma das melhores
    // notas dividida pelas materias DO CURSO. Repetir a conta aqui faria a
    // atracao e o ranking discordarem no dia em que uma das duas mudasse.
    const std::vector<Ranking::Linha> linhas =
        Ranking::Geral(fichas, materias.Quantas(), kQuantasLinhas);

    if (linhas.empty()) return {};

    Cartaz cartaz;

    cartaz.push_back(Texto("MELHORES DO CURSO", largura / 2.0f, altura * 0.14f,
                           44, 900, Color::White));
    cartaz.push_back(Texto("media das melhores notas, sobre as "
                           + std::to_string(materias.Quantas()) + " materias",
                           largura / 2.0f, altura * 0.21f, 22, 1100, Color::LightBlue));

    constexpr float kPrimeira = 0.32f;
    constexpr float kEspaco = 0.082f;

    for (size_t i = 0; i < linhas.size(); ++i) {

        const float y = altura * (kPrimeira + kEspaco * static_cast<float>(i));
        const Ranking::Linha& l = linhas[i];

        cartaz.push_back(Texto(std::to_string(l.posicao) + ".", largura * 0.34f, y,
                               32, 90, Color::White));
        cartaz.push_back(Texto(Matricula::ParaExibir(l.matricula), largura * 0.47f, y,
                               32, 240, Color::White));
        // SEM DOURADO POR NOTA CHEIA aqui, e sem verde de aprovacao: este numero
        // NAO e uma nota, e a media de quem jogou 2 das 11 materias e baixa por
        // definicao. O dourado marca a PRIMEIRA POSICAO, que e o que uma lista
        // de melhores quer dizer.
        cartaz.push_back(Texto(ComDuasCasas(l.nota), largura * 0.64f, y, 32, 200,
                               l.posicao == 1 ? Color::Gold : Color::White));
    }

    return cartaz;
}

DrawComponent* Atracao::Texto(const std::string& conteudo, const float x, const float y,
                              const int tamanho, const int larguraMaxima, const Vector3& cor) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));

    const auto dc = ator->AddComponent<DrawTextComponent>(
        conteudo, mFonte.get(), larguraMaxima, tamanho + 8, tamanho, 255);

    // A QUEBRA ACOMPANHA A LARGURA DECLARADA: o padrao do componente e 500 px,
    // e qualquer caixa mais larga que isso quebraria no meio sem pedir.
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);
    dc->SetColor(cor);

    AddActor(std::move(ator));
    return dc;
}

void Atracao::MostrarSomente(const size_t qual) const {

    for (size_t p = 0; p < mCartazes.size(); ++p) {
        for (DrawComponent* desenho : mCartazes[p]) {
            if (desenho) desenho->SetIsVisible(p == qual);
        }
    }
}

void Atracao::OnUpdate(const float deltaTime) {

    if (mCartazes.size() <= 1) return;

    // A condicao e sempre verdadeira: o que a Contagem mede aqui e so a passagem
    // do tempo. E a mesma classe das duas esperas do gabinete.
    if (mTrocaDeCartaz.Passou(deltaTime, true)) {
        mCartazAtual = (mCartazAtual + 1) % mCartazes.size();
        MostrarSomente(mCartazAtual);
    }
}

void Atracao::OnProcessInput(const Uint8* keyState) {

    if (!keyState) return;

    // QUALQUER TECLA. Nao ha escolha nesta tela, entao nao ha botao certo - quem
    // encostou no painel quer jogar, e mandar procurar a tecla certa seria o
    // contrario do que uma tela de atracao faz.
    bool alguma = false;
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        if (keyState[i]) { alguma = true; break; }
    }

    if (alguma && !mAlgumaAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mAlgumaAnterior = alguma;
}
