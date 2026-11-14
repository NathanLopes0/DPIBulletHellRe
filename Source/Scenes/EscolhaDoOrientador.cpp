//
// Antes do TCC: quem vai lutar do lado do aluno.
//

#include "EscolhaDoOrientador.h"
#include "TextoDeCena.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Regras/Nota.h"
#include "../Interface/Hud.h"
#include "../Curso/MateriasArquivo.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "../Actors/Actor.h"
#include "../Actors/Teachers/BossFactory/IBossFactory.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawLinhasComponent.h"
#include "../Regras/Golpe.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

EscolhaDoOrientador::EscolhaDoOrientador(Game* game)
    : Scene(game, SceneType::EscolhaDoOrientador),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
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

    // O mapa do curso e do jogador 1, mas aqui o que se mede e quem a dupla conhece melhor.
    std::vector<const Progresso*> alunos;
    for (const auto& sessao : mGame->Sessoes()) alunos.push_back(&sessao.progresso);

    mCandidatos = Orientacao::Candidatos(Materias::Carregadas(), alunos);
    const int quantos = std::min(static_cast<int>(alunos.size()), kMaxAlunos);
    mVotacao = Orientacao::Votacao(static_cast<int>(mCandidatos.size()), std::max(1, quantos));

    // Os cartoes centrados, quantos forem ate tres; com mais (os das optativas vem a mais), os tres
    // primeiros no lugar de sempre e os outros a direita, fora da tela - o carrossel os traz.
    const float meio = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    const int naTela = std::min(static_cast<int>(mCandidatos.size()), kVisiveis);
    const float primeiro = naTela == 0 ? 0.0f : -0.5f * static_cast<float>(naTela - 1);
    for (size_t i = 0; i < mCandidatos.size(); ++i) {
        mCentros.emplace_back(meio + (primeiro + static_cast<float>(i)) * kEntreCartoes, kCartaoY);
    }
    mBases = mCentros;

    CriarCabecalho();
    for (int i = 0; i < static_cast<int>(mCandidatos.size()); ++i) CriarCartao(i);
    CriarCursores();
    CriarRodape();

    // As setas dizem que ha cartoes escondidos de um lado ou do outro.
    if (static_cast<int>(mCandidatos.size()) > kVisiveis) {
        const float largura = static_cast<float>(mGame->GetWindowWidth());
        mSetaEsquerda = Texto("\xc2\xab", Vector2(22.0f, kCartaoY), 44, 40, Color::Gold);
        mSetaDireita = Texto("\xc2\xbb", Vector2(largura - 22.0f, kCartaoY), 44, 40, Color::Gold);
        // No comeco nada esta escondido a esquerda.
        if (auto* t = mSetaEsquerda->GetComponent<DrawTextComponent>()) t->SetIsVisible(false);
    }

    Atualizar();
}

Actor* EscolhaDoOrientador::Texto(const std::string& conteudo, const Vector2& onde, const int tamanho,
                                  const int largura, const Vector3& cor, const int linhas) {
    auto* dc = CriarTexto(*this, mFonte.get(), conteudo, onde, tamanho, largura,
                          (tamanho + 8) * linhas, kOrdemDoCursor - 10);
    dc->SetColor(cor);
    return dc->GetOwner();
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
    Texto("ESCOLHA O ORIENTADOR", Vector2(meio, 34.0f), 30, 700, Color::White);

    // Quem cobra e o professor da materia, e nao um nome escrito aqui.
    const Materias::Materia* m = Materias::Carregadas().Por(mGame->GetSelectedStage());
    const std::string banca = (m && !m->professor.empty())
        ? ("O Prof. " + m->professor + " cobra monografias e perguntas de banca.")
        : std::string("A banca cobra monografias e perguntas.");
    Texto(banca, Vector2(meio, 68.0f), 20, 1000, Color::LightBlue);

    Texto(EmDupla() ? "Cada um vota no poder que quer do lado: se discordarem, fica o da maior nota."
                    : "Quem luta ao seu lado? Cada professor traz um poder da matéria dele: é o botão 3.",
          Vector2(meio, 93.0f), 18, 1000, Color::LightYellow);
}

Actor* EscolhaDoOrientador::Guardar(Actor* ator) {
    if (ator && !mNosDosCartoes.empty()) mNosDosCartoes.back().push_back({ator, ator->GetPosition()});
    return ator;
}

