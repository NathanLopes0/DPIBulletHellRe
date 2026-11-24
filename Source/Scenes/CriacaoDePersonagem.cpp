//
// A tela onde o aluno monta a personagem dele, camada por camada.
//

#include "CriacaoDePersonagem.h"
#include "TextoDeCena.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>

#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Personagem/ComporPersonagem.h"
#include "../Personagem/PersonagensArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// A personagem e desenhada em quadros de 64; sem ampliar, ela sairia do
    /// tamanho que tem em jogo - pequena demais para escolher olhando.
    constexpr float kAmpliacao = 4.0f;

    // A personagem fica a esquerda e a tabela a direita: as duas juntas no centro
    // nao caberiam sem encolher uma delas.
    constexpr float kPersonagemX = 0.24f;
    constexpr float kPersonagemY = 0.48f;

    // A tabela: uma linha por categoria (tom de pele, cabelo, camisa, calca, sapato, mochila), o
    // objeto numa coluna e a cor na do lado.
    constexpr float kObjetoX = 0.555f;
    constexpr float kCorX = 0.82f;
    constexpr float kCabecalho = 0.20f;
    constexpr float kPrimeiraLinha = 0.28f;
    constexpr float kEspacoEntreLinhas = 0.088f;

    constexpr int kTamanhoDaLinha = 24;
    constexpr int kSelecaoLargura = 304;
    constexpr int kSelecaoAltura = 50;
    constexpr int kLadoDaAmostra = 28;
    /// Do meio da celula da cor ao meio da amostra, a esquerda do nome.
    constexpr float kAmostraX = -120.0f;

    constexpr SDL_Color kBordaDaSelecao = {235, 235, 235, 255};
    constexpr SDL_Color kFundoDaSelecao = { 34,  38,  48, 170};

    /// O botao CONFIRMAR: no meio das duas colunas, uma linha depois da ultima. Dourado quando o foco
    /// esta nele.
    constexpr float kBotaoX = (kObjetoX + kCorX) / 2.0f;
    constexpr int kBotaoLargura = 304;
    constexpr int kBotaoAltura = 50;
    constexpr SDL_Color kBordaDoBotao = {150, 150, 160, 255};
    constexpr SDL_Color kFundoDoBotao = { 34,  38,  48, 200};
    constexpr SDL_Color kBordaDoBotaoEmFoco = {255, 214, 72, 255};
    constexpr SDL_Color kFundoDoBotaoEmFoco = { 70,  58,  20, 230};

    std::string NomeDaPeca(const Personagens::Categoria& c, const std::string& id) {
        const Personagens::Peca* p = c.PecaPor(id);
        return p ? p->nome : std::string();
    }

    std::string NomeDaCor(const Personagens::Categoria& c, const std::string& id) {
        const Personagens::Cor* cor = c.CorPor(id);
        return cor ? cor->nome : std::string();
    }

    /// "(3 de 15)": onde a escolha esta na lista, que da a volta - com quinze cabelos, sem isto nao
    /// se sabe se ja passou por todos.
    template <typename T>
    std::string Posicao(const std::vector<T>& lista, const std::string& id) {
        for (size_t i = 0; i < lista.size(); ++i) {
            if (lista[i].id == id) return "(" + std::to_string(i + 1) + " de " + std::to_string(lista.size()) + ")";
        }
        return "";
    }
}

CriacaoDePersonagem::CriacaoDePersonagem(Game* game)
    : Scene(game, SceneType::CriacaoDePersonagem),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

Actor* CriacaoDePersonagem::Texto(const std::string& conteudo, const float x, const float y,
                                  const int tamanho, const int larguraMaxima) {
    return CriarTexto(*this, mFonte.get(), conteudo, Vector2(x, y), tamanho, larguraMaxima, tamanho + 8)->GetOwner();
}

void CriacaoDePersonagem::MontarLinhas() {

    for (const auto& c : Personagens::Carregado().categorias) {
        // Categoria sem nada para trocar nao rende linha.
        if (c.pecas.size() <= 1 && c.cores.size() <= 1) continue;
        mLinhas.push_back(Linha{c.id, c.nome, c.pecas.size() > 1, c.cores.size() > 1});
    }
}

bool CriacaoDePersonagem::Editavel(const int linha, const int coluna) const {
    if (linha < 0 || linha >= static_cast<int>(mLinhas.size())) return false;
    const Linha& l = mLinhas[static_cast<size_t>(linha)];
    return coluna == kObjeto ? l.temPeca : l.temCor;
}

