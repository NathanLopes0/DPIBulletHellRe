//
// A tela que o gabinete mostra quando ninguem esta jogando.
//

#include "Atracao.h"
#include "TextoDeCena.h"

#include <SDL_scancode.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Aluno/Exportacao.h"
#include "../Aluno/FichaArquivo.h"
#include "../Motor/Font.h"
#include "../Motor/Game.h"
#include "../Curso/Materias.h"
#include "../Curso/MateriasArquivo.h"
#include "../Aluno/Matricula.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"
#include "../Aluno/Mural.h"
#include "../Aluno/Ranking.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Personagem/ComporPersonagem.h"
#include "../Personagem/PersonagensArquivo.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

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
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

void Atracao::Load() {

    CriarFundoDoCorredor(*this);

    // As fichas sao lidas uma vez, para os dois cartazes que as usam.
    const Materias::Lista& materias = Materias::Carregadas();
    std::vector<Ficha::Dados> dados;
    std::vector<Exportacao::FichaDeAluno> fichas;
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        dados.push_back(FichaArquivo::Carregar(matricula, materias));
        dados.back().matricula = matricula;
        fichas.push_back(Exportacao::FichaDeAluno{matricula, dados.back().progresso});
    }

    mCartazes.push_back(CriarChamada());

    // Os paineis dos melhores (um por lista) e o mural so existem se houver quem mostrar.
    for (const Ranking::Lista& lista : Ranking::kListas) {
        Cartaz melhores = CriarMelhores(fichas, dados, lista);
        if (!melhores.empty()) mCartazes.push_back(std::move(melhores));
    }
    Cartaz mural = CriarMural(fichas, dados);
    if (!mural.empty()) mCartazes.push_back(std::move(mural));

    MostrarSomente(0);
}

Atracao::Cartaz Atracao::CriarChamada() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    Cartaz cartaz;

    // Centrada na tela, e nao mais em cima.
    auto titulo = std::make_unique<Actor>(this);
    titulo->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    cartaz.push_back(titulo->AddComponent<DrawSpriteComponent>(
        Caminhos::Asset("DPIBHTitleMainMenu.png"), 75));
    AddActor(std::move(titulo));

    // A arte do titulo ocupa ate y = 607 de 800, entao as duas linhas vivem na faixa de baixo.
    // \xc3\xa7 e \xc3\xa3 sao o c cedilha e o a til.
    cartaz.push_back(Texto("Uma gradua\xc3\xa7\xc3\xa3o em formato de Arcade",
                           largura / 2.0f, altura * 0.86f, 28, 1000, Color::White));

    cartaz.push_back(Texto(Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Um, "jogar"}},
                                          mGame->JeitoDoRodape()),
                           largura / 2.0f, altura * 0.945f, 32, 700, Color::Gold));

    return cartaz;
}

Atracao::Cartaz Atracao::CriarMelhores(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                       const std::vector<Ficha::Dados>& dados, const Ranking::Lista lista) {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    const Materias::Lista& materias = Materias::Carregadas();

    // A mesma conta do ranking do menu, e pelo mesmo caminho.
    const std::vector<int> contadas = materias.DoGeral();
    const bool dupla = lista.modo == Progresso::Modo::Dupla;
    struct Linha { int posicao; std::string um, dois; float nota; };
    std::vector<Linha> linhas;
    if (dupla) {
        for (const auto& l : Ranking::DuplasGeral(fichas, contadas, lista.nivel, kQuantasLinhas)) {
            linhas.push_back({l.posicao, l.um, l.dois, l.nota});
        }
    } else {
        for (const auto& l : Ranking::Geral(fichas, contadas, lista.nivel, kQuantasLinhas)) {
            linhas.push_back({l.posicao, l.matricula, "", l.nota});
        }
    }
    if (linhas.empty()) return {};

    Cartaz cartaz;

    const bool dificil = lista.nivel == Dificuldade::Nivel::Dificil;
    // \xc3\xad e o i agudo.
    const std::string titulo = std::string(dupla ? "MELHORES DUPLAS" : "MELHORES DO CURSO") +
                               (dificil ? " \xc2\xb7 DIF\xc3\x8d" "CIL" : "");
    cartaz.push_back(Texto(titulo, largura / 2.0f, altura * 0.14f, 44, 1000, dificil ? Color::LightPink : Color::White));
    cartaz.push_back(Texto("media das melhores notas " + std::string(dupla ? "em dupla " : "") + "no " +
                               (dificil ? "Dif\xc3\xad" "cil" : "Normal") + ", sobre as " +
                               std::to_string(contadas.size()) + " materias",
                           largura / 2.0f, altura * 0.21f, 22, 1100, Color::LightBlue));

    constexpr float kPrimeira = 0.32f;
    constexpr float kEspaco = 0.082f;

    // Quem ja se formou aparece de capelo, como no ranking e no mural.
    const Personagens::Catalogo& catalogo = Personagens::Carregado();
    const int formatura = catalogo.formatura.Existe() ? materias.IndiceDe(catalogo.formatura.materia) : -1;

    for (size_t i = 0; i < linhas.size(); ++i) {

        const float y = altura * (kPrimeira + kEspaco * static_cast<float>(i));
        const Linha& l = linhas[i];

        cartaz.push_back(Texto(std::to_string(l.posicao) + ".", largura * (dupla ? 0.25f : 0.31f), y,
                               32, 90, Color::White));
        if (dupla) {
            Avatar(cartaz, dados, l.um, Vector2(largura * 0.30f, y), formatura);
            Avatar(cartaz, dados, l.dois, Vector2(largura * 0.335f, y), formatura);
            cartaz.push_back(Texto(Matricula::ParaExibir(l.um) + " + " + Matricula::ParaExibir(l.dois),
                                   largura * 0.50f, y, 30, 420, Color::White));
        } else {
            Avatar(cartaz, dados, l.um, Vector2(largura * 0.375f, y), formatura);
            cartaz.push_back(Texto(Matricula::ParaExibir(l.um), largura * 0.50f, y, 32, 240, Color::White));
        }
        // Sem dourado por nota cheia aqui, e sem verde de aprovacao.
        cartaz.push_back(Texto(ComDuasCasas(l.nota), largura * (dupla ? 0.69f : 0.64f), y, 32, 200,
                               l.posicao == 1 ? Color::Gold : Color::White));
    }

    return cartaz;
}

