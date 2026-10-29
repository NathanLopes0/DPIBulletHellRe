//
// O registro da tentativa: os dados do TCC, anotados enquanto a luta acontece.
//

#include "RegistroDaTentativa.h"

#include <algorithm>

#include "Battle.h"
#include "../../Game.h"
#include "../../Data/MateriasArquivo.h"
#include "../../Student/Matricula.h"
#include "../../Gameplay/Nota.h"
#include "../../Core/Relogio.h"
#include "../../Data/TentativasArquivo.h"
#include "../../Actors/Orientador/Orientador.h"
#include "../../Actors/Player/Player.h"

// A VERSAO vem do CMake (o commit de onde o executavel saiu); compilado de
// outro jeito, "dev".
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
    t.notaMaxima = t.notaMinima = b.GetNotaAtual();
    t.semente = b.SementeDaBatalha();
    if (const Orientador* o = b.GetOrientador()) t.orientador = o->Professor();

    // O JOGADOR i E A SESSAO i: e assim que Battle::LoadPlayer os cria.
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

void RegistroDaTentativa::AoPassarOTempo(Battle&, const float deltaTime, const bool emPausa) {
    if (!mComecou) return;
    (emPausa ? mRelogioDaPausa : mRelogio) += deltaTime;
}

void RegistroDaTentativa::AoFimDoQuadro(Battle& b) {
    if (!Anotando()) return;
    Tentativas::Tentativa& t = mTentativa;
    const float nota = b.GetNotaAtual();
    t.notaMaxima = std::max(t.notaMaxima, nota);
    t.notaMinima = std::min(t.notaMinima, nota);
    if (Nota::ECheia(nota)) t.notaCheia = true;
    const auto& jogadores = b.Jogadores();
    for (size_t i = 0; i < jogadores.size() && i < t.jogadores.size(); ++i) {
        if (jogadores[i]) t.jogadores[i].tiros = jogadores[i]->Disparos();
    }
    if (const Orientador* o = b.GetOrientador()) t.poderesUsados = o->Poderes();
}

void RegistroDaTentativa::AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) {
    if (!Anotando()) return;

    // A FASE QUE ACABA acabou pelo relogio: e so assim que se troca de fase.
    if (!mTentativa.fases.empty()) {
        Tentativas::Fase& anterior = mTentativa.fases.back();
        anterior.fim = mRelogio;
        anterior.notaNoFim = b.GetNotaAtual();   // ja com o bonus de fase limpa, se houve
        anterior.completa = true;
    }

    Tentativas::Fase f;
    f.ordem = static_cast<int>(mTentativa.fases.size()) + 1;
    f.nome = fase;
    f.anuncio = anuncio;
    f.inicio = mRelogio;
    f.notaNoInicio = b.GetNotaAtual();
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
    // O TIRO GUARDA O DONO, e e isso que da a precisao de cada aluno (e os
    // acertos do orientador).
    if (auto* r = De(b, quemAtirou)) ++r->acertosNoProfessor;
    else if (b.GetOrientador() && quemAtirou == b.GetOrientador()) ++mTentativa.acertosDoOrientador;
}

void RegistroDaTentativa::AoPegarPontoExtra(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->pontosExtrasPegos;
}

void RegistroDaTentativa::AoUsarPontoExtra(Battle& b, const Player& aluno) {
    if (!Anotando()) return;
    if (auto* r = De(b, &aluno)) ++r->pontosExtrasUsados;
}

void RegistroDaTentativa::AoTerminar(Battle& b, const Tentativas::Resultado resultado, const bool ultimaFaseCompleta) {
    if (!Anotando()) return;
    mGravada = true;

    // NENHUM ATOR E LIDO AQUI: o fim pode vir do destrutor da batalha.
    Tentativas::Tentativa& t = mTentativa;
    const float nota = b.GetNotaAtual();
    t.fim = Relogio::Agora();
    t.duracao = mRelogio;
    t.tempoEmPausa = mRelogioDaPausa;
    t.resultado = resultado;
    t.notaFinal = nota;
    t.notaMaxima = std::max(t.notaMaxima, nota);
    t.notaMinima = std::min(t.notaMinima, nota);
    if (Nota::ECheia(nota)) t.notaCheia = true;
    t.acertosSofridos = b.AcertosSofridos();
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
