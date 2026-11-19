//
// O professor generico da fase tutorial (INF 191 - Introducao a Ciencia da Computacao).
//

#include "Generico.h"

#include <algorithm>
#include <cmath>

#include "PainelDeCodigo.h"
#include "../../Player/Player.h"
#include "../../../Attacks/AttackParameters/AttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Entrada/Painel.h"
#include "../../../Motor/Game.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/FaixaDaNota.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../../Scenes/Scene.h"

namespace {
    const SDL_Color kBranco{240, 244, 255, 255};
    const SDL_Color kApagado{150, 156, 176, 200};
    const SDL_Color kEscuro{18, 20, 30, 235};
    const SDL_Color kVerde{110, 230, 120, 255};
    const SDL_Color kVermelho{255, 90, 90, 255};
    const SDL_Color kAmarelo{255, 214, 90, 255};
    const SDL_Color kVermelhoDoManche{230, 70, 70, 255};

    /// Onde os desenhos ficam em relacao ao aluno: acima dele, longe da chuva que vem de cima.
    const Vector2 kAcimaDoAluno(0.0f, -100.0f);
    constexpr float kRaioDoBotao = 22.0f;
    /// O manche desenhado: o raio da base, a altura da haste e o raio da bola.
    constexpr float kBaseDoManche = 19.0f;
    constexpr float kHasteDoManche = 32.0f;
    constexpr float kBolaDoManche = 10.0f;
    /// Onde a tecla do computador fica, debaixo do desenho; e o vao entre dois desenhos lado a lado.
    constexpr float kTeclaAbaixo = 52.0f;
    constexpr float kLadoALado = 48.0f;
    constexpr float kSetaNaBarraDura = 0.6f;
}

Generico::Generico(Scene* scene) : Boss(scene) {}

float Generico::Tempo() const {
    const auto* fsm = GetComponent<FSMComponent>();
    return fsm ? fsm->GetStateTime() : 0.0f;
}