void Atracao::Avatar(Cartaz& cartaz, const std::vector<Ficha::Dados>& dados, const std::string& matricula,
                     const Vector2 onde, const int formatura) {
    // A personagem de quem esta ali, parada: numa lista, seis andando distrairiam do que ela diz.
    const Personagens::Catalogo& catalogo = Personagens::Carregado();
    for (const Ficha::Dados& d : dados) {
        if (d.matricula != matricula || d.aparencia.Vazia()) continue;
        const bool formado = formatura >= 0 && d.progresso.Aprovado(formatura);
        const Personagens::Composta c = Personagens::Compor(mGame, catalogo, d.aparencia, formado);
        if (!c.ok) return;
        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(onde);
        ator->SetScale(kEscalaNosMelhores);
        auto* desenho = ator->AddComponent<DrawAnimatedComponent>(c.chaveDaTextura, c.atlas, 100);
        desenho->AddAnimation("Parado", {0});
        desenho->SetAnimation("Parado");
        cartaz.push_back(desenho);
        AddActor(std::move(ator));
        return;
    }
}

Atracao::Cartaz Atracao::CriarMural(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                    const std::vector<Ficha::Dados>& dados) {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    const Personagens::Catalogo& catalogo = Personagens::Carregado();
    if (!catalogo.formatura.Existe()) return {};
    const int formatura = Materias::Carregadas().IndiceDe(catalogo.formatura.materia);
    const std::vector<Mural::Formado> formados = Mural::Recentes(fichas, formatura, kQuantosNoMural);
    if (formados.empty()) return {};

    Cartaz cartaz;
    cartaz.push_back(Texto("MURAL DOS FORMADOS", largura / 2.0f, altura * 0.14f, 44, 900, Color::Gold));
    cartaz.push_back(Texto("quem passou no TCC, dos mais recentes", largura / 2.0f, altura * 0.21f,
                           22, 1100, Color::LightBlue));

    // Uma fileira so fica no meio; duas, uma em cima da outra. Cada fileira se centra.
    const size_t fileiras = (formados.size() + kPorFileira - 1) / kPorFileira;
    for (size_t i = 0; i < formados.size(); ++i) {
        const size_t fileira = i / kPorFileira;
        const size_t naFileira = std::min(kPorFileira, formados.size() - fileira * kPorFileira);
        const float coluna = static_cast<float>(i % kPorFileira) - static_cast<float>(naFileira - 1) / 2.0f;
        const float x = largura / 2.0f + coluna * largura * 0.2f;
        const float y = altura * (fileiras == 1 ? 0.48f : (fileira == 0 ? 0.38f : 0.68f));
        const Mural::Formado& f = formados[i];

        // A personagem de capelo, andando.
        for (const Ficha::Dados& d : dados) {
            if (d.matricula != f.matricula || d.aparencia.Vazia()) continue;
            const Personagens::Composta c = Personagens::Compor(mGame, catalogo, d.aparencia, true);
            if (!c.ok) break;
            auto ator = std::make_unique<Actor>(this);
            ator->SetPosition(Vector2(x, y));
            ator->SetScale(kEscalaNoMural);
            auto* desenho = ator->AddComponent<DrawAnimatedComponent>(c.chaveDaTextura, c.atlas, 100);
            desenho->AddAnimation("Andando", {0, 1, 2, 3});
            desenho->SetAnimFPS(6.0f);
            desenho->SetAnimation("Andando");
            cartaz.push_back(desenho);
            AddActor(std::move(ator));
            break;
        }
        cartaz.push_back(Texto(Matricula::ParaExibir(f.matricula), x, y + 78.0f, 26, 230, Color::White));
        const std::string data = Mural::Data(f.quando);
        if (!data.empty()) cartaz.push_back(Texto(data, x, y + 106.0f, 20, 230, Color::LightBlue));
    }

    return cartaz;
}

DrawComponent* Atracao::Texto(const std::string& conteudo, const float x, const float y,
                              const int tamanho, const int larguraMaxima, const Vector3& cor) {
    auto* dc = CriarTexto(*this, mFonte.get(), conteudo, Vector2(x, y), tamanho, larguraMaxima, tamanho + 8);
    dc->SetColor(cor);
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

    // A condicao e sempre verdadeira: o que a Contagem mede aqui e so a passagem do tempo.
    if (mTrocaDeCartaz.Passou(deltaTime, true)) {
        mCartazAtual = (mCartazAtual + 1) % mCartazes.size();
        MostrarSomente(mCartazAtual);
    }
}

void Atracao::OnProcessInput(const Uint8* keyState) {

    if (!keyState) return;

    // Qualquer tecla.
    bool alguma = false;
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        if (keyState[i]) { alguma = true; break; }
    }

    if (mBordaAlguma.Bateu(alguma)) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
}