void CriacaoDePersonagem::AjustarColuna() {
    mColunaEmFoco = mColunaPreferida;
    if (!Editavel(mLinhaEmFoco, mColunaEmFoco)) mColunaEmFoco = mColunaEmFoco == kObjeto ? kCor : kObjeto;
}

void CriacaoDePersonagem::Load() {

    CriarFundoDoCorredor(*this);

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    // De quem e a personagem.
    mQuem = mGame->PosicaoEntrando();
    const bool emDupla = mGame->QuantosJogadores() > 1;

    Texto(emDupla ? ("PERSONAGEM DO JOGADOR " + std::to_string(mQuem + 1))
                  : std::string("SUA PERSONAGEM"),
          largura / 2.0f, altura * 0.09f, 52, 900);

    const Personagens::Catalogo& catalogo = Personagens::Carregado();
    mAparencia = catalogo.Resolver(mGame->AparenciaDe(mQuem));

    MontarLinhas();

    auto personagem = std::make_unique<Actor>(this);
    personagem->SetPosition(Vector2(largura * kPersonagemX, altura * kPersonagemY));
    personagem->SetScale(kAmpliacao);
    mPersonagemAtor = personagem.get();
    AddActor(std::move(personagem));

    // A moldura entra antes dos rotulos e com ordem menor, para ficar atras.
    auto selecao = std::make_unique<Actor>(this);
    selecao->SetPosition(Vector2(largura * kObjetoX, altura * kPrimeiraLinha));
    selecao->AddComponent<DrawCaixaComponent>(kSelecaoLargura, kSelecaoAltura,
                                              kBordaDaSelecao, kFundoDaSelecao, 3, 90);
    mSelecaoAtor = selecao.get();
    AddActor(std::move(selecao));

    Texto("OBJETO", largura * kObjetoX, altura * kCabecalho, 24, kSelecaoLargura);
    Texto("COR", largura * kCorX, altura * kCabecalho, 24, kSelecaoLargura);

    for (size_t i = 0; i < mLinhas.size(); ++i) {
        const float y = altura * (kPrimeiraLinha + static_cast<float>(i) * kEspacoEntreLinhas);
        mObjetoAtores.push_back(Texto(" ", largura * kObjetoX, y, kTamanhoDaLinha, kSelecaoLargura - 24));
        // O nome da cor fica a direita da amostra.
        mCorAtores.push_back(Texto(" ", largura * kCorX + 18.0f, y, kTamanhoDaLinha, kSelecaoLargura - 80));
        auto amostra = std::make_unique<Actor>(this);
        amostra->SetPosition(Vector2(largura * kCorX + kAmostraX, y));
        amostra->AddComponent<DrawCaixaComponent>(kLadoDaAmostra, kLadoDaAmostra, SDL_Color{235, 235, 235, 255},
                                                  SDL_Color{0, 0, 0, 0}, 2, 95);
        mAmostras.push_back(amostra.get());
        AddActor(std::move(amostra));
    }
    AjustarColuna();

    // O CONFIRMAR, uma linha depois da ultima: a personagem so e gravada nele.
    {
        const float y = altura * (kPrimeiraLinha + static_cast<float>(mLinhas.size()) * kEspacoEntreLinhas);
        auto botao = std::make_unique<Actor>(this);
        botao->SetPosition(Vector2(largura * kBotaoX, y));
        botao->AddComponent<DrawCaixaComponent>(kBotaoLargura, kBotaoAltura, kBordaDoBotao, kFundoDoBotao, 3, 92);
        mBotaoAtor = botao.get();
        AddActor(std::move(botao));
        Texto("CONFIRMAR", largura * kBotaoX, y, 26, kBotaoLargura - 24);
    }

    if (mLinhas.empty()) {
        Texto("Nada para escolher: confira Assets/personagens.json.",
              largura / 2.0f, altura * 0.80f, 24, 1000);
    }

    // Uma linha so para o manche, e o rodape saindo dos botoes.
    Texto("MANCHE  cima e baixo escolhem a linha - os lados trocam",
          largura / 2.0f, altura * 0.88f, 22, 1100);
    mRodapeAtor = Texto(Painel::Rodape({{PainelDaVez(), Painel::Botao::Tres, "Alternar objeto / cor"},
                          {PainelDaVez(), Painel::Botao::Um,   "confirmar"},
                          {PainelDaVez(), Painel::Botao::Dois, "voltar"}},
                         mGame->JeitoDoRodape()),
          largura / 2.0f, altura * 0.92f, 22, 1000);

    Recompor();
    Redesenhar();
}