void Generico::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;

    // O placar das etapas fica apagado: o tutorial nao tem uma palavra.
    if (!mEtapasSemTexto) {
        mEtapasSemTexto = true;
        auto etapas = EtapasDa(kFase);
        for (auto& e : etapas) e.second.clear();
        RegistrarEtapas(kFase, etapas);
    }
    if (!mLinhas && mScene) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();
    EsconderTeclas();

    Battle* b = Batalha();
    if (!b || !b->EmCombate()) return;
    const float t = Tempo();
    const Tutorial::Etapa etapa = Tutorial::EtapaEm(t);
    const float te = Tutorial::NaEtapa(t);
    const bool noComputador = !b->GetGame()->NoGabinete();

    // O Ponto Extra da etapa dele: um para cada aluno, acima dele, uma vez.
    if (etapa == Tutorial::Etapa::PontoExtra && mOndeDoPontoExtra.empty()) {
        const SDL_FRect c = Campo();
        for (const Player* a : b->Jogadores()) {
            if (!a) continue;
            const Vector2 onde(std::clamp(a->GetPosition().x, c.x + 60.0f, c.x + c.w - 60.0f),
                               std::max(c.y + 80.0f, a->GetPosition().y - Tutorial::kPontoExtraAcima));
            mOndeDoPontoExtra.push_back(onde);
            b->SoltarPontoExtra(onde);
        }
    }

    // Ninguem reprova aqui.
    if (b->NotaAtual() < Tutorial::kNotaMinima) {
        b->DefinirNota(Tutorial::kNotaMinima);
        mNotaAntes = Tutorial::kNotaMinima;
    }
    if (etapa != Tutorial::Etapa::Chegada) AcompanharANota(deltaTime);

    size_t tecla = 0;
    const std::vector<Player*>& alunos = b->Jogadores();
    for (size_t i = 0; i < alunos.size(); ++i) {
        const Player* a = alunos[i];
        if (!a) continue;
        const Painel::Jogador quem = Painel::kJogadores[std::min<size_t>(i, 1)];
        const Vector2 sobre = a->GetPosition() + kAcimaDoAluno;
        switch (etapa) {
            case Tutorial::Etapa::Desviar:
                Manche(sobre, Tutorial::LadoDoManche(te));
                if (noComputador) EscreverTecla(tecla, Painel::NomeDoManche(quem), sobre + Vector2(0.0f, kTeclaAbaixo));
                break;
            case Tutorial::Etapa::Atirar: {
                Botao(sobre, '1', Tutorial::BotaoApertado(te));
                if (noComputador) EscreverTecla(tecla, Painel::NomeDaTecla(quem, Painel::Botao::Um), sobre + Vector2(0.0f, kTeclaAbaixo));
                // A linha do aluno ao professor, tracejada e andando para cima: e para la que se atira.
                const Vector2 de = sobre + Vector2(0.0f, -kRaioDoBotao - 6.0f);
                const Vector2 ate = GetPosition() + Vector2(0.0f, 60.0f);
                const Vector2 d = ate - de;
                const float comprimento = d.Length();
                if (comprimento > 40.0f) {
                    const Vector2 u = d * (1.0f / comprimento);
                    constexpr float kTraco = 14.0f, kPasso = 30.0f;
                    const float desloca = std::fmod(te * 90.0f, kPasso);
                    for (float s = desloca; s + kTraco < comprimento - 20.0f; s += kPasso) {
                        Linha(de + u * s, de + u * (s + kTraco), kAmarelo);
                    }
                    Seta(ate - u * 40.0f, ate - u * 8.0f, kAmarelo);
                }
                break;
            }
            case Tutorial::Etapa::SobeEDesce:
                // Os dois juntos: desviar e atirar.
                Manche(sobre + Vector2(-kLadoALado, 0.0f), Tutorial::LadoDoManche(te));
                Botao(sobre + Vector2(kLadoALado, 0.0f), '1', Tutorial::BotaoApertado(te));
                if (noComputador) {
                    EscreverTecla(tecla, Painel::NomeDoManche(quem), sobre + Vector2(-kLadoALado, kTeclaAbaixo));
                    EscreverTecla(tecla, Painel::NomeDaTecla(quem, Painel::Botao::Um), sobre + Vector2(kLadoALado, kTeclaAbaixo));
                }
                break;
            case Tutorial::Etapa::Foco: {
                // O Botao 4 segurado: sempre aceso, com o anel de quem segura.
                Botao(sobre, '4', true);
                Circulo(sobre, kRaioDoBotao + 7.0f + 3.0f * std::sin(te * 6.0f), kAmarelo, 28);
                Circulo(sobre, kRaioDoBotao + 8.0f + 3.0f * std::sin(te * 6.0f), kAmarelo, 28);
                if (noComputador) EscreverTecla(tecla, Painel::NomeDaTeclaDoFoco(quem), sobre + Vector2(0.0f, kTeclaAbaixo));
                break;
            }
            case Tutorial::Etapa::PontoExtra: {
                if (a->PontosExtras() > 0) {
                    // Com ele na mao: o Botao 2, que limpa a chuva.
                    Botao(sobre, '2', Tutorial::BotaoApertado(te));
                    if (noComputador) EscreverTecla(tecla, Painel::NomeDaTecla(quem, Painel::Botao::Dois), sobre + Vector2(0.0f, kTeclaAbaixo));
                } else if (i < mOndeDoPontoExtra.size()) {
                    // Sem ele: a seta tracejada do aluno ate o Ponto Extra, que espera no campo.
                    const Vector2 de = a->GetPosition() + Vector2(0.0f, -26.0f);
                    const Vector2 ate = mOndeDoPontoExtra[i];
                    const Vector2 d = ate - de;
                    const float comprimento = d.Length();
                    if (comprimento > 50.0f) {
                        const Vector2 u = d * (1.0f / comprimento);
                        constexpr float kTraco = 12.0f, kPasso = 26.0f;
                        const float desloca = std::fmod(te * 80.0f, kPasso);
                        for (float s = desloca; s + kTraco < comprimento - 30.0f; s += kPasso) {
                            Linha(de + u * s, de + u * (s + kTraco), kVerde);
                        }
                        Seta(ate - u * 52.0f, ate - u * 24.0f, kVerde);
                    }
                    Circulo(ate, 22.0f + 3.0f * std::sin(te * 6.0f), kVerde, 26);
                }
                break;
            }
            case Tutorial::Etapa::Chegada:
            case Tutorial::Etapa::Fim:
                break;
        }
    }

    // A nota que sobe e desce: cada sinal voa ate a ponta da barra, e a seta fica ao lado dela.
    if (const FaixaDaNota* faixa = b->Faixa()) {
        const Vector2 ponta = faixa->Ponta(b->NotaAtual());
        for (const Voo& v : mVoos) Sinal(Tutorial::NoVoo(v.de, ponta, v.idade), v.mais, 10.0f);
        if (mSetaNaBarra > 0.0f) {
            const Vector2 base = ponta + Vector2(0.0f, -34.0f);
            if (mUltimoMais) Seta(base + Vector2(0.0f, 14.0f), base + Vector2(0.0f, -14.0f), kVerde, 10.0f);
            else Seta(base + Vector2(0.0f, -14.0f), base + Vector2(0.0f, 14.0f), kVermelho, 10.0f);
        }
    }

    // O fim: o visto verde no meio do campo, e a prova se encerra sozinha.
    if (etapa == Tutorial::Etapa::Fim) {
        // O que ainda voava some: a festa e so festa.
        if (!mLimpouNoFim) {
            mLimpouNoFim = true;
            if (ProjectileManager* tiros = b->GerenteDeProjeteis()) tiros->ClearBossProjectiles();
        }
        const SDL_FRect c = Campo();
        const float u = std::clamp(te / 0.5f, 0.0f, 1.0f);
        Visto(Vector2(c.x + c.w / 2.0f, c.y + c.h / 2.0f), 30.0f + 50.0f * u);
        for (int k = 0; k < 3; ++k) Circulo(GetPosition(), 30.0f + std::fmod(te * 120.0f + k * 40.0f, 120.0f), kAmarelo, 28);
    }
    if (Tutorial::Acabou(t) && !mEncerrou) {
        mEncerrou = true;
        b->EncerrarBatalha(true);
    }
}

