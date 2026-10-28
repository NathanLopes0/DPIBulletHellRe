//
// Antes do TCC: quem vai lutar do lado do aluno.
//

#include "EscolhaDoOrientador.h"

#include <iomanip>
#include <sstream>

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Game.h"
#include "../Font.h"
#include "../Nota.h"
#include "../Hud.h"
#include "../MateriasArquivo.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Actors/Teachers/BossFactory/IBossFactory.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

EscolhaDoOrientador::EscolhaDoOrientador(Game* game)
    : Scene(game, SceneType::EscolhaDoOrientador),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

Painel::Jogador EscolhaDoOrientador::DoAluno(const int aluno) {
    return aluno == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
}

Vector3 EscolhaDoOrientador::ParaVector3(const SDL_Color& cor) {
    return Vector3(static_cast<float>(cor.r) / 255.0f, static_cast<float>(cor.g) / 255.0f,
                   static_cast<float>(cor.b) / 255.0f);
}

SDL_Color EscolhaDoOrientador::CorDoAluno(const int aluno) const {
    if (!EmDupla()) return kCorSozinho;
    const Hud::Rgb c = Hud::CorDoAluno(aluno == 1 ? 1 : 0);
    return SDL_Color{c.r, c.g, c.b, 255};
}

void EscolhaDoOrientador::Load() {

    CriarFundoDoCorredor(*this);

    // UM PROGRESSO POR ALUNO: em dupla a nota de cada materia e a maior dos
    // dois (Orientacao::Candidatos). O mapa do curso e do jogador 1 (RF3), mas
    // aqui o que se mede e quem a dupla conhece melhor.
    std::vector<const Progresso*> alunos;
    for (const auto& sessao : mGame->Sessoes()) alunos.push_back(&sessao.progresso);

    mCandidatos = Orientacao::Candidatos(Materias::Carregadas(), alunos);
    const int quantos = std::min(static_cast<int>(alunos.size()), kMaxAlunos);
    mVotacao = Orientacao::Votacao(static_cast<int>(mCandidatos.size()), std::max(1, quantos));

    // OS CARTOES CENTRADOS, quantos forem. Quem abre o TCC tem sempre tres (o
    // teste de dados garante), mas a tela nao pode depender disso para nao
    // desenhar um buraco.
    const float meio = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    const float primeiro = mCandidatos.empty() ? 0.0f : -0.5f * static_cast<float>(mCandidatos.size() - 1);
    for (size_t i = 0; i < mCandidatos.size(); ++i) {
        mCentros.emplace_back(meio + (primeiro + static_cast<float>(i)) * kEntreCartoes, kCartaoY);
    }

    CriarCabecalho();
    for (int i = 0; i < static_cast<int>(mCandidatos.size()); ++i) CriarCartao(i);
    CriarCursores();
    CriarRodape();

    Atualizar();
}

Actor* EscolhaDoOrientador::Texto(const std::string& conteudo, const Vector2& onde, const int tamanho,
                                  const int largura, const Vector3& cor, const int linhas) {
    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(onde);
    const auto dc = ator->AddComponent<DrawTextComponent>(
        conteudo.empty() ? std::string(" ") : conteudo, mFonte.get(), largura,
        (tamanho + 8) * linhas, tamanho, kOrdemDoCursor - 10);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(largura));
    dc->SetAjustarAoTexto(true);
    dc->SetColor(cor);

    Actor* observador = ator.get();
    AddActor(std::move(ator));
    return observador;
}

void EscolhaDoOrientador::Escrever(Actor* ator, const std::string& texto, const Vector3& cor) {
    if (!ator) return;
    if (const auto dc = ator->GetComponent<DrawTextComponent>()) {
        // Espaco, e nao vazio: ver StageSelect::EscreverNoCartao.
        dc->SetText(texto.empty() ? std::string(" ") : texto);
        dc->SetColor(cor);
    }
}