void CriacaoDePersonagem::Recompor() {

    if (!mPersonagemAtor) return;

    const Personagens::Composta composta =
        Personagens::Compor(mGame, Personagens::Carregado(), mAparencia, mGame->Formado(mQuem));

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

    // So uma textura de previa viva por vez.
    if (!mChaveEmUso.empty() && mChaveEmUso != composta.chaveDaTextura) {
        mGame->EsquecerTextura(mChaveEmUso);
    }
    mChaveEmUso = composta.chaveDaTextura;
}

void CriacaoDePersonagem::Redesenhar() const {

    const Personagens::Catalogo& catalogo = Personagens::Carregado();

    for (size_t i = 0; i < mObjetoAtores.size() && i < mLinhas.size(); ++i) {

        const Linha& linha = mLinhas[i];
        const Personagens::Categoria* c = catalogo.Por(linha.categoria);
        const Personagens::Escolha* e = mAparencia.Por(linha.categoria);
        if (c == nullptr || e == nullptr) continue;

        // O objeto: "Cabelo: Moicano (3 de 15)"; a categoria de uma peca so (o tom de pele) e so o
        // nome dela.
        const std::string peca = NomeDaPeca(*c, e->peca);
        const std::string objeto = linha.temPeca
            ? linha.rotulo + ": " + (peca.empty() ? "?" : peca) + "  " + Posicao(c->pecas, e->peca)
            : linha.rotulo;
        if (auto dc = mObjetoAtores[i]->GetComponent<DrawTextComponent>()) dc->SetText(objeto);

        // A cor: a amostra e o nome, "Castanho (2 de 16)".
        const Personagens::Cor* cor = c->CorPor(e->cor);
        const std::string nome = NomeDaCor(*c, e->cor);
        if (auto dc = mCorAtores[i]->GetComponent<DrawTextComponent>()) {
            dc->SetText(linha.temCor ? (nome.empty() ? "?" : nome) + "  " + Posicao(c->cores, e->cor) : std::string("-"));
        }
        if (auto* caixa = mAmostras[i]->GetComponent<DrawCaixaComponent>()) {
            caixa->SetIsVisible(linha.temCor && cor != nullptr);
            if (cor) caixa->SetFundo(SDL_Color{cor->r, cor->g, cor->b, 255});
        }
    }

    // No botao, a moldura da tabela some e o botao acende.
    if (mBotaoAtor) {
        if (auto* caixa = mBotaoAtor->GetComponent<DrawCaixaComponent>()) {
            caixa->SetMoldura(NoBotao() ? kBordaDoBotaoEmFoco : kBordaDoBotao);
            caixa->SetFundo(NoBotao() ? kFundoDoBotaoEmFoco : kFundoDoBotao);
        }
    }
    if (mSelecaoAtor) {
        if (auto* caixa = mSelecaoAtor->GetComponent<DrawCaixaComponent>()) caixa->SetIsVisible(!NoBotao());
    }

    if (mSelecaoAtor && !mLinhas.empty() && !NoBotao()) {
        const auto largura = static_cast<float>(mGame->GetWindowWidth());
        const auto altura  = static_cast<float>(mGame->GetWindowHeight());
        mSelecaoAtor->SetPosition(Vector2(
            largura * (mColunaEmFoco == kCor ? kCorX : kObjetoX),
            altura * (kPrimeiraLinha + static_cast<float>(mLinhaEmFoco) * kEspacoEntreLinhas)));
    }

    AtualizarRodape();
}

void CriacaoDePersonagem::AtualizarRodape() const {

    if (!mRodapeAtor) return;

    std::string dicaBotaoTres;

    // Quando o foco esta no CONFIRMAR, nao existe outra coluna.
    if (NoBotao()) {
        dicaBotaoTres = "sem alternancia aqui";
    }
    else {
        // Descobre qual seria a outra coluna.
        const int outraColuna =
            mColunaEmFoco == kObjeto ? kCor : kObjeto;

        // Algumas categorias possuem apenas uma opcao editavel.
        // Nesse caso, o Botao 3 nao produz nenhuma mudanca.
        if (!Editavel(mLinhaEmFoco, outraColuna)) {
            dicaBotaoTres = "sem alternancia nesta linha";
        }
        else if (mColunaEmFoco == kObjeto) {
            dicaBotaoTres = "Objeto ativo; alternar para COR";
        }
        else {
            dicaBotaoTres = "Cor ativa; alternar para OBJETO";
        }
    }

    // Reconstroi o texto utilizando o sistema de rodape do projeto.
    const std::string texto = Painel::Rodape({
        {PainelDaVez(), Painel::Botao::Tres, dicaBotaoTres},
        {PainelDaVez(), Painel::Botao::Um, "confirmar"},
        {PainelDaVez(), Painel::Botao::Dois, "voltar"}
    }, mGame->JeitoDoRodape());

    // Atualiza a textura do texto existente.
    if (auto* dc = mRodapeAtor->GetComponent<DrawTextComponent>()) {
        dc->SetText(texto);
    }
}

