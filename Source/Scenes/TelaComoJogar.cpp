//
// A tela "Como Jogar", do menu principal.
//

#include "TelaComoJogar.h"
#include "TextoDeCena.h"

#include <cctype>

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Entrada/Painel.h"
#include "../Interface/ManualArquivo.h"
#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Actors/Actor.h"
#include "../Aluno/Mencoes.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    /// A placa atras do texto: a foto do corredor e clara e cheia de detalhe, e texto branco solto
    /// sobre ela some (FundoDeCena.h).
    constexpr int kLarguraDaPlaca = 1000;
    constexpr int kAlturaDaPlaca = 560;

    /// Em fracao da altura da tela, como as outras telas de menu.
    constexpr float kTopoDaPlaca = 0.155f;
    constexpr float kRodape = 0.93f;

    /// A partir do topo da placa, em pixels.
    constexpr float kTituloNaPlaca = 52.0f;
    constexpr float kPrimeiraLinha = 128.0f;
    constexpr float kEntreLinhas = 44.0f;
    constexpr float kIndicadorNaPlaca = 520.0f;
    constexpr float kSetas = 90.0f;   ///< do meio do indicador ao meio de cada seta

    /// Larga, para nenhuma linha quebrar: a que nao cabe encolhe a letra
    /// (SetAjustarAoTexto) em vez de virar duas.
    constexpr unsigned kSemQuebra = 4000;

    /// A grade das mencoes, a partir do topo da placa: a primeira linha, o passo, o selo em 3x
    /// (39 x 30) e o texto depois dele.
    constexpr float kPrimeiraMencao = 122.0f;
    constexpr float kEntreMencoes = 54.0f;
    constexpr float kEscalaDoSelo = 3.0f;
    constexpr float kTextoDepoisDoSelo = 34.0f;
    constexpr float kColuna = 470.0f;
    constexpr int kLetraDoNome = 21;
    constexpr int kLetraDoComo = 16;
    const Vector3 kCorDoComo(0.78f, 0.82f, 0.90f);

    const SDL_Color kMolduraDaPlaca{120, 140, 180, 230};
    const SDL_Color kFundoDaPlaca{12, 13, 17, 215};
}

TelaComoJogar::TelaComoJogar(Game* game)
    : Scene(game, SceneType::ComoJogar),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

DrawTextComponent* TelaComoJogar::Texto(const std::string& conteudo, const float x, const float y,
                                        const int tamanho, const int larguraMaxima) {
    auto* dc = CriarTexto(*this, mFonte.get(), conteudo, Vector2(x, y), tamanho, larguraMaxima, tamanho + 8);
    // Sem quebra: a linha que nao cabe encolhe, em vez de virar duas.
    dc->SetLarguraDeQuebra(kSemQuebra);
    return dc;
}

DrawTextComponent* TelaComoJogar::TextoAEsquerda(const std::string& conteudo, const float x, const float y,
                                                 const int tamanho, const int largura) {
    auto* dc = CriarTexto(*this, mFonte.get(), conteudo, Vector2(x + static_cast<float>(largura) / 2.0f, y),
                          tamanho, largura, tamanho + 8);
    dc->SetLarguraDeQuebra(kSemQuebra);
    dc->SetAEsquerda(true);
    return dc;
}

void TelaComoJogar::CriarAsMencoes(const float topo, const float meio) {
    // As notas e os modos (os sete primeiros) a esquerda; os desafios a direita, com um titulo.
    const float esquerda = meio - static_cast<float>(kLarguraDaPlaca) / 2.0f + 30.0f;
    const float direita = meio + 20.0f;
    auto* desafios = TextoAEsquerda("Os desafios: passe jogando de outro jeito", direita, topo + kPrimeiraMencao,
                                    kLetraDoNome, static_cast<int>(kColuna));
    desafios->SetColor(Color::Gold);
    mTextosDasMencoes.push_back(desafios);

    int naEsquerda = 0, naDireita = 0;
    for (int i = 0; i < Mencoes::kQuantas; ++i) {
        const Mencoes::Mencao m = Mencoes::kTodas[static_cast<size_t>(i)];
        const bool desafio = m >= Mencoes::kDesafios.front();
        const float x = desafio ? direita : esquerda;
        const int linha = desafio ? 1 + naDireita++ : naEsquerda++;
        const float y = topo + kPrimeiraMencao + kEntreMencoes * static_cast<float>(linha);

        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(x + 20.0f, y));
        auto* selo = ator->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/DPIBHSelos.png"),
                                                               Caminhos::Asset("Icons/DPIBHSelos.json"), 200);
        selo->AddAnimation("aceso", {2 * i});   // o aceso da mencao i (Ferramentas/gerar_interface.py)
        selo->SetAnimation("aceso");
        selo->SetEscalaPropria(kEscalaDoSelo);
        mSelos.push_back(selo);
        AddActor(std::move(ator));

        const Mencoes::Cor c = Mencoes::CorDa(m);
        std::string nome = Mencoes::Nome(m);
        nome[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(nome[0])));
        const float xTexto = x + 20.0f + kTextoDepoisDoSelo;
        const int largura = static_cast<int>(kColuna - 20.0f - kTextoDepoisDoSelo);
        auto* dcNome = TextoAEsquerda(nome, xTexto, y - 11.0f, kLetraDoNome, largura);
        dcNome->SetColor(Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f));
        auto* dcComo = TextoAEsquerda(Mencoes::ComoGanhar(m), xTexto, y + 12.0f, kLetraDoComo, largura);
        dcComo->SetColor(kCorDoComo);
        mTextosDasMencoes.push_back(dcNome);
        mTextosDasMencoes.push_back(dcComo);
    }

    // No pe da coluna dos desafios, onde sobra lugar: onde ver as que ja tem.
    auto* nota = TextoAEsquerda("Cada mat\xc3\xa9ria tem as suas: o boletim mostra quais voc\xc3\xaa j\xc3\xa1 tem.",
                                direita, topo + kPrimeiraMencao + kEntreMencoes * 6.0f, kLetraDoComo,
                                static_cast<int>(kColuna));
    nota->SetColor(kCorDoComo);
    mTextosDasMencoes.push_back(nota);
}