void EscolhaDoOrientador::CriarCabecalho() {

    const float meio = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    Texto("ESCOLHA O ORIENTADOR", Vector2(meio, 58.0f), 34, 700, Color::White);

    // QUEM COBRA E O PROFESSOR DA MATERIA, e nao um nome escrito aqui: a
    // materia que pede orientacao e a que foi selecionada.
    const Materias::Materia* m = Materias::Carregadas().Por(mGame->GetSelectedStage());
    const std::string banca = (m && !m->professor.empty())
        ? ("O Prof. " + m->professor + " cobra monografias e perguntas de banca.")
        : std::string("A banca cobra monografias e perguntas.");
    Texto(banca, Vector2(meio, 104.0f), 22, 1000, Color::LightBlue);

    Texto(EmDupla() ? "Cada um vota. Se discordarem, fica o professor da maior nota."
                    : "Quem luta ao seu lado? Um professor das suas maiores notas.",
          Vector2(meio, 136.0f), 20, 1000, Color::LightYellow);
}

void EscolhaDoOrientador::CriarCartao(const int indice) {

    const Orientacao::Candidato& c = mCandidatos[static_cast<size_t>(indice)];
    const Vector2 centro = mCentros[static_cast<size_t>(indice)];
    const Materias::Materia* m = Materias::Carregadas().Por(c.materia);

    auto cartao = std::make_unique<Actor>(this);
    cartao->SetPosition(centro);
    cartao->AddComponent<DrawCaixaComponent>(
        static_cast<int>(kCartaoLargura), static_cast<int>(kCartaoAltura),
        kMolduraDoCartao, kFundoDoCartao, 2, kOrdemDoCartao);
    AddActor(std::move(cartao));

    // O RETRATO E O CARTAO DE CHEFE DELE, lido da fabrica (IBossFactory::Retrato):
    // o rosto que o aluno enfrentou na materia e o que agora luta do lado dele.
    DrawSpriteComponent* retrato = nullptr;
    if (IBossFactory* fabrica = mGame->GetFactory(static_cast<size_t>(c.materia))) {
        auto rosto = std::make_unique<Actor>(this);
        rosto->SetPosition(centro + Vector2(0.0f, -96.0f));
        rosto->SetScale(kEscalaDoRetrato);
        retrato = rosto->AddComponent<DrawSpriteComponent>(
            Caminhos::Asset(fabrica->Retrato() + ".png"), kOrdemDoRetrato);
        AddActor(std::move(rosto));
    }
    mRetratos.push_back(retrato);

    const int largura = static_cast<int>(kCartaoLargura) - 30;
    Texto("Prof. " + c.professor, centro + Vector2(0.0f, 20.0f), 26, largura, Color::White);
    Texto(m ? m->nome : std::string(), centro + Vector2(0.0f, 56.0f), 22, largura, Color::LightYellow);
    Texto(m ? m->nomeCompleto : std::string(), centro + Vector2(0.0f, 98.0f), 18, largura,
          Color::LightBlue, 2);

    // A NOTA QUE TROUXE O PROFESSOR ATE AQUI - e, na dupla, de quem ela e.
    std::stringstream nota;
    nota << "Nota " << std::fixed << std::setprecision(2) << c.nota;
    const Vector3 corDaNota = Nota::ECheia(c.nota) ? Color::Gold : Color::LightGreen;
    Texto(nota.str(), centro + Vector2(0.0f, 146.0f), 22, largura, corDaNota);
    if (EmDupla()) {
        Texto("de J" + std::to_string(c.deQuem + 1), centro + Vector2(0.0f, 176.0f), 16, largura,
              ParaVector3(CorDoAluno(c.deQuem)));
    }
}