void Generico::AcompanharANota(const float deltaTime) {
    Battle* b = Batalha();
    if (!b) return;
    const float agora = b->NotaAtual();
    if (mNotaAntes < 0.0f) mNotaAntes = agora;
    const Tutorial::Sinal s = Tutorial::SinalDaNota(mNotaAntes, agora);
    mNotaAntes = agora;
    if (s != Tutorial::Sinal::Nada) {
        const bool mais = s == Tutorial::Sinal::Subiu;
        Vector2 de = GetPosition();
        if (!mais) {
            // O "-" sai de quem levou o tiro.
            for (const Player* a : b->Jogadores()) {
                if (a && a->AcabouDeApanhar()) { de = a->GetPosition(); break; }
            }
        }
        mVoos.push_back(Voo{de, mais, 0.0f});
        mUltimoMais = mais;
        mSetaNaBarra = kSetaNaBarraDura;
    }
    for (Voo& v : mVoos) v.idade += deltaTime;
    mVoos.erase(std::remove_if(mVoos.begin(), mVoos.end(), [](const Voo& v) { return v.idade >= Tutorial::kVooDoSinal; }),
                mVoos.end());
    mSetaNaBarra = std::max(0.0f, mSetaNaBarra - deltaTime);
}

void Generico::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (Tutorial::EtapaEm(Tempo()) != Tutorial::Etapa::SobeEDesce) return;
    const Vector2 d = GetDirectionToPlayer();
    if (d.LengthSq() > 1e-6f) params.centralAngle = Math::ToDegrees(std::atan2(d.y, d.x));
}

void Generico::AoTerminarABatalha() {
    if (mLinhas) mLinhas->Limpar();
    EsconderTeclas();
    mVoos.clear();
}

// ---- O desenho ----

void Generico::Linha(const Vector2 de, const Vector2 ate, const SDL_Color cor) {
    if (mLinhas) mLinhas->Acrescentar(de, ate, cor);
}

void Generico::Circulo(const Vector2 centro, const float raio, const SDL_Color cor, const int lados) {
    for (int k = 0; k < lados; ++k) {
        const float a0 = Math::TwoPi * static_cast<float>(k) / static_cast<float>(lados);
        const float a1 = Math::TwoPi * static_cast<float>(k + 1) / static_cast<float>(lados);
        Linha(centro + Vector2(Math::Cos(a0), Math::Sin(a0)) * raio, centro + Vector2(Math::Cos(a1), Math::Sin(a1)) * raio, cor);
    }
}

void Generico::Disco(const Vector2 centro, const float raio, const SDL_Color cor) {
    // Linhas horizontais, uma por pixel: o disco cheio.
    for (float dy = -raio; dy <= raio; dy += 1.0f) {
        const float dx = std::sqrt(std::max(0.0f, raio * raio - dy * dy));
        Linha(centro + Vector2(-dx, dy), centro + Vector2(dx, dy), cor);
    }
}

void Generico::Seta(const Vector2 de, const Vector2 ate, const SDL_Color cor, const float ponta) {
    const Vector2 d = ate - de;
    const float comprimento = d.Length();
    if (comprimento < 1.0f) return;
    const Vector2 u = d * (1.0f / comprimento);
    const Vector2 n(-u.y, u.x);
    for (float s = -1.0f; s <= 1.0f; s += 1.0f) Linha(de + n * s, ate + n * s, cor);
    Linha(ate, ate - u * ponta + n * (ponta * 0.7f), cor);
    Linha(ate, ate - u * ponta - n * (ponta * 0.7f), cor);
    Linha(ate - n, ate - u * ponta + n * (ponta * 0.7f) - n, cor);
    Linha(ate + n, ate - u * ponta - n * (ponta * 0.7f) + n, cor);
}

