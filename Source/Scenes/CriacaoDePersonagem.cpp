//
// A tela onde o aluno monta a personagem dele, camada por camada.
//

#include "CriacaoDePersonagem.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../CaminhosArquivo.h"
#include "../ComporPersonagem.h"
#include "../PersonagensArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// A personagem e desenhada em quadros de 64; sem ampliar, ela sairia do
    /// tamanho que tem em jogo - pequena demais para escolher olhando.
    constexpr float kAmpliacao = 4.0f;

    // A personagem fica a esquerda e a lista a direita: as duas juntas no centro
    // nao caberiam sem encolher uma delas.
    constexpr float kPersonagemX = 0.28f;
    constexpr float kPersonagemY = 0.48f;

    constexpr float kListaX = 0.66f;
    constexpr float kPrimeiraLinha = 0.30f;
    constexpr float kEspacoEntreLinhas = 0.062f;

    constexpr int kTamanhoDaLinha = 26;
    constexpr int kSelecaoLargura = 500;
    constexpr int kSelecaoAltura = 42;

    constexpr SDL_Color kBordaDaSelecao = {235, 235, 235, 255};
    constexpr SDL_Color kFundoDaSelecao = { 34,  38,  48, 170};

    /// O nome de uma peca, ou vazio quando ela nao existe mais.
    std::string NomeDaPeca(const Personagens::Categoria& c, const std::string& id) {
        const Personagens::Peca* p = c.PecaPor(id);
        return p ? p->nome : std::string();
    }

    std::string NomeDaCor(const Personagens::Categoria& c, const std::string& id) {
        const Personagens::Cor* cor = c.CorPor(id);
        return cor ? cor->nome : std::string();
    }
}

CriacaoDePersonagem::CriacaoDePersonagem(Game* game)
    : Scene(game, SceneType::CriacaoDePersonagem),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

Actor* CriacaoDePersonagem::Texto(const std::string& conteudo, const float x, const float y,
                                  const int tamanho, const int larguraMaxima) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));

    auto dc = ator->AddComponent<DrawTextComponent>(conteudo, mFonte.get(),
                                                    larguraMaxima, tamanho + 8, tamanho, 255);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void CriacaoDePersonagem::MontarLinhas() {

    for (const auto& c : Personagens::Carregado().categorias) {

        // Categoria de uma peca so nao rende linha de tipo: uma linha que nao
        // muda nada seria so um lugar a mais para a seta parar.
        if (c.pecas.size() > 1) {
            mLinhas.push_back(Linha{c.id, false, c.nome});
        }
        if (c.cores.size() > 1) {
            // Quando a categoria tem linha de tipo, a de cor vem logo abaixo e
            // se chama so "Cor": repetir a categoria daria "Cor: Calca: Jeans",
            // e escrever "Cor da calca" exigiria concordancia de genero por
            // categoria - algo que o catalogo nao tem como saber.
            const bool temLinhaDeTipo = c.pecas.size() > 1;
            mLinhas.push_back(Linha{c.id, true, temLinhaDeTipo ? "Cor" : c.nome});
        }
    }
}

void CriacaoDePersonagem::Load() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    Texto("SUA PERSONAGEM", largura / 2.0f, altura * 0.09f, 52, 900);

    // Comeca do que o perfil ja tem - vazio para quem esta criando agora, e
    // nesse caso Resolver devolve a aparencia padrao.
    const Personagens::Catalogo& catalogo = Personagens::Carregado();
    mAparencia = catalogo.Resolver(mGame->AparenciaAtual());

    MontarLinhas();

    auto personagem = std::make_unique<Actor>(this);
    personagem->SetPosition(Vector2(largura * kPersonagemX, altura * kPersonagemY));
    personagem->SetScale(kAmpliacao);
    mPersonagemAtor = personagem.get();
    AddActor(std::move(personagem));

    // A moldura entra antes dos rotulos e com ordem menor, para ficar atras.
    auto selecao = std::make_unique<Actor>(this);
    selecao->SetPosition(Vector2(largura * kListaX, altura * kPrimeiraLinha));
    selecao->AddComponent<DrawCaixaComponent>(kSelecaoLargura, kSelecaoAltura,
                                              kBordaDaSelecao, kFundoDaSelecao, 3, 90);
    mSelecaoAtor = selecao.get();
    AddActor(std::move(selecao));

    for (size_t i = 0; i < mLinhas.size(); ++i) {
        const float y = altura * (kPrimeiraLinha + static_cast<float>(i) * kEspacoEntreLinhas);
        mLinhaAtores.push_back(Texto(" ", largura * kListaX, y, kTamanhoDaLinha,
                                     kSelecaoLargura - 30));
    }

    if (mLinhas.empty()) {
        Texto("Nada para escolher: confira Assets/personagens.json.",
              largura / 2.0f, altura * 0.80f, 24, 1000);
    }

    Texto("CIMA e BAIXO  escolhem o que mudar", largura / 2.0f, altura * 0.80f, 22, 700);
    Texto("ESQUERDA e DIREITA  trocam",       largura / 2.0f, altura * 0.86f, 22, 700);
    Texto("ENTER  confirmar     ESC  voltar", largura / 2.0f, altura * 0.92f, 22, 700);

    Recompor();
    Redesenhar();
}

