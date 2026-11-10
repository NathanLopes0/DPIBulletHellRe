//
// O registro da tentativa: os dados do TCC, anotados enquanto a luta acontece.
//

#include "RegistroDaTentativa.h"

#include <algorithm>

#include "Battle.h"
#include "../../Motor/Game.h"
#include "../../Curso/MateriasArquivo.h"
#include "../../Aluno/Matricula.h"
#include "../../Regras/Nota.h"
#include "../../Motor/Relogio.h"
#include "../../Pesquisa/TentativasArquivo.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../Actors/Player/Player.h"

// A versao vem do CMake; compilado de outro jeito, "dev".
#ifndef DPI_VERSAO
#define DPI_VERSAO "dev"
#endif

void RegistroDaTentativa::AoComecar(Battle& b) {
    Tentativas::Tentativa& t = mTentativa;
    t.inicio = Relogio::Agora();
    t.id = Tentativas::NovoId(t.inicio, b.SementeDaBatalha());
    t.versaoDoJogo = DPI_VERSAO;
    if (const Materias::Materia* m = Materias::Carregadas().Por(b.MateriaDaBatalha())) {
        t.materia = m->codigo;
        t.nomeDaMateria = m->nomeCompleto.empty() ? m->nome : m->nomeCompleto;
        t.professor = m->professor;
    }
    t.dificuldade = Dificuldade::Chave(b.NivelDeDificuldade());
    t.modo = b.Jogadores().size() > 1 ? "dupla" : "solo";
    t.notaMaxima = t.notaMinima = b.NotaAtual();
    t.semente = b.SementeDaBatalha();
    t.condicao = b.GetGame()->Condicao().Nome();
    t.comPapeis = b.GetGame()->Condicao().Ligada(Experimento::Mecanica::Papeis);
    if (const Orientador* o = b.OrientadorEmCampo()) t.orientador = o->Professor();

    // O jogador i E a sessao i: e assim que Battle::CriarJogadores os cria.
    const auto& sessoes = b.GetGame()->Sessoes();
    for (size_t i = 0; i < b.Jogadores().size(); ++i) {
        Tentativas::Jogador j;
        j.posicao = static_cast<int>(i) + 1;
        j.matricula = i < sessoes.size() ? sessoes[i].matricula : "";
        if (j.matricula.empty()) j.matricula = Matricula::kVisitante;
        j.visitante = j.matricula == Matricula::kVisitante;
        t.jogadores.push_back(j);
    }
    if (t.jogadores.size() == 2) {
        t.jogadores[0].parceiro = t.jogadores[1].matricula;
        t.jogadores[1].parceiro = t.jogadores[0].matricula;
    }
    mComecou = true;
}

void RegistroDaTentativa::AoPassarOTempo(Battle& b, const float deltaTime, const bool emPausa) {
    if (!mComecou) return;
    (emPausa ? mRelogioDaPausa : mRelogio) += deltaTime;
    if (emPausa || !Anotando()) return;

    // Na dupla: em quem a mira esta presa, e quao longe um do outro eles estao.
    const auto& jogadores = b.Jogadores();
    if (jogadores.size() < 2 || mTentativa.jogadores.size() < 2) return;
    const int foco = b.FocoDaMira();
    if (foco >= 0 && static_cast<size_t>(foco) < mTentativa.jogadores.size()) {
        mTentativa.jogadores[static_cast<size_t>(foco)].tempoNaMira += deltaTime;
    }
    if (jogadores[0] && jogadores[1]) {
        mDistanciaVezesTempo += (jogadores[0]->GetPosition() - jogadores[1]->GetPosition()).Length() * deltaTime;
    }
}

void RegistroDaTentativa::AoFimDoQuadro(Battle& b) {
    if (!Anotando()) return;
    Tentativas::Tentativa& t = mTentativa;
    const float nota = b.NotaAtual();
    t.notaMaxima = std::max(t.notaMaxima, nota);
    t.notaMinima = std::min(t.notaMinima, nota);
    if (Nota::ECheia(nota)) t.notaCheia = true;
    const auto& jogadores = b.Jogadores();
    for (size_t i = 0; i < jogadores.size() && i < t.jogadores.size(); ++i) {
        if (jogadores[i]) t.jogadores[i].tiros = jogadores[i]->Disparos();
    }
    if (const Orientador* o = b.OrientadorEmCampo()) t.poderesUsados = o->Poderes();
}

void RegistroDaTentativa::AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) {
    if (!Anotando()) return;

    // A fase que acaba acabou pelo relogio: e so assim que se troca de fase.
    if (!mTentativa.fases.empty()) {
        Tentativas::Fase& anterior = mTentativa.fases.back();
        anterior.fim = mRelogio;
        anterior.notaNoFim = b.NotaAtual();   // ja com o bonus de fase limpa, se houve
        anterior.completa = true;
    }

    Tentativas::Fase f;
    f.ordem = static_cast<int>(mTentativa.fases.size()) + 1;
    f.nome = fase;
    f.anuncio = anuncio;
    f.inicio = mRelogio;
    f.notaNoInicio = b.NotaAtual();
    if (f.nome == "StateFinal") mTentativa.fezExame = true;
    mTentativa.fases.push_back(std::move(f));
}