void Generico::Algarismo(const char d, const Vector2 centro, const float altura, const SDL_Color cor) {
    const float h = altura / 2.0f, w = altura * 0.3f;
    const Vector2 a(centro.x - w, centro.y - h), b(centro.x + w, centro.y - h);
    const Vector2 c(centro.x - w, centro.y), e(centro.x + w, centro.y);
    const Vector2 f(centro.x - w, centro.y + h), g(centro.x + w, centro.y + h);
    const auto grosso = [&](const Vector2 p, const Vector2 q) {
        for (float s = -1.0f; s <= 1.0f; s += 1.0f) {
            const Vector2 off = std::fabs(p.x - q.x) < 0.5f ? Vector2(s, 0.0f) : Vector2(0.0f, s);
            Linha(p + off, q + off, cor);
        }
    };
    if (d == '1') {
        grosso(b, g);
        Linha(b, b + Vector2(-w, w * 0.8f), cor);
    } else if (d == '2') {
        grosso(a, b);
        grosso(b, e);
        grosso(e, c);
        grosso(c, f);
        grosso(f, g);
    } else if (d == '4') {
        grosso(a, c);
        grosso(c, e);
        grosso(b, g);
    }
}

void Generico::Manche(const Vector2 centro, const Vector2 lado) {
    // A base do manche, a haste inclinada para o lado da vez e a bola vermelha em cima.
    const Vector2 base = centro + Vector2(0.0f, 20.0f);
    Disco(base, kBaseDoManche, kEscuro);
    Circulo(base, kBaseDoManche, kApagado, 26);
    const Vector2 topo = base + Vector2(lado.x * 16.0f, -kHasteDoManche + lado.y * 9.0f);
    for (float s = -2.5f; s <= 2.5f; s += 1.0f) Linha(base + Vector2(s, 0.0f), topo + Vector2(s, 0.0f), kBranco);
    Disco(topo, kBolaDoManche, kVermelhoDoManche);
    // A seta do lado da vez, saindo da bola.
    Seta(topo + lado * (kBolaDoManche + 6.0f), topo + lado * (kBolaDoManche + 28.0f), kBranco, 9.0f);
}

void Generico::Botao(const Vector2 centro, const char numero, const bool apertado) {
    // Apertado, o botao afunda e acende.
    const Vector2 c = centro + Vector2(0.0f, apertado ? 2.0f : 0.0f);
    if (!apertado) Disco(centro + Vector2(0.0f, 3.0f), kRaioDoBotao, kEscuro);
    Disco(c, kRaioDoBotao, apertado ? kAmarelo : kApagado);
    Circulo(c, kRaioDoBotao, kBranco, 24);
    Algarismo(numero, c, 24.0f, kEscuro);
}

void Generico::Sinal(const Vector2 centro, const bool mais, const float tamanho) {
    const SDL_Color cor = mais ? kVerde : kVermelho;
    for (float s = -2.0f; s <= 2.0f; s += 1.0f) {
        Linha(centro + Vector2(-tamanho, s), centro + Vector2(tamanho, s), cor);
        if (mais) Linha(centro + Vector2(s, -tamanho), centro + Vector2(s, tamanho), cor);
    }
}

void Generico::Visto(const Vector2 centro, const float tamanho) {
    const Vector2 a = centro + Vector2(-tamanho, 0.0f);
    const Vector2 b = centro + Vector2(-tamanho * 0.3f, tamanho * 0.65f);
    const Vector2 c = centro + Vector2(tamanho, -tamanho * 0.7f);
    for (float s = -4.0f; s <= 4.0f; s += 1.0f) {
        Linha(a + Vector2(0.0f, s), b + Vector2(0.0f, s), kVerde);
        Linha(b + Vector2(0.0f, s), c + Vector2(0.0f, s), kVerde);
    }
}

// ---- As teclas ----

EtiquetaDeCodigo* Generico::Tecla(const size_t k) {
    if (!mScene) return nullptr;
    while (mTeclas.size() <= k) {
        auto nova = std::make_unique<EtiquetaDeCodigo>(mScene, " ", 90, 24, kOrdemDasLinhas + 1);
        nova->Pintar(SDL_Color{235, 240, 255, 120}, SDL_Color{10, 12, 24, 215}, SDL_Color{240, 244, 255, 255});
        nova->Mostrar(false);
        mTeclas.push_back(nova.get());
        mScene->AddActor(std::move(nova));
    }
    return mTeclas[k];
}

void Generico::EsconderTeclas() {
    for (EtiquetaDeCodigo* e : mTeclas) if (e) e->Mostrar(false);
}

void Generico::EscreverTecla(size_t& k, const std::string& texto, const Vector2 onde) {
    EtiquetaDeCodigo* e = Tecla(k++);
    if (!e) return;
    e->Escrever(texto);
    e->SetPosition(onde);
    e->Mostrar(true);
}
