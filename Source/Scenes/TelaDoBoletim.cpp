//
// A tela do boletim do semestre: as materias do aluno, periodo a periodo.
//

#include "TelaDoBoletim.h"

#include <SDL_scancode.h>
#include <cstdio>

#include "FundoDeCena.h"
#include "TextoDeCena.h"
#include "../Actors/Actor.h"
#include "../Aluno/FichaArquivo.h"
#include "../Aluno/Horas.h"
#include "../Aluno/Matricula.h"
#include "../Aluno/Ranking.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Curso/MateriasArquivo.h"
#include "../Entrada/Painel.h"
#include "../Interface/Hud.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Font.h"
#include "../Motor/Fontes.h"
#include "../Motor/Game.h"
#include "../Regras/Nota.h"

namespace {
    /// As colunas, em fracao da largura: a materia a esquerda, o resto em colunas estreitas.
    constexpr float kColunaMateria    = 0.04f;
    constexpr float kColunaProfessor  = 0.40f;
    constexpr float kColunaNota       = 0.515f;
    constexpr float kColunaSituacao   = 0.615f;
    constexpr float kColunaTentativas = 0.735f;
    constexpr float kColunaMencoes    = 0.79f;

    constexpr float kTopoDaTabela = 0.215f;
    constexpr float kPassoDaTabela = 0.036f;
    constexpr int kLetraDaTabela = 20;

    const Vector3 kApagado(0.55f, 0.57f, 0.62f);
    const Vector3 kTituloDaColuna(0.62f, 0.78f, 0.95f);

    Vector3 CorDa(const Boletim::Linha& l) {
        switch (l.situacao) {
            case Boletim::Situacao::Aprovade:  return Nota::ECheia(l.melhor) ? Color::Gold : Color::LightGreen;
            case Boletim::Situacao::Reprovade: return Color::LightPink;
            case Boletim::Situacao::ACursar:   return Color::White;
            case Boletim::Situacao::Fechada:   return kApagado;
        }
        return Color::White;
    }
}

TelaDoBoletim::TelaDoBoletim(Game* game)
    : Scene(game, SceneType::Boletim),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset(Fontes::kJogo));
}

void TelaDoBoletim::Load() {

    CriarFundoDoCorredor(*this);

    const auto altura  = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    // A turma, para a posicao no curso.
    const Materias::Lista& materias = Materias::Carregadas();
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        mFichas.push_back(f);
    }

    Texto("BOLETIM DO SEMESTRE", largura / 2.0f, altura * 0.075f, 40, 900);
    mAlunoAtor = Texto(" ", largura / 2.0f, altura * 0.13f, 24, 900);

    // Os titulos das colunas.
    const float yTitulos = altura * 0.18f;
    const auto titulo = [&](const char* t, const float x, const int w) {
        Escrever(Texto(t, largura * x, yTitulos, 18, w, true), t, kTituloDaColuna);
    };
    titulo("Mat\xc3\xa9ria", kColunaMateria, 400);
    titulo("Professor", kColunaProfessor, 130);
    titulo("Melhor nota", kColunaNota, 110);
    titulo("Situa\xc3\xa7\xc3\xa3o", kColunaSituacao, 130);
    titulo("Tent.", kColunaTentativas - 0.01f, 60);
    titulo("Men\xc3\xa7\xc3\xb5" "es", kColunaMencoes, 200);

    for (size_t i = 0; i < kLinhasDaTabela; ++i) {
        const float y = altura * (kTopoDaTabela + kPassoDaTabela * static_cast<float>(i));
        LinhaDeTela l;
        l.materia    = Texto(" ", largura * kColunaMateria, y, kLetraDaTabela, static_cast<int>(largura * 0.35f), true);
        l.professor  = Texto(" ", largura * kColunaProfessor, y, kLetraDaTabela, 130, true);
        l.nota       = Texto(" ", largura * kColunaNota, y, kLetraDaTabela, 110, true);
        l.situacao   = Texto(" ", largura * kColunaSituacao, y, kLetraDaTabela, 130, true);
        l.tentativas = Texto(" ", largura * kColunaTentativas, y, kLetraDaTabela, 50, true);
        l.mencoes    = std::make_unique<SelosDeMencao>(*this, mFonte.get(), Vector2(largura * kColunaMencoes, y));
        mLinhas.push_back(std::move(l));
    }

    // A legenda dos selos, embaixo da tabela.
    Escrever(Texto(" ", largura / 2.0f, altura * 0.835f, 16, 1150), Mencoes::Legenda(), kTituloDaColuna);

    mResumoAtor  = Texto(" ", largura / 2.0f, altura * 0.875f, 26, 1100);
    mPosicaoAtor = Texto(" ", largura / 2.0f, altura * 0.915f, 20, 1100);
    mRodapeAtor  = Texto(" ", largura / 2.0f, altura * 0.96f, 20, 1100);

    Redesenhar();
}