void RegistroDaTentativa::AoLimparFase(Battle&) {
    if (!Anotando()) return;
    ++mTentativa.fasesLimpas;
    if (!mTentativa.fases.empty()) mTentativa.fases.back().limpa = true;
}

void RegistroDaTentativa::AoSerAcertado(Battle& b, const Player& aluno, bool) {
    if (!Anotando()) return;
    if (!mTentativa.fases.empty()) ++mTentativa.fases.back().acertosSofridos;
    if (auto* r = De(b, &aluno)) ++r->acertosSofridos;
}

void RegistroDaTentativa::AoAcertarOProfessor(Battle& b, const Actor* quemAtirou) {
    if (!Anotando()) return;
    // O tiro guarda o dono, e e isso que da a precisao de cada aluno (e os acertos do orientador).
    if (auto* r = De(b, quemAtirou)) ++r->acertosNoProfessor;
    else if (b.OrientadorEmCampo() && quemAtirou == b.OrientadorEmCampo()) ++mTentativa.acertosDoOrientador;
}

void RegistroDaTentativa::AoAcertarDePerto(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->acertosDePerto;
    if (!mTentativa.fases.empty()) ++mTentativa.fases.back().acertosDePerto;
}

void RegistroDaTentativa::AoRaspar(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->raspoes;
    if (!mTentativa.fases.empty()) ++mTentativa.fases.back().raspoes;
}

void RegistroDaTentativa::AoCobrir(Battle& b, const Player& quemCobriu, const Player&) {
    if (!Anotando()) return;
    if (auto* r = De(b, &quemCobriu)) ++r->coberturas;
    if (!mTentativa.fases.empty()) ++mTentativa.fases.back().coberturas;
}

void RegistroDaTentativa::AoIrNoAluno(Battle& b, const Player& alvo) {
    if (!Anotando()) return;
    // A chance e do outro: e ele quem pode cobrir.
    const auto& jogadores = b.Jogadores();
    for (size_t i = 0; i < jogadores.size() && i < mTentativa.jogadores.size(); ++i) {
        if (jogadores[i] && jogadores[i] != &alvo) ++mTentativa.jogadores[i].oportunidadesDeCobertura;
    }
}

void RegistroDaTentativa::AoPegarPontoExtra(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->pontosExtrasPegos;
}

void RegistroDaTentativa::AoUsarPontoExtra(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->pontosExtrasUsados;
}

void RegistroDaTentativa::AoSincronizar(Battle&) {
    if (!Anotando()) return;
    ++mTentativa.sincronias;
}

void RegistroDaTentativa::AoSinalizar(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->sinais;
}

void RegistroDaTentativa::AoTocarAqui(Battle& b) {
    // Vem no resultado, com a tentativa ja gravada: e um arquivo a parte, ligado a ela pelo id.
    if (!mGravada) return;
    ++mToques;
    TentativasArquivo::GravarToque(mTentativa, mToques, b.TempoNoResultado());
}

void RegistroDaTentativa::AoTerminar(Battle& b, const Tentativas::Resultado resultado, const bool ultimaFaseCompleta) {
    if (!Anotando()) return;
    mGravada = true;

    // Nenhum ator e lido aqui: o fim pode vir do destrutor da batalha.
    Tentativas::Tentativa& t = mTentativa;
    const float nota = b.NotaAtual();
    t.fim = Relogio::Agora();
    t.duracao = mRelogio;
    t.tempoEmPausa = mRelogioDaPausa;
    t.resultado = resultado;
    t.notaFinal = nota;
    t.notaMaxima = std::max(t.notaMaxima, nota);
    t.notaMinima = std::min(t.notaMinima, nota);
    if (Nota::ECheia(nota)) t.notaCheia = true;
    t.acertosSofridos = b.AcertosSofridos();
    t.distanciaMedia = mRelogio > 0.0f ? mDistanciaVezesTempo / mRelogio : 0.0f;
    if (!t.fases.empty()) {
        Tentativas::Fase& ultima = t.fases.back();
        ultima.fim = mRelogio;
        ultima.notaNoFim = nota;
        ultima.completa = ultimaFaseCompleta;
    }
    TentativasArquivo::Gravar(t);
}

Tentativas::Jogador* RegistroDaTentativa::De(const Battle& b, const Actor* quem) {
    if (!quem) return nullptr;
    const auto& jogadores = b.Jogadores();
    for (size_t i = 0; i < jogadores.size() && i < mTentativa.jogadores.size(); ++i) {
        if (jogadores[i] == quem) return &mTentativa.jogadores[i];
    }
    return nullptr;
}
