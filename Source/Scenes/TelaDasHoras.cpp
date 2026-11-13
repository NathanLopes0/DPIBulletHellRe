//
// A tela das horas complementares.
//

#include "TelaDasHoras.h"

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "TextoDeCena.h"
#include "../Actors/Actor.h"
#include "../Aluno/Horas.h"
#include "../Aluno/Matricula.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Curso/MateriasArquivo.h"
#include "../Entrada/Painel.h"
#include "../Interface/Hud.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Font.h"
#include "../Motor/Fontes.h"
#include "../Motor/Game.h"

namespace {
    constexpr float kColunaTitulo    = 0.08f;
    constexpr float kColunaDescricao = 0.30f;
    constexpr float kColunaHoras     = 0.70f;
    constexpr float kColunaCumpridas = 0.82f;
    constexpr float kTopo = 0.29f;
    constexpr float kPasso = 0.052f;

    const Vector3 kApagado(0.55f, 0.57f, 0.62f);
    const Vector3 kTituloDaColuna(0.62f, 0.78f, 0.95f);
}

TelaDasHoras::TelaDasHoras(Game* game)
    : Scene(game, SceneType::Horas),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

Actor* TelaDasHoras::Texto(const float x, const float y, const int tamanho, const int largura, const bool aEsquerda) {
    const Vector2 onde(aEsquerda ? x + static_cast<float>(largura) / 2.0f : x, y);
    DrawTextComponent* texto = CriarTexto(*this, mFonte.get(), " ", onde, tamanho, largura, tamanho + 8);
    texto->SetAEsquerda(aEsquerda);
    return texto->GetOwner();
}

void TelaDasHoras::Escrever(Actor* ator, const std::string& texto, const Vector3& cor) {
    if (!ator) return;
    if (auto* t = ator->GetComponent<DrawTextComponent>()) {
        t->SetText(texto.empty() ? " " : texto);
        t->SetColor(cor);
    }
}

void TelaDasHoras::Load() {

    CriarFundoDoCorredor(*this);

    const auto altura  = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    Escrever(Texto(largura / 2.0f, altura * 0.075f, 40, 900), "HORAS COMPLEMENTARES", Color::White);
    mAlunoAtor = Texto(largura / 2.0f, altura * 0.13f, 24, 900);
    mTotalAtor = Texto(largura / 2.0f, altura * 0.185f, 30, 1000);

    // \xc3\xa7\xc3\xa3 e "ca" de descricao.
    const float yTitulos = altura * 0.245f;
    Escrever(Texto(largura * kColunaTitulo, yTitulos, 18, 240, true), "Atividade", kTituloDaColuna);
    Escrever(Texto(largura * kColunaDescricao, yTitulos, 18, 440, true), "O que pede, numa prova aprovada", kTituloDaColuna);
    Escrever(Texto(largura * kColunaHoras, yTitulos, 18, 130, true), "Vale", kTituloDaColuna);
    Escrever(Texto(largura * kColunaCumpridas, yTitulos, 18, 150, true), "Cumprida", kTituloDaColuna);

    for (size_t i = 0; i < Horas::Todas().size(); ++i) {
        const float y = altura * (kTopo + kPasso * static_cast<float>(i));
        LinhaDeTela l;
        l.titulo    = Texto(largura * kColunaTitulo, y, 22, 250, true);
        l.descricao = Texto(largura * kColunaDescricao, y, 20, 460, true);
        l.horas     = Texto(largura * kColunaHoras, y, 20, 130, true);
        l.cumpridas = Texto(largura * kColunaCumpridas, y, 20, 150, true);
        mLinhas.push_back(l);
    }

    mRodapeAtor = Texto(largura / 2.0f, altura * 0.96f, 20, 1100);
    Redesenhar();
}

void TelaDasHoras::Redesenhar() {

    const Materias::Lista& materias = Materias::Carregadas();
    const auto& sessoes = mGame->Sessoes();
    if (mAluno >= sessoes.size()) mAluno = 0;
    const bool dupla = sessoes.size() > 1;
    const std::string& matricula = mGame->MatriculaDe(mAluno);
    const bool visitante = matricula.empty() || matricula == Matricula::kVisitante;

    // \xc3\xa3 e o a com til, \xc3\xad o i com acento.
    std::string quem = visitante ? std::string("Visitante - n\xc3\xa3o salva") : "Matr\xc3\xad" "cula " + matricula;
    if (dupla) quem = "Aluno " + std::to_string(mAluno + 1) + ": " + quem;
    const Hud::Rgb c = Hud::CorDoAluno(mAluno);
    Escrever(mAlunoAtor, quem, dupla ? Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f) : Color::LightBlue);

    const auto& chaves = sessoes[mAluno].progresso.Atividades();
    const int horas = Horas::De(chaves);
    const bool cumpriu = horas >= Horas::kMeta;
    // O saldo e o que ainda se gasta com optativas; a carga conta tudo o que foi ganho.
    const int gastas = Horas::Gastas(chaves, materias);
    const std::string saldo = "  \xc2\xb7  para optativas: " + std::to_string(Horas::Saldo(chaves, materias)) + " h" +
                              (gastas > 0 ? " (" + std::to_string(gastas) + " gastas)" : "");
    Escrever(mTotalAtor,
             std::to_string(horas) + " h de " + std::to_string(Horas::kMeta) +
                 (cumpriu ? "  \xc2\xb7  carga cumprida!" : "  \xc2\xb7  faltam " + std::to_string(Horas::kMeta - horas) + " h") +
                 saldo,
             cumpriu ? Color::Gold : Color::White);

    const auto quadro = Horas::Quadro(chaves, materias);
    for (size_t i = 0; i < mLinhas.size() && i < quadro.size(); ++i) {
        const Horas::Linha& l = quadro[i];
        const bool toda = l.cumpridas >= l.possiveis;
        const Vector3 cor = toda ? Color::Gold : (l.cumpridas > 0 ? Color::LightGreen : Color::White);
        Escrever(mLinhas[i].titulo, l.dados.titulo, cor);
        Escrever(mLinhas[i].descricao, l.dados.descricao, l.cumpridas > 0 ? Color::White : kApagado);
        // \xc3\xa9 e o e com acento.
        Escrever(mLinhas[i].horas,
                 std::to_string(l.dados.horas) + " h" + (l.dados.porMateria ? " por mat\xc3\xa9ria" : ""), Color::LightBlue);
        Escrever(mLinhas[i].cumpridas,
                 l.dados.porMateria ? std::to_string(l.cumpridas) + " de " + std::to_string(l.possiveis)
                                    : std::string(l.cumpridas > 0 ? "sim" : "-"),
                 cor);
    }

    std::vector<Painel::Acao> itens;
    if (dupla) itens.push_back(Painel::Manche(Painel::Jogador::Um, "trocar de aluno"));
    itens.emplace_back(Painel::Jogador::Um, Painel::Botao::Dois, "voltar ao boletim");
    Escrever(mRodapeAtor, Painel::Rodape(itens, mGame->JeitoDoRodape()), Color::White);
}

void TelaDasHoras::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        mGame->RequestSceneChange(SceneType::Boletim);
        return;
    }

    const bool esquerda = mBordaEsquerda.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Esquerda));
    const bool direita = mBordaDireita.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Direita));
    const size_t quantos = mGame->Sessoes().size();
    if ((esquerda || direita) && quantos > 1) {
        mAluno = (mAluno + (direita ? 1 : quantos - 1)) % quantos;
        Redesenhar();
    }
}