void EscolhaDoOrientador::Deslizar(const float deltaTime) {
    const int n = static_cast<int>(mCandidatos.size());
    if (n <= kVisiveis) return;
    // O primeiro a vista acompanha os cursores: o do ultimo aluno que saiu da janela manda.
    for (int a = 0; a < mVotacao.Alunos(); ++a) {
        const int c = mVotacao.Cursor(a);
        if (c < mPrimeiroVisivel) mPrimeiroVisivel = c;
        if (c >= mPrimeiroVisivel + kVisiveis) mPrimeiroVisivel = c - kVisiveis + 1;
    }
    mPrimeiroVisivel = std::clamp(mPrimeiroVisivel, 0, n - kVisiveis);
    const float alvo = -static_cast<float>(mPrimeiroVisivel) * kEntreCartoes;
    if (std::fabs(alvo - mDeslocamento) < 0.5f) {
        if (mDeslocamento == alvo) return;
        mDeslocamento = alvo;
    } else {
        mDeslocamento += (alvo - mDeslocamento) * std::min(1.0f, 12.0f * deltaTime);
    }
    const Vector2 d(mDeslocamento, 0.0f);
    for (auto& cartao : mNosDosCartoes) {
        for (NoDoCartao& no : cartao) no.ator->SetPosition(no.base + d);
    }
    for (size_t i = 0; i < mCentros.size(); ++i) mCentros[i] = mBases[i] + d;
    for (PreviaDoCartao& p : mPrevias) p.canto = p.cantoBase + d;
    if (auto* t = mSetaEsquerda ? mSetaEsquerda->GetComponent<DrawTextComponent>() : nullptr) t->SetIsVisible(mPrimeiroVisivel > 0);
    if (auto* t = mSetaDireita ? mSetaDireita->GetComponent<DrawTextComponent>() : nullptr) t->SetIsVisible(mPrimeiroVisivel + kVisiveis < n);
    Atualizar();
}

void EscolhaDoOrientador::CriarCartao(const int indice) {

    mNosDosCartoes.emplace_back();
    const Orientacao::Candidato& c = mCandidatos[static_cast<size_t>(indice)];
    const Vector2 centro = mCentros[static_cast<size_t>(indice)];
    const Materias::Materia* m = Materias::Carregadas().Por(c.materia);

    auto cartao = std::make_unique<Actor>(this);
    cartao->SetPosition(centro);
    cartao->AddComponent<DrawCaixaComponent>(
        static_cast<int>(kCartaoLargura), static_cast<int>(kCartaoAltura),
        m && m->optativa ? kMolduraDaOptativa : kMolduraDoCartao, kFundoDoCartao, 2, kOrdemDoCartao);
    Guardar(AddActor(std::move(cartao)));

    // O que o cartao mostra e medido do topo dele.
    const Vector2 topo = centro + Vector2(0.0f, -kCartaoAltura / 2.0f);

    DrawSpriteComponent* retrato = nullptr;
    if (IBossFactory* fabrica = mGame->GetFactory(static_cast<size_t>(c.materia))) {
        auto rosto = std::make_unique<Actor>(this);
        rosto->SetPosition(topo + Vector2(0.0f, kRetratoEm));
        rosto->SetScale(kEscalaDoRetrato);
        retrato = rosto->AddComponent<DrawSpriteComponent>(
            Caminhos::Asset(fabrica->Retrato() + ".png"), kOrdemDoRetrato);
        Guardar(AddActor(std::move(rosto)));
    }
    mRetratos.push_back(retrato);

    // O orientador que a optativa deu vem a mais, e o cartao diz isso.
    if (m && m->optativa) {
        Guardar(Texto("OPTATIVA", topo + Vector2(-kCartaoLargura / 2.0f + 52.0f, 19.0f), 14, 90, Color::LightBlue));
    }

    const int largura = static_cast<int>(kCartaoLargura) - 30;
    Guardar(Texto("Prof. " + c.professor, topo + Vector2(0.0f, kProfessorEm), 24, largura, Color::White));
    Guardar(Texto(m ? m->nome : std::string(), topo + Vector2(0.0f, kMateriaEm), 20, largura, Color::LightYellow));
    Guardar(Texto(m ? m->nomeCompleto : std::string(), topo + Vector2(0.0f, kNomeDaMateriaEm), 16, largura,
                  Color::LightBlue, 2));

    std::stringstream nota;
    nota << "Nota " << std::fixed << std::setprecision(2) << c.nota;
    if (EmDupla()) nota << "  (de J" << c.deQuem + 1 << ")";
    const Vector3 corDaNota = Nota::ECheia(c.nota) ? Color::Gold : Color::LightGreen;
    Guardar(Texto(nota.str(), topo + Vector2(0.0f, kNotaEm), 20, largura, corDaNota));

    CriarCurriculo(indice, topo);
    CriarPrevia(indice, topo);
}