Actor* TelaDoBoletim::Texto(const std::string& conteudo, const float x, const float y, const int tamanho,
                            const int larguraMaxima, const bool aEsquerda) {
    // Uma coluna alinhada a esquerda e desenhada a partir do x dado, e nao centrada nele.
    const Vector2 onde(aEsquerda ? x + static_cast<float>(larguraMaxima) / 2.0f : x, y);
    DrawTextComponent* texto = CriarTexto(*this, mFonte.get(), conteudo, onde, tamanho, larguraMaxima, tamanho + 8);
    texto->SetAEsquerda(aEsquerda);
    return texto->GetOwner();
}

void TelaDoBoletim::Escrever(Actor* ator, const std::string& texto, const Vector3& cor) {
    if (!ator) return;
    if (auto* t = ator->GetComponent<DrawTextComponent>()) {
        // Espaco, e nao vazio: texto vazio nao gera textura.
        t->SetText(texto.empty() ? " " : texto);
        t->SetColor(cor);
    }
}

void TelaDoBoletim::Redesenhar() {

    const Materias::Lista& materias = Materias::Carregadas();
    const auto& sessoes = mGame->Sessoes();
    if (mAluno >= sessoes.size()) mAluno = 0;
    const bool dupla = sessoes.size() > 1;
    const std::string& matricula = mGame->MatriculaDe(mAluno);
    const bool visitante = matricula.empty() || matricula == Matricula::kVisitante;

    // Quem: a matricula, ou o aviso de que o visitante nao salva; na dupla, na cor do aluno.
    std::string quem = visitante ? std::string("Visitante - n\xc3\xa3o salva") : "Matr\xc3\xad" "cula " + matricula;
    if (dupla) quem = "Aluno " + std::to_string(mAluno + 1) + ": " + quem;
    const Hud::Rgb c = Hud::CorDoAluno(mAluno);
    Escrever(mAlunoAtor, quem, dupla ? Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f) : Color::LightBlue);

    // A tabela: o titulo de cada periodo, e as materias dele.
    const Boletim::Dados d = Boletim::Montar(sessoes[mAluno].progresso, materias);
    size_t i = 0;
    int periodo = 0;
    const auto vazia = [&](const LinhaDeTela& l) {
        for (Actor* a : {l.materia, l.professor, l.nota, l.situacao, l.tentativas}) Escrever(a, " ", Color::White);
        l.mencoes->Esconder();
    };
    for (const Boletim::Linha& l : d.linhas) {
        if (l.periodo != periodo && i < mLinhas.size()) {
            periodo = l.periodo;
            vazia(mLinhas[i]);
            Escrever(mLinhas[i].materia, Boletim::Periodo(periodo), Color::Gold);
            ++i;
        }
        if (i >= mLinhas.size()) break;
        const Vector3 cor = CorDa(l);
        const std::string nome = l.nome.empty() ? l.codigo : l.codigo + " - " + l.nome;
        Escrever(mLinhas[i].materia, "   " + nome, l.situacao == Boletim::Situacao::Fechada ? kApagado : Color::White);
        Escrever(mLinhas[i].professor, l.professor, l.situacao == Boletim::Situacao::Fechada ? kApagado : Color::White);
        Escrever(mLinhas[i].nota, Boletim::Nota(l.melhor), cor);
        Escrever(mLinhas[i].situacao, Boletim::NomeDa(l.situacao), cor);
        Escrever(mLinhas[i].tentativas, Boletim::Tentativas(l), Color::White);
        mLinhas[i].mencoes->Mostrar(Mencoes::Da(sessoes[mAluno].progresso, materias, l.materia));
        ++i;
    }
    for (; i < mLinhas.size(); ++i) vazia(mLinhas[i]);

    // O resumo: o CR, as aprovadas e as mencoes do curso.
    const Mencoes::Total mencoes = Mencoes::DoCurso(sessoes[mAluno].progresso, materias);
    char cr[32];
    std::snprintf(cr, sizeof(cr), "%.2f", static_cast<double>(d.cr));
    std::string resumo = std::string("CR: ") + (d.cursadas > 0 ? cr : "-") + "     Aprovadas: " +
                         std::to_string(d.aprovadas) + " de " + std::to_string(d.total);
    resumo += "     Men\xc3\xa7\xc3\xb5" "es: " + std::to_string(mencoes.ganhas) + " de " + std::to_string(mencoes.possiveis);
    resumo += "     Horas: " + std::to_string(Horas::De(sessoes[mAluno].progresso.Atividades())) + " de " +
              std::to_string(Horas::kMeta);
    Escrever(mResumoAtor, resumo, Color::White);

    // A posicao no curso, pela media do curso no nivel da partida - so de quem tem ficha.
    std::string posicao = "O CR \xc3\xa9 a m\xc3\xa9" "dia das melhores notas das mat\xc3\xa9rias cursadas.";
    if (!visitante) {
        const int p = Ranking::PosicaoNoGeral(mFichas, materias.Quantas(), mGame->NivelDeDificuldade(), matricula);
        if (p > 0) {
            posicao = "Posi\xc3\xa7\xc3\xa3o no curso: " + std::to_string(p) + "\xc2\xba (" +
                      (mGame->NivelDeDificuldade() == Dificuldade::Nivel::Dificil ? "Dif\xc3\xad" "cil" : "Normal") + ")";
        }
    }
    Escrever(mPosicaoAtor, posicao, Color::LightBlue);

    std::vector<Painel::Acao> itens;
    if (dupla) itens.push_back(Painel::Manche(Painel::Jogador::Um, "trocar de aluno"));
    itens.emplace_back(Painel::Jogador::Um, Painel::Botao::Um, "horas complementares");
    itens.emplace_back(Painel::Jogador::Um, Painel::Botao::Dois, "voltar");
    itens.emplace_back(Painel::Jogador::Um, Painel::Botao::Tres, "ranking");
    Escrever(mRodapeAtor, Painel::Rodape(itens, mGame->JeitoDoRodape()), Color::White);
}

void TelaDoBoletim::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        mGame->RequestSceneChange(SceneType::StageSelect);
        return;
    }

    // As horas complementares do aluno (TelaDasHoras), que volta para ca.
    if (mBordaHoras.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um))) {
        mGame->RequestSceneChange(SceneType::Horas);
        return;
    }

    // O ranking da materia em foco na selecao - ou o do curso, se nenhuma estiver.
    const bool ranking = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Tres);
    if (mBordaRanking.Bateu(ranking)) {
        const int materia = mGame->MateriaEmFoco();
        mGame->PedirRanking(Game::PedidoDeRanking{materia < 0, materia < 0 ? 0 : materia, SceneType::Boletim});
        mGame->RequestSceneChange(SceneType::Ranking);
        return;
    }

    // Na dupla, o manche para os lados troca de aluno.
    const bool esquerda = mBordaEsquerda.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Esquerda));
    const bool direita = mBordaDireita.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Direita));
    const size_t quantos = mGame->Sessoes().size();
    if ((esquerda || direita) && quantos > 1) {
        mAluno = (mAluno + (direita ? 1 : quantos - 1)) % quantos;
        Redesenhar();
    }
}