void EscolhaDoOrientador::CriarCursores() {

    for (int a = 0; a < mVotacao.Alunos(); ++a) {
        const float folga = kFolgaDoCursor + kFolgaEntreCursores * static_cast<float>(a);

        auto cursor = std::make_unique<Actor>(this);
        cursor->AddComponent<DrawCaixaComponent>(
            static_cast<int>(kCartaoLargura + 2.0f * folga), static_cast<int>(kCartaoAltura + 2.0f * folga),
            CorDoAluno(a), kSemFundo, kEspessuraDoCursor, kOrdemDoCursor);
        mCursores[static_cast<size_t>(a)] = cursor.get();
        AddActor(std::move(cursor));

        const std::string selo = EmDupla() ? ("J" + std::to_string(a + 1) + " VOTOU") : std::string("ESCOLHIDO");
        mSelos[static_cast<size_t>(a)] = Texto(selo, Vector2(0.0f, 0.0f), 20, 160, ParaVector3(CorDoAluno(a)));
    }

    mEstado = Texto(" ", Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f, 688.0f),
                    26, 1000, Color::White);
}

void EscolhaDoOrientador::CriarRodape() {

    const float meio = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    const float base = static_cast<float>(mGame->GetWindowHeight()) - 34.0f;

    if (!EmDupla()) {
        Texto(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                              {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                              {Painel::Jogador::Um, Painel::Botao::Dois, "desfazer ou voltar"}},
                             mGame->JeitoDoRodape()),
              Vector2(meio, base), 22, 1100, Color::White);
        return;
    }

    // UMA LINHA POR ALUNO, da cor dele: as teclas de cada um sao outras, e no
    // gabinete e o lado do painel que diz de quem e o manche.
    for (int a = 0; a < mVotacao.Alunos(); ++a) {
        const Painel::Jogador quem = DoAluno(a);
        const std::string linha = "J" + std::to_string(a + 1) + "   " + Painel::Rodape(
            {Painel::Manche(quem, "escolher"),
             {quem, Painel::Botao::Um, "votar"},
             {quem, Painel::Botao::Dois, a == 0 ? "desfazer ou voltar" : "desfazer"}},
            mGame->JeitoDoRodape());
        Texto(linha, Vector2(meio, base - 34.0f * static_cast<float>(mVotacao.Alunos() - 1 - a)),
              20, 1100, ParaVector3(CorDoAluno(a)));
    }
}

void EscolhaDoOrientador::OnProcessInput(const Uint8* keyState) {

    if (mSaindo) return;

    for (int a = 0; a < mVotacao.Alunos(); ++a) {
        const Painel::Jogador quem = DoAluno(a);
        Bordas& antes = mBordas[static_cast<size_t>(a)];

        const bool esquerda = Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda);
        const bool direita  = Painel::Apertado(keyState, quem, Painel::Direcao::Direita);
        bool votar    = Painel::Apertado(keyState, quem, Painel::Botao::Um);
        bool desfazer = Painel::Apertado(keyState, quem, Painel::Botao::Dois);
        if (a == 0) {
            // O ENTER e o ESC de quem desenvolve, como na selecao de fases.
            votar = votar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
            desfazer = desfazer || keyState[SDL_SCANCODE_ESCAPE];
        }

        if (esquerda && !antes.esquerda) { mVotacao.Mover(a, -1); mMudou = true; }
        if (direita && !antes.direita)   { mVotacao.Mover(a, +1); mMudou = true; }
        if (votar && !antes.votar)       { mVotacao.Votar(a);     mMudou = true; }
        if (desfazer && !antes.desfazer) {
            if (mVotacao.Desfazer(a)) {
                mMudou = true;
            } else if (a == 0) {
                // SEM VOTO PARA DESFAZER, O J1 DESISTE DO TCC: volta para a
                // selecao, sem orientador nenhum guardado.
                mSaindo = true;
                mGame->DefinirOrientador(-1);
                mGame->RequestSceneChange(SceneType::StageSelect);
                return;
            }
        }
        antes = Bordas{esquerda, direita, votar, desfazer};
    }
}