void TelaComoJogar::MostrarAsMencoes(const bool mostrar) {
    for (DrawAnimatedComponent* s : mSelos) s->SetIsVisible(mostrar);
    for (DrawTextComponent* t : mTextosDasMencoes) t->SetIsVisible(mostrar);
}

void TelaComoJogar::Load() {

    CriarFundoDoCorredor(*this);

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const float meio = largura / 2.0f;
    const float topo = altura * kTopoDaPlaca;

    Texto("COMO JOGAR", meio, topo / 2.0f, 52, 760);

    auto placa = std::make_unique<Actor>(this);
    placa->SetPosition(Vector2(meio, topo + kAlturaDaPlaca / 2.0f));
    placa->AddComponent<DrawCaixaComponent>(kLarguraDaPlaca, kAlturaDaPlaca, kMolduraDaPlaca, kFundoDaPlaca, 3, 100);
    AddActor(std::move(placa));

    mTitulo = Texto("", meio, topo + kTituloNaPlaca, 40, kLarguraDaPlaca - 60);
    mTitulo->SetColor(Color::Gold);
    for (int i = 0; i < Manual::kLinhasNoMaximo; ++i) {
        mLinhas.push_back(Texto("", meio, topo + kPrimeiraLinha + kEntreLinhas * static_cast<float>(i),
                                26, kLarguraDaPlaca - 60));
    }

    const float yIndicador = topo + kIndicadorNaPlaca;
    mIndicador = Texto("", meio, yIndicador, 24, 200);
    mSetaAnterior = Texto("<", meio - kSetas, yIndicador, 24, 40);
    mSetaProxima = Texto(">", meio + kSetas, yIndicador, 24, 40);

    mRodape = Texto("", meio, altura * kRodape, 24, 1000);

    CriarAsMencoes(topo, meio);

    Mostrar();
}

void TelaComoJogar::Mostrar() {
    const auto& paginas = Manual::Carregado().paginas;
    const int quantas = static_cast<int>(paginas.size());
    const bool arcade = mGame->NoGabinete();

    std::vector<std::string> linhas;
    bool mencoes = false;
    if (quantas == 0) {
        // Sem o arquivo, a tela diz o que houve em vez de ficar em branco.
        mTitulo->SetText("Manual indispon\xc3\xadvel");
        linhas = {"O arquivo Assets/como-jogar.json n\xc3\xa3o foi encontrado."};
        mIndicador->SetIsVisible(false);
    } else {
        const Manual::Pagina& p = paginas[static_cast<size_t>(mPagina)];
        mTitulo->SetText(p.titulo);
        mencoes = p.mencoes;
        if (!mencoes) linhas = Manual::Linhas(p, arcade);
        mIndicador->SetText(Manual::Indicador(mPagina, quantas));
        mIndicador->SetIsVisible(true);
    }
    for (size_t i = 0; i < mLinhas.size(); ++i) {
        const bool usada = i < linhas.size();
        mLinhas[i]->SetIsVisible(usada);
        if (usada) mLinhas[i]->SetText(linhas[i].empty() ? " " : linhas[i]);
    }

    MostrarAsMencoes(mencoes);

    const bool temAnterior = mPagina > 0;
    const bool temProxima = mPagina + 1 < quantas;
    mSetaAnterior->SetIsVisible(temAnterior);
    mSetaProxima->SetIsVisible(temProxima);

    // \xc3\xa1 e \xc3\xb3 sao o a e o o agudos: o MSVC nao le acento em literal.
    mRodape->SetText(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "p\xc3\xa1ginas"),
                                     {Painel::Jogador::Um, Painel::Botao::Um,
                                      temProxima ? "pr\xc3\xb3xima" : "terminar"},
                                     {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                                    mGame->JeitoDoRodape()));
}

void TelaComoJogar::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }

    // Qualquer um dos dois vira a pagina: a dupla le junta.
    bool esquerda = false, direita = false, avancar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        esquerda = esquerda || Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda);
        direita = direita || Painel::Apertado(keyState, quem, Painel::Direcao::Direita);
        avancar = avancar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    avancar = avancar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    const int quantas = static_cast<int>(Manual::Carregado().paginas.size());
    int passo = 0;
    if (mBordaEsquerda.Bateu(esquerda)) passo -= 1;
    if (mBordaDireita.Bateu(direita)) passo += 1;
    const bool avancou = mBordaAvancar.Bateu(avancar);

    if (avancou) {
        // Na ultima (ou sem manual), o botao 1 termina: o leitor chegou ao fim.
        if (mPagina + 1 >= quantas) {
            mGame->RequestSceneChange(SceneType::MainMenu);
            return;
        }
        passo += 1;
    }

    const int nova = Manual::Virar(mPagina, passo, quantas);
    if (nova != mPagina) {
        mPagina = nova;
        Mostrar();
    }
}