void CriacaoDePersonagem::Trocar(const int passo) {

    if (mLinhas.empty()) return;

    if (!Editavel(mLinhaEmFoco, mColunaEmFoco)) return;
    const Linha& linha = mLinhas[static_cast<size_t>(mLinhaEmFoco)];
    const Personagens::Categoria* c = Personagens::Carregado().Por(linha.categoria);
    if (c == nullptr) return;

    Personagens::Escolha e = *mAparencia.Por(linha.categoria);

    // Anda na lista de ids da categoria, dando a volta nas pontas.
    if (mColunaEmFoco == kCor) {
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

    // As linhas e, depois da ultima, o botao CONFIRMAR. O manche para nas pontas, sem dar a volta:
    // do topo, para cima nao pula para o botao.
    const int quantas = static_cast<int>(mLinhas.size()) + 1;

    // O painel da vez: quem monta a personagem do jogador 2 e o jogador 2, no manche dele.
    const Painel::Jogador kQuem = PainelDaVez();

    const bool cima     = Painel::Apertado(keyState, kQuem, Painel::Direcao::Cima);
    const bool baixo    = Painel::Apertado(keyState, kQuem, Painel::Direcao::Baixo);
    const bool esquerda = Painel::Apertado(keyState, kQuem, Painel::Direcao::Esquerda);
    const bool direita  = Painel::Apertado(keyState, kQuem, Painel::Direcao::Direita);

    // As bordas andam sempre, mesmo sem linha a escolher.
    const bool subiu = mBordaCima.Bateu(cima);
    const bool desceu = mBordaBaixo.Bateu(baixo);
    const bool paraEsquerda = mBordaEsquerda.Bateu(esquerda);
    const bool paraDireita = mBordaDireita.Bateu(direita);
    const bool trocouDeColuna = mBordaColuna.Bateu(Painel::Apertado(keyState, kQuem, Painel::Botao::Tres));
    {
        if (subiu) {
            mLinhaEmFoco = std::max(0, mLinhaEmFoco - 1);
            AjustarColuna();
            Redesenhar();
        }
        if (desceu) {
            mLinhaEmFoco = std::min(quantas - 1, mLinhaEmFoco + 1);
            AjustarColuna();
            Redesenhar();
        }
        // O Botao 3 passa para a outra coluna, se ela troca alguma coisa nesta linha.
        if (trocouDeColuna) {
            const int outra = mColunaEmFoco == kObjeto ? kCor : kObjeto;
            if (Editavel(mLinhaEmFoco, outra)) {
                mColunaPreferida = outra;
                mColunaEmFoco = outra;
                Redesenhar();
            }
        }
        if (paraEsquerda) Trocar(-1);
        if (paraDireita) Trocar(+1);
    }

    const bool confirmar = Painel::Apertado(keyState, kQuem, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (mBordaConfirmar.Bateu(confirmar)) {
        // So no CONFIRMAR: para chegar nele, o aluno passa por todas as linhas da personagem.
        if (NoBotao()) Confirmar();
        return;
    }

    const bool voltar = Painel::Apertado(keyState, kQuem, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        // Na edicao, volta para quem abriu sem gravar nada; na entrada, para a matricula.
        if (mGame->EditandoPersonagem()) {
            mGame->TerminarEdicaoDePersonagem();
            return;
        }
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }
}

void CriacaoDePersonagem::Confirmar() {
    mGame->DefinirAparencia(mAparencia);

    // Quem veio da dificuldade ou do boletim - um perfil que ja existia - volta para la.
    if (mGame->EditandoPersonagem()) {
        mGame->TerminarEdicaoDePersonagem();
        return;
    }

    // Nao vai direto para a selecao.
    mGame->AvancarEntrada();
}