void EscolhaDoOrientador::CriarCurriculo(const int indice, const Vector2& topo) {

    // Quantas vezes ele ja orientou quem esta jogando: na dupla, a de quem mais foi orientado.
    const Orientacao::Candidato& c = mCandidatos[static_cast<size_t>(indice)];
    int vezes = 0;
    for (const auto& sessao : mGame->Sessoes()) vezes = std::max(vezes, sessao.progresso.Orientacoes(c.materia));

    const SDL_Color cor = vezes > 0 ? kCorDeQuemOrientou : kCorDoNovo;
    const Vector2 canto = topo + Vector2(kCartaoLargura / 2.0f - kCurriculoLargura / 2.0f - 8.0f,
                                         kCurriculoAltura / 2.0f + 8.0f);
    auto selo = std::make_unique<Actor>(this);
    selo->SetPosition(canto);
    selo->AddComponent<DrawCaixaComponent>(static_cast<int>(kCurriculoLargura), static_cast<int>(kCurriculoAltura),
                                           cor, kFundoDaPrevia, 1, kOrdemDoRetrato + 2);
    Guardar(AddActor(std::move(selo)));
    // \xc3\xa1 e o a com acento.
    const std::string texto = vezes > 0 ? "j\xc3\xa1 orientou " + std::to_string(vezes) + "x" : std::string("novo");
    auto* dc = CriarTexto(*this, mFonte.get(), texto, canto, 14, static_cast<int>(kCurriculoLargura) - 6,
                          static_cast<int>(kCurriculoAltura), kOrdemDoRetrato + 3);
    dc->SetColor(ParaVector3(cor));
    Guardar(dc->GetOwner());
}

void EscolhaDoOrientador::CriarPrevia(const int indice, const Vector2& topo) {

    const Orientacao::Candidato& c = mCandidatos[static_cast<size_t>(indice)];
    const Poder::Descricao poder = Poder::Da(Golpe::DoProfessor(c.professor));
    const int largura = static_cast<int>(kCartaoLargura) - 30;

    PreviaDoCartao previa;
    previa.tipo = poder.tipo;
    if (poder.tipo == Poder::Tipo::Nenhum) {
        mPrevias.push_back(previa);
        return;
    }

    Guardar(Texto("PODER: " + poder.nome, topo + Vector2(0.0f, kPoderEm), 22, largura, Color::Gold));

    // O campo em miniatura, e o desenho animado dentro dele.
    const Vector2 meio = topo + Vector2(0.0f, kPreviaEm);
    auto campo = std::make_unique<Actor>(this);
    campo->SetPosition(meio);
    campo->AddComponent<DrawCaixaComponent>(static_cast<int>(kPreviaLargura) + 8, static_cast<int>(kPreviaAltura) + 8,
                                            kMolduraDaPrevia, kFundoDaPrevia, 1, kOrdemDoRetrato);
    previa.linhas = campo->AddComponent<DrawLinhasComponent>(kOrdemDoRetrato + 1);
    previa.canto = meio - Vector2(kPreviaLargura / 2.0f, kPreviaAltura / 2.0f);
    previa.cantoBase = previa.canto;
    Guardar(AddActor(std::move(campo)));

    previa.legenda = Guardar(Texto(" ", topo + Vector2(0.0f, kLegendaEm), 16, largura, Color::White));
    Guardar(Texto(poder.efeito, topo + Vector2(0.0f, kEfeitoEm), 15, largura, Color::LightBlue, 2));
    mPrevias.push_back(previa);
}

SDL_Color EscolhaDoOrientador::CorDaMarca(const Poder::Marca marca) {
    switch (marca) {
        case Poder::Marca::TiroDoHugo: return SDL_Color{240, 96, 96, 255};
        case Poder::Marca::TiroSeu:    return SDL_Color{110, 230, 200, 255};
        case Poder::Marca::Aluno:      return SDL_Color{255, 255, 255, 255};
        case Poder::Marca::Hugo:       return SDL_Color{178, 150, 250, 255};
        case Poder::Marca::Figura:     return SDL_Color{255, 214, 72, 255};
        case Poder::Marca::Estrutura:  return SDL_Color{255, 190, 100, 235};
        case Poder::Marca::Apagado:    return SDL_Color{120, 128, 150, 255};
    }
    return SDL_Color{255, 255, 255, 255};
}