void EscolhaDoOrientador::OnUpdate(const float deltaTime) {

    if (mSaindo) return;

    const int escolhido = Orientacao::Resultado(mCandidatos, mVotacao.Votos());
    if (escolhido != mEscolhido) {
        mEscolhido = escolhido;
        mAnunciando = 0.0f;
        mMudou = true;
    }
    if (mMudou) {
        Atualizar();
        mMudou = false;
    }

    // SEM CANDIDATO A BATALHA SEGUE SOZINHA, depois do mesmo anuncio: e melhor
    // que prender o aluno numa tela sem nada para escolher.
    const bool decidido = mCandidatos.empty() || escolhido >= 0;
    if (!decidido) return;

    mAnunciando += deltaTime;
    if (mAnunciando < kAnuncio) return;

    mSaindo = true;
    mGame->DefinirOrientador(escolhido >= 0 ? mCandidatos[static_cast<size_t>(escolhido)].materia : -1);
    mGame->RequestSceneChange(SceneType::Battle);
}

std::string EscolhaDoOrientador::FraseDoEstado(Vector3& cor) const {

    if (mCandidatos.empty()) {
        cor = Color::LightPink;
        return "Nenhum professor para orientar - a banca sera sozinha.";
    }

    if (mEscolhido >= 0) {
        cor = Color::Gold;
        const std::string quem = "Prof. " + mCandidatos[static_cast<size_t>(mEscolhido)].professor;
        if (!EmDupla())               return "Orientador: " + quem;
        if (mVotacao.Concordaram())   return "Os dois escolheram o " + quem;
        return "Votos diferentes: fica o " + quem + ", da maior nota";
    }

    cor = Color::LightBlue;
    if (!EmDupla()) return " ";

    const bool um = mVotacao.Votou(0);
    const bool dois = mVotacao.Votou(1);
    if (!um && !dois) return "Faltam os dois votos";
    return um ? "Falta o voto do J2" : "Falta o voto do J1";
}

void EscolhaDoOrientador::Atualizar() {

    const bool temCartao = !mCentros.empty();

    for (int a = 0; a < mVotacao.Alunos(); ++a) {
        Actor* cursor = mCursores[static_cast<size_t>(a)];
        Actor* selo = mSelos[static_cast<size_t>(a)];
        if (!cursor || !selo) continue;

        const Vector2 centro = temCartao ? mCentros[static_cast<size_t>(mVotacao.Cursor(a))] : Vector2::Zero;
        const bool votou = mVotacao.Votou(a);

        // O CURSOR ACENDE QUANDO TRAVA: andando ele e meio apagado, votado e
        // a cor cheia, com o selo em cima do cartao.
        cursor->SetPosition(centro);
        if (auto* caixa = cursor->GetComponent<DrawCaixaComponent>()) {
            SDL_Color cor = CorDoAluno(a);
            cor.a = votou ? 255 : 150;
            caixa->SetMoldura(cor);
            caixa->SetIsVisible(temCartao);
        }

        // Em dupla os selos ficam lado a lado, para os dois caberem no mesmo cartao.
        const float lado = EmDupla() ? (a == 0 ? -72.0f : 72.0f) : 0.0f;
        selo->SetPosition(centro + Vector2(lado, -kCartaoAltura / 2.0f - 44.0f));
        if (auto* dc = selo->GetComponent<DrawTextComponent>()) dc->SetIsVisible(temCartao && votou);
    }

    // ENQUANTO O ESCOLHIDO E ANUNCIADO, OS OUTROS ESCURECEM.
    for (size_t i = 0; i < mRetratos.size(); ++i) {
        if (!mRetratos[i]) continue;
        const Uint8 luz = (mEscolhido < 0 || static_cast<int>(i) == mEscolhido) ? 255 : kRetratoApagado;
        mRetratos[i]->SetColor(luz, luz, luz);
    }

    Vector3 cor = Color::White;
    const std::string frase = FraseDoEstado(cor);
    Escrever(mEstado, frase, cor);
}
