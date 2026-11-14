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
#include "../Components/DrawComponents/DrawLinhasComponent.h"
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
    /// As colunas, em fracao da largura: o periodo numa coluna estreita (so na primeira materia
    /// dele), a materia, e o resto em colunas estreitas.
    constexpr float kColunaPeriodo    = 0.03f;
    constexpr float kColunaMateria    = 0.12f;
    constexpr float kColunaProfessor  = 0.42f;
    constexpr float kColunaNota       = 0.515f;
    constexpr float kColunaSituacao   = 0.59f;
    constexpr float kColunaTentativas = 0.685f;
    constexpr float kColunaMencoes    = 0.73f;

    constexpr float kTopoDaTabela = 0.215f;
    constexpr float kPassoDaTabela = 0.034f;
    constexpr int kLetraDaTabela = 18;
    /// A largura da coluna da materia: o nome que passa dela e encurtado ate caber.
    constexpr int kLarguraDoNome = 362;
    const SDL_Color kCorDoFio{96, 118, 170, 110};

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

    // Quem voltou da troca de personagem cai no mesmo aluno.
    mAluno = mGame->AlunoDoBoletim();

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
    titulo("Per\xc3\xadodo", kColunaPeriodo, 100);
    titulo("Mat\xc3\xa9ria", kColunaMateria, 340);
    titulo("Professor", kColunaProfessor, 110);
    titulo("Nota", kColunaNota, 90);
    titulo("Situa\xc3\xa7\xc3\xa3o", kColunaSituacao, 110);
    titulo("Tent.", kColunaTentativas - 0.01f, 60);
    titulo("Men\xc3\xa7\xc3\xb5" "es", kColunaMencoes, 200);

    for (size_t i = 0; i < kLinhasDaTabela; ++i) {
        const float y = altura * (kTopoDaTabela + kPassoDaTabela * static_cast<float>(i));
        LinhaDeTela l;
        l.periodo    = Texto(" ", largura * kColunaPeriodo, y, 16, 100, true);
        l.materia    = Texto(" ", largura * kColunaMateria, y, kLetraDaTabela, kLarguraDoNome, true);
        l.professor  = Texto(" ", largura * kColunaProfessor, y, kLetraDaTabela, 110, true);
        l.nota       = Texto(" ", largura * kColunaNota, y, kLetraDaTabela, 90, true);
        l.situacao   = Texto(" ", largura * kColunaSituacao, y, kLetraDaTabela, 110, true);
        l.tentativas = Texto(" ", largura * kColunaTentativas, y, kLetraDaTabela, 50, true);
        l.mencoes    = std::make_unique<SelosDeMencao>(*this, Vector2(largura * kColunaMencoes, y), 200, true);
        mLinhas.push_back(std::move(l));
    }

    // Os fios entre os periodos (redesenhados com a tabela).
    auto fios = std::make_unique<Actor>(this);
    mFios = fios->AddComponent<DrawLinhasComponent>(60);
    AddActor(std::move(fios));

    // A legenda dos selos, embaixo da tabela, em duas linhas: numa so, ela encostava no resumo. O
    // corte e no separador mais perto do meio.
    const std::string legenda = Mencoes::Legenda();
    const std::string ponto = "  \xc2\xb7  ";
    size_t corte = std::string::npos;
    for (size_t p = legenda.find(ponto); p != std::string::npos; p = legenda.find(ponto, p + 1)) {
        if (corte == std::string::npos || (p > legenda.size() / 2 ? p - legenda.size() / 2 : legenda.size() / 2 - p) <
                                              (corte > legenda.size() / 2 ? corte - legenda.size() / 2 : legenda.size() / 2 - corte)) {
            corte = p;
        }
    }
    const std::string primeira = corte == std::string::npos ? legenda : legenda.substr(0, corte);
    const std::string segunda = corte == std::string::npos ? std::string(" ") : legenda.substr(corte + ponto.size());
    Escrever(Texto(" ", largura / 2.0f, altura * 0.785f, 16, 1100), primeira, kTituloDaColuna);
    Escrever(Texto(" ", largura / 2.0f, altura * 0.815f, 16, 1100), segunda, kTituloDaColuna);

    mResumoAtor  = Texto(" ", largura / 2.0f, altura * 0.865f, 24, 1100);
    mPosicaoAtor = Texto(" ", largura / 2.0f, altura * 0.91f, 18, 1100);
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
    mGame->LembrarAlunoDoBoletim(mAluno);
    const bool dupla = sessoes.size() > 1;
    const std::string& matricula = mGame->MatriculaDe(mAluno);
    const bool visitante = matricula.empty() || matricula == Matricula::kVisitante;

    // Quem: a matricula, ou o aviso de que o visitante nao salva; na dupla, na cor do aluno.
    std::string quem = visitante ? std::string("Visitante - n\xc3\xa3o salva") : "Matr\xc3\xad" "cula " + matricula;
    if (dupla) quem = "Aluno " + std::to_string(mAluno + 1) + ": " + quem;
    const Hud::Rgb c = Hud::CorDoAluno(mAluno);
    Escrever(mAlunoAtor, quem, dupla ? Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f) : Color::LightBlue);

    // A tabela: uma linha por materia; o periodo so na primeira dele, com um fio em cima.
    const Boletim::Dados d = Boletim::Montar(sessoes[mAluno].progresso, materias);
    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    if (mFios) mFios->Limpar();
    size_t i = 0;
    std::string secao;
    const auto vazia = [&](const LinhaDeTela& l) {
        for (Actor* a : {l.periodo, l.materia, l.professor, l.nota, l.situacao, l.tentativas}) Escrever(a, " ", Color::White);
        l.mencoes->Esconder();
    };
    for (const Boletim::Linha& l : d.linhas) {
        if (i >= mLinhas.size()) break;
        const std::string essa = Boletim::Secao(l);
        if (essa != secao) {
            secao = essa;
            Escrever(mLinhas[i].periodo, secao, l.optativa ? Color::LightBlue : Color::Gold);
            if (i > 0 && mFios) {
                const float y = altura * (kTopoDaTabela + kPassoDaTabela * (static_cast<float>(i) - 0.5f));
                mFios->Acrescentar(Vector2(largura * kColunaPeriodo, y), Vector2(largura * 0.97f, y), kCorDoFio);
            }
        } else {
            Escrever(mLinhas[i].periodo, " ", Color::White);
        }
        const Vector3 cor = CorDa(l);
        const std::string nome = l.nome.empty() ? l.codigo : l.codigo + " - " + l.nome;
        // Encurta letra a letra ate caber numa linha so (a medida e a da fonte de verdade).
        std::string cabe = nome;
        for (size_t letras = nome.size(); letras > 4 && mFonte->LarguraDe(cabe, kLetraDaTabela) > kLarguraDoNome - 6; --letras) {
            cabe = Boletim::Encurtar(nome, letras);
        }
        Escrever(mLinhas[i].materia, cabe,
                 l.situacao == Boletim::Situacao::Fechada ? kApagado : Color::White);
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
    const auto& chaves = sessoes[mAluno].progresso.Atividades();
    resumo += "     Horas: " + std::to_string(Horas::De(chaves)) + " de " + std::to_string(Horas::kMeta);
    // Quem ja gastou horas com optativas ve o saldo; quem nao gastou, o saldo e o total.
    if (Horas::Gastas(chaves, materias) > 0) resumo += " (saldo " + std::to_string(Horas::Saldo(chaves, materias)) + ")";
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
    itens.emplace_back("MANCHE ACIMA", "CIMA", "personagem");
    itens.emplace_back(Painel::Jogador::Um, Painel::Botao::Um, "horas");
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

    // A personagem do aluno em tela, quando ele quiser: a criacao de personagem volta para ca.
    if (mBordaPersonagem.Bateu(Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Cima))) {
        mGame->EditarPersonagem(mAluno, SceneType::Boletim);
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