void CriacaoDePersonagem::Recompor() {

    if (!mPersonagemAtor) return;

    const Personagens::Composta composta =
        Personagens::Compor(mGame, Personagens::Carregado(), mAparencia);

    if (!composta.ok) {
        SDL_Log("CRIACAO: nao consegui compor esta aparencia; a anterior continua em tela.");
        return;
    }

    auto dc = mPersonagemAtor->GetComponent<DrawAnimatedComponent>();
    if (dc == nullptr) {
        dc = mPersonagemAtor->AddComponent<DrawAnimatedComponent>(composta.chaveDaTextura,
                                                                   composta.atlas);
        dc->AddAnimation("Andando", {0, 1, 2, 3});
        dc->SetAnimFPS(6.0f);   // mais devagar que em jogo: aqui e para olhar
    }
    else {
        dc->LoadSpriteSheet(composta.chaveDaTextura, composta.atlas);
    }
    dc->SetAnimation("Andando");

    // SO UMA TEXTURA DE PREVIA VIVA POR VEZ. Cada troca compoe uma nova, e sem
    // jogar fora a anterior uma sessao de escolhas deixaria dezenas delas no
    // cache ate o jogo fechar. A ultima sobrevive de proposito: e exatamente a
    // que o Player vai pedir ao entrar na batalha.
    if (!mChaveEmUso.empty() && mChaveEmUso != composta.chaveDaTextura) {
        mGame->EsquecerTextura(mChaveEmUso);
    }
    mChaveEmUso = composta.chaveDaTextura;
}

void CriacaoDePersonagem::Redesenhar() const {

    const Personagens::Catalogo& catalogo = Personagens::Carregado();

    for (size_t i = 0; i < mLinhaAtores.size() && i < mLinhas.size(); ++i) {

        const Linha& linha = mLinhas[i];
        const Personagens::Categoria* c = catalogo.Por(linha.categoria);
        const Personagens::Escolha* e = mAparencia.Por(linha.categoria);
        if (c == nullptr || e == nullptr) continue;

        const std::string valor = linha.ehCor ? NomeDaCor(*c, e->cor)
                                              : NomeDaPeca(*c, e->peca);

        if (auto dc = mLinhaAtores[i]->GetComponent<DrawTextComponent>()) {
            dc->SetText(linha.rotulo + ":  " + (valor.empty() ? "?" : valor));
        }
    }

    if (mSelecaoAtor && !mLinhas.empty()) {
        const auto largura = static_cast<float>(mGame->GetWindowWidth());
        const auto altura  = static_cast<float>(mGame->GetWindowHeight());
        mSelecaoAtor->SetPosition(Vector2(
            largura * kListaX,
            altura * (kPrimeiraLinha + static_cast<float>(mLinhaEmFoco) * kEspacoEntreLinhas)));
    }
}

void CriacaoDePersonagem::Trocar(const int passo) {

    if (mLinhas.empty()) return;

    const Linha& linha = mLinhas[static_cast<size_t>(mLinhaEmFoco)];
    const Personagens::Categoria* c = Personagens::Carregado().Por(linha.categoria);
    if (c == nullptr) return;

    Personagens::Escolha e = *mAparencia.Por(linha.categoria);

    // Anda na lista de ids da categoria, dando a volta nas pontas. Somar o
    // tamanho antes do resto: em C++ o resto de um negativo e negativo.
    if (linha.ehCor) {
        const int n = static_cast<int>(c->cores.size());
        int atual = 0;
        for (int i = 0; i < n; ++i) if (c->cores[static_cast<size_t>(i)].id == e.cor) atual = i;
        e.cor = c->cores[static_cast<size_t>((atual + passo + n) % n)].id;
    }
    else {
        const int n = static_cast<int>(c->pecas.size());
        int atual = 0;
        for (int i = 0; i < n; ++i) if (c->pecas[static_cast<size_t>(i)].id == e.peca) atual = i;
        e.peca = c->pecas[static_cast<size_t>((atual + passo + n) % n)].id;
    }

    mAparencia.Definir(linha.categoria, e);
    Recompor();
    Redesenhar();
}

void CriacaoDePersonagem::OnProcessInput(const Uint8* keyState) {

    const int quantas = static_cast<int>(mLinhas.size());

    const bool cima     = keyState[SDL_SCANCODE_UP]    || keyState[SDL_SCANCODE_W];
    const bool baixo    = keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S];
    const bool esquerda = keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A];
    const bool direita  = keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D];

    if (quantas > 0) {
        if (cima && !mCimaAnterior) {
            mLinhaEmFoco = (mLinhaEmFoco - 1 + quantas) % quantas;
            Redesenhar();
        }
        if (baixo && !mBaixoAnterior) {
            mLinhaEmFoco = (mLinhaEmFoco + 1) % quantas;
            Redesenhar();
        }
        if (esquerda && !mEsquerdaAnterior) Trocar(-1);
        if (direita && !mDireitaAnterior) Trocar(+1);
    }
    mCimaAnterior = cima;
    mBaixoAnterior = baixo;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;

    const bool confirmar = keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;
        // Grava na hora: quem acabou de montar a personagem espera encontra-la
        // ao carregar o perfil, mesmo fechando o jogo agora.
        mGame->DefinirAparencia(mAparencia);
        mGame->RequestSceneChange(SceneType::StageSelect);
        return;
    }
    mConfirmarAnterior = confirmar;

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        // Volta para a matricula. Nada foi gravado ainda, entao a matricula
        // digitada continua livre para ser usada de novo.
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }
    mVoltarAnterior = voltar;
}

void CriacaoDePersonagem::OnUpdate(const float deltaTime) {

}