void EscolhaDoOrientador::DesenharPrevias() {

    for (PreviaDoCartao& previa : mPrevias) {
        if (!previa.linhas) continue;
        const Poder::Quadro quadro = Poder::Previa(previa.tipo, mTempoDaPrevia);
        auto noCampo = [&previa](const float x, const float y) {
            return previa.canto + Vector2(x * kPreviaLargura, y * kPreviaAltura);
        };

        previa.linhas->Limpar();
        for (const Poder::Traco& t : quadro.tracos) {
            previa.linhas->Acrescentar(noCampo(t.x0, t.y0), noCampo(t.x1, t.y1), CorDaMarca(t.cor));
        }
        for (const Poder::Ponto& p : quadro.pontos) {
            const Vector2 c = noCampo(p.x, p.y);
            const SDL_Color cor = CorDaMarca(p.marca);
            auto poligono = [&](const std::vector<Vector2>& cantos) {
                for (size_t i = 0; i < cantos.size(); ++i) {
                    previa.linhas->Acrescentar(c + cantos[i], c + cantos[(i + 1) % cantos.size()], cor);
                }
            };
            switch (p.marca) {
                case Poder::Marca::Aluno:      // um triangulo, de ponta para cima
                    poligono({Vector2(0.0f, -7.0f), Vector2(6.0f, 5.0f), Vector2(-6.0f, 5.0f)});
                    break;
                case Poder::Marca::Hugo:       // o cartao do professor
                    poligono({Vector2(-9.0f, -7.0f), Vector2(9.0f, -7.0f), Vector2(9.0f, 7.0f), Vector2(-9.0f, 7.0f)});
                    poligono({Vector2(-5.0f, -3.0f), Vector2(5.0f, -3.0f), Vector2(5.0f, 3.0f), Vector2(-5.0f, 3.0f)});
                    break;
                case Poder::Marca::Figura:
                    poligono({Vector2(-4.0f, -4.0f), Vector2(4.0f, -4.0f), Vector2(4.0f, 4.0f), Vector2(-4.0f, 4.0f)});
                    break;
                case Poder::Marca::TiroSeu:    // sobe: um traco em pe
                    poligono({Vector2(0.0f, -4.0f), Vector2(2.0f, 0.0f), Vector2(0.0f, 4.0f), Vector2(-2.0f, 0.0f)});
                    break;
                default:                       // o tiro do Hugo, e o que esta pendente
                    poligono({Vector2(0.0f, -3.0f), Vector2(3.0f, 0.0f), Vector2(0.0f, 3.0f), Vector2(-3.0f, 0.0f)});
                    poligono({Vector2(0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(-1.0f, 0.0f)});
                    break;
            }
        }

        // A legenda so e reescrita quando o passo muda.
        if (quadro.legenda != previa.escrita) {
            previa.escrita = quadro.legenda;
            Escrever(previa.legenda, quadro.legenda, Color::White);
        }
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

    mEstado = Texto(" ", Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f, 704.0f),
                    24, 1000, Color::White);
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

    // Uma linha por aluno, da cor dele.
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
            // O enter e o ESC de quem desenvolve, como na selecao de fases.
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
                // Sem voto para desfazer, o J1 desiste do TCC.
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

    mTempoDaPrevia += deltaTime;
    Deslizar(deltaTime);
    DesenharPrevias();

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

    // Sem candidato a batalha segue sozinha, depois do mesmo anuncio.
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

        cursor->SetPosition(centro);
        if (auto* caixa = cursor->GetComponent<DrawCaixaComponent>()) {
            SDL_Color cor = CorDoAluno(a);
            cor.a = votou ? 255 : 150;
            caixa->SetMoldura(cor);
            caixa->SetIsVisible(temCartao);
        }

        // Em dupla os selos ficam lado a lado, para os dois caberem no mesmo cartao.
        const float lado = EmDupla() ? (a == 0 ? -72.0f : 72.0f) : 0.0f;
        selo->SetPosition(centro + Vector2(lado, -kCartaoAltura / 2.0f - kSeloAcima));
        if (auto* dc = selo->GetComponent<DrawTextComponent>()) dc->SetIsVisible(temCartao && votou);
    }

    // Enquanto o escolhido e anunciado, os outros escurecem.
    for (size_t i = 0; i < mRetratos.size(); ++i) {
        if (!mRetratos[i]) continue;
        const Uint8 luz = (mEscolhido < 0 || static_cast<int>(i) == mEscolhido) ? 255 : kRetratoApagado;
        mRetratos[i]->SetColor(luz, luz, luz);
    }

    Vector3 cor = Color::White;
    const std::string frase = FraseDoEstado(cor);
    Escrever(mEstado, frase, cor);
}
