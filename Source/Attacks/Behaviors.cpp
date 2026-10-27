//
// Created by gensh on 17/10/2025.
//


#include "Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Scenes/Battle/Battle.h"
#include "../Actors/Teachers/Boss.h"
#include "Flex.h"
#include "Promessa.h"
#include "Amarra.h"
#include "../Random.h"

void AccelerateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!accelerated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= accelerateDelay) {
                accelerated = true;
                auto currVel = bossProj->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(accelerateSpeedValue != -1) {
                    currVel *= static_cast<float>(accelerateSpeedValue);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux + currVel);
                }
                else {
                    accelerateSpeedPercent = Math::Max(accelerateSpeedPercent, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * accelerateSpeedPercent);
                }
            }
        }
    }
}
void SlowDownBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!slowedDown) {
            elapsedTime += deltaTime;
            if(elapsedTime >= slowdownDelay) {
                slowedDown = true;
                auto currVel = p->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(slowdownSpeedValue != -1) {
                    currVel *= static_cast<float>(slowdownSpeedValue);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux - currVel);
                }
                else {
                    Math::Clamp(slowdownSpeedPercent, 0.f, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * slowdownSpeedPercent);
                }
            }
        }
    }
}
void ActivateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!activated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= activationDelay) {
                p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(activationVelocity);
                activated = true;
            }
        }
    }
}

void DeactivateBehavior::update(Projectile *p, const float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if (!deactivated) {
            elapsedTime += deltaTime;
            if (elapsedTime >= deactivationDelay) {
                p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
                deactivated = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// TrackingBehavior
// ---------------------------------------------------------------------------
void TrackingBehavior::update(Projectile* p, float deltaTime) {

    // Precisa saber onde o jogador esta, e so BossProjectile sabe.
    auto bossProj = dynamic_cast<BossProjectile*>(p);
    if (!bossProj) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    auto rb = bossProj->GetComponent<RigidBodyComponent>();
    if (!rb) {
        finished = true;
        return;
    }

    // Decaimento linear: a forca cai de trackingStrength ate 0 ao longo de
    // trackingTime. Quando zera, o projetil segue reto e o behavior sai da
    // lista (isFinished).
    const float t = elapsedTime - startDelay;
    if (trackingTime <= 0.0f || t >= trackingTime) {
        finished = true;
        return;
    }
    const float forca = trackingStrength * (1.0f - t / trackingTime);

    const Vector2 velocity = rb->GetVelocity();
    const float speed = velocity.Length();

    // Sem velocidade nao ha direcao a corrigir. Sair daqui tambem evita
    // normalizar um vetor nulo.
    if (Math::NearZero(speed)) return;

    // Passo de correcao neste frame, limitado a 1.0 para nunca ultrapassar a
    // direcao desejada (o que faria o projetil oscilar em vez de convergir).
    float step = forca * deltaTime;
    if (step > 1.0f) step = 1.0f;

    const Vector2 desired = bossProj->GetPlayerDirection() * speed;
    Vector2 corrected = velocity + (desired - velocity) * step;

    // Se a correcao anulou o vetor (caso raro: velocidade exatamente oposta a
    // desejada com step 0.5), manter a velocidade anterior. Sem esta guarda o
    // projetil pararia, nunca sairia da tela e nunca voltaria para o pool.
    if (Math::NearZero(corrected.Length())) return;

    corrected.Normalize();
    rb->SetVelocity(corrected * speed);
}

// ---------------------------------------------------------------------------
// WobbleBehavior
// ---------------------------------------------------------------------------
void WobbleBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    // Captura a direcao base UMA unica vez. Reler a velocidade todo frame faria
    // a oscilacao realimentar a si mesma e o projetil sairia em espiral.
    if (!started) {
        const Vector2 velocity = rb->GetVelocity();
        baseSpeed = velocity.Length();

        if (Math::NearZero(baseSpeed)) {
            // Projetil parado na ativacao: nao ha eixo para oscilar em torno.
            finished = true;
            return;
        }

        baseDirection = velocity;
        baseDirection.Normalize();
        started = true;
    }

    const float t = elapsedTime - startDelay;
    if (duration <= 0.0f || t >= duration) {
        // Termina alinhado com a direcao base, sem desvio residual.
        rb->SetVelocity(DirecionarPreservandoModulo(rb->GetVelocity(), baseDirection, baseSpeed));
        finished = true;
        return;
    }

    // Amplitude decai linearmente; a fase avanca com a frequencia dada.
    const float decay = 1.0f - t / duration;
    const float angleDeg = maxAngle * decay * Math::Sin(2.0f * Math::Pi * frequency * t);
    const float angleRad = Math::ToRadians(angleDeg);

    // Rotaciona a direcao base pelo desvio atual.
    const float c = Math::Cos(angleRad);
    const float s = Math::Sin(angleRad);
    const Vector2 rotated(baseDirection.x * c - baseDirection.y * s,
                          baseDirection.x * s + baseDirection.y * c);

    // Mesmo contrato do PathBehavior: a oscilacao decide a DIRECAO e o modulo
    // vem da velocidade atual. Usar baseSpeed aqui anulava qualquer Modifier.
    rb->SetVelocity(DirecionarPreservandoModulo(rb->GetVelocity(), rotated, baseSpeed));
}

// ---------------------------------------------------------------------------
// PathBehavior
// ---------------------------------------------------------------------------
void PathBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb || !waypoints || waypoints->empty()) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    // Captura posicao e orientacao UMA vez, na ativacao. Tudo depois disso e
    // calculado relativo a este instante.
    if (!started) {

        // --- PONTE COM O MOTOR (Humble Object) ---
        // Esta e a unica parte que conhece Projectile e componentes. Ela apenas
        // le o mundo e monta o retrato; toda a decisao mora em
        // ResolverReferencial, que e puro e coberto por testes unitarios.
        ContextoDeResolucao contexto;
        contexto.posicaoProjetil    = p->GetPosition();
        contexto.velocidadeProjetil = rb->GetVelocity();

        if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
            contexto.temJogador        = bossProj->HasPlayer();
            contexto.posicaoJogador    = bossProj->GetPlayerPosition();
            contexto.velocidadeJogador = bossProj->GetPlayerVelocity();
        }

        const Referencial referencial = ResolverReferencial(contexto, mira);
        origin = referencial.origem;
        cosR   = referencial.cos;
        sinR   = referencial.sin;

        // Velocidade do percurso: 0 significa "mantenha a que o projetil tem".
        // O piso de 100 evita projetil parado, que nunca sairia da tela e nunca
        // voltaria ao pool.
        if (pathSpeed <= 0.0f) {
            const float velocidadeAtual = contexto.velocidadeProjetil.Length();
            pathSpeed = Math::NearZero(velocidadeAtual) ? 100.0f : velocidadeAtual;
        }

        // Aplica o modulo inicial UMA vez, ja na direcao do referencial. Depois
        // disto o modulo so muda por Modifier - ver o passo abaixo.
        rb->SetVelocity(Vector2(cosR, sinR) * pathSpeed);

        started = true;
    }

    // PARADA EM CURSO: o projetil fica onde esta, visivel e com colisor ligado.
    // Nao avanca waypoint e nao redireciona - sair daqui e so deixar o contador
    // chegar a zero, e no quadro seguinte o teste de chegada roda de novo, agora
    // com paradaFeita marcada.
    if (paradaRestante > 0.0f) {
        paradaRestante -= deltaTime;
        rb->SetVelocity(Vector2::Zero);
        return;
    }

    // Waypoint atual, convertido de espaco de caminho para espaco de mundo.
    const Vector2& wp = (*waypoints)[current];
    const Vector2 target(origin.x + wp.x * cosR - wp.y * sinR,
                         origin.y + wp.x * sinR + wp.y * cosR);

    Vector2 toTarget = target - p->GetPosition();
    const float distance = toTarget.Length();

    // Tolerancia proporcional ao passo do frame. Com um valor fixo pequeno, um
    // projetil rapido passa por cima do waypoint sem nunca entrar no raio de
    // chegada e fica orbitando em torno dele indefinidamente.
    // Tolerancia pelo modulo ATUAL, nao pelo pathSpeed: se um Modifier
    // desacelerou o projetil, o passo por frame encolheu junto.
    float tolerance = rb->GetVelocity().Length() * deltaTime * 1.5f;
    if (tolerance < 2.0f) tolerance = 2.0f;

    if (distance <= tolerance) {

        // Chegou no ponto de parada: segura AQUI, sem avancar o waypoint. O
        // avanco acontece quando este mesmo teste rodar de novo com paradaFeita
        // marcada, depois que o contador zerar.
        if (!paradaFeita && pararPor > 0.0f && static_cast<int>(current) == pararNoPonto) {
            paradaFeita    = true;
            paradaRestante = pararPor;
            rb->SetVelocity(Vector2::Zero);
            return;
        }

        ++current;

        if (current >= waypoints->size()) {
            // Fim do caminho: encerra e deixa a velocidade atual valendo, entao
            // o projetil segue reto. Zerar a velocidade aqui criaria um
            // projetil imortal, parado na tela e nunca devolvido ao pool.
            finished = true;
        }
        return;
    }

    // CONTRATO DE ProjectileMotion: decide a DIRECAO e preserva o MODULO.
    // O modulo inicial ja foi aplicado na ativacao (pathSpeed); daqui em
    // diante quem manda nele sao os Modifiers. Escrever toTarget * pathSpeed
    // aqui anulava um SlowDown aplicado depois da ativacao no frame seguinte.
    rb->SetVelocity(DirecionarPreservandoModulo(rb->GetVelocity(), toTarget, pathSpeed));
}

// ---------------------------------------------------------------------------
// PathBehavior

// ---------------------------------------------------------------------------
// MiraPeriodicaBehavior
// ---------------------------------------------------------------------------
void MiraPeriodicaBehavior::update(Projectile* p, float deltaTime) {

    // Precisa saber onde o jogador esta, e so BossProjectile sabe.
    auto bossProj = dynamic_cast<BossProjectile*>(p);
    if (!bossProj) {
        feitas = ritmo.repeticoes;
        return;
    }

    elapsedTime += deltaTime;
    // O instante da proxima mirada vem do ritmo, nao de um contador proprio:
    // e o que garante que ela caia sempre no inicio de uma investida.
    if (feitas >= ritmo.repeticoes || elapsedTime < ritmo.InicioDaInvestida(feitas)) return;

    auto rb = bossProj->GetComponent<RigidBodyComponent>();
    if (!rb) {
        feitas = ritmo.repeticoes;
        return;
    }

    if (bossProj->HasPlayer()) {
        const Vector2 paraJogador = bossProj->GetPlayerPosition() - p->GetPosition();
        const Vector2 velocidade = rb->GetVelocity();
        // O modulo minimo e o proprio modulo atual: esta Motion nunca acelera
        // nem freia o projetil, so o gira.
        rb->SetVelocity(DirecionarPreservandoModulo(velocidade, paraJogador, velocidade.Length()));
    }

    ++feitas;
}

// ---------------------------------------------------------------------------
// PulsoDeVelocidadeBehavior
// ---------------------------------------------------------------------------
void PulsoDeVelocidadeBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb) {
        terminou = true;
        return;
    }

    elapsedTime += deltaTime;

    const FaseDoPulso fase = CalcularFaseDoPulso(elapsedTime, ritmo,
                                                 moduloInvestida, moduloPausa);
    if (!fase.ativo) {
        // Inativo tem duas causas: ainda no atraso, ou todos os ciclos ja
        // aconteceram. So a segunda encerra o Modifier.
        if (fase.ciclo >= ritmo.repeticoes) terminou = true;
        return;
    }

    rb->SetVelocity(AjustarModulo(rb->GetVelocity(), fase.modulo));
}

void DeslocarBehavior::update(Projectile* p, const float deltaTime) {
    t += deltaTime;

    const float alvo = Flex::Progresso(t, atraso, duracao);
    p->SetPosition(p->GetPosition() + delta * (alvo - jaDeslocado));
    jaDeslocado = alvo;

    if (t >= atraso + duracao) terminou = true;
}

void PromessaBehavior::update(Projectile* p, const float deltaTime) {

    t += deltaTime;
    if (t < atraso) return;
    assentou = true;

    if (!Promessa::Resolve(Random::GetFloatRange(0.0f, 1.0f), fracao)) {
        p->Kill();   // rejeitada
        return;
    }

    // RESOLVIDA: sai apontada para o alvo, com o modulo do arquivo. Parada, ela
    // nao tem direcao nenhuma, entao a direcao vem do alvo, e nao da velocidade.
    auto* promessa = dynamic_cast<BossProjectile*>(p);
    auto* rb = p->GetComponent<RigidBodyComponent>();
    if (promessa && rb) rb->SetVelocity(promessa->GetPlayerDirection() * velocidade);

    if (!escreve.empty()) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(escreve);
    }
}

void AmarrarBehavior::update(Projectile* p, const float deltaTime) {

    auto* rb = p->GetComponent<RigidBodyComponent>();
    const auto* batalha = dynamic_cast<Battle*>(p->GetScene());
    if (!rb || !batalha) { corda.cortada = true; return; }

    const Vector2 letra = p->GetPosition();
    const Boss* no = nullptr;
    float maisPerto = 0.0f;
    for (const Boss* corpo : batalha->Corpos()) {
        if (!corpo) continue;
        const float d = (corpo->GetPosition() - letra).LengthSq();
        if (!no || d < maisPerto) { no = corpo; maisPerto = d; }
    }
    if (!no) { corda.cortada = true; return; }   // sem professor, a corda cai

    const auto lance = Amarra::Passo(corda, letra, rb->GetVelocity(), no->GetPosition(), deltaTime);
    p->SetPosition(lance.posicao);
    rb->SetVelocity(lance.velocidade);

    if (lance.evento == Amarra::Evento::Amarrou && !escreve.empty()) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(escreve);
    }
}

void HistoriaDaSprintBehavior::update(Projectile* p, const float deltaTime) {
    auto* rb = p->GetComponent<RigidBodyComponent>();
    if (!rb || !plano) { caiu = true; return; }

    auto* historia = dynamic_cast<BossProjectile*>(p);

    t += deltaTime;
    if (t < plano->revisao) {
        p->SetPosition(Sprint::PosicaoDaHistoria(*plano, indice, t));
        rb->SetVelocity(Vector2::Zero);

        // SOLTA, ela deixa de ser blindada: na mao do cursor o tiro so some.
        if (!solta && t >= plano->arraste) {
            solta = true;
            if (historia) historia->DefinirBlindado(false);
        }
        // PARADA NA VAGA, ela entra no posto do quadro: esta em andamento, e
        // a PontosAttack atira dela.
        if (!noQuadro && t >= plano->arraste + plano->espalha) {
            noQuadro = true;
            if (historia) historia->DefinirPosto(Sprint::kPostoDoQuadro);
        }
        return;
    }

    // A REVISAO: o que nao foi entregue sai do quadro e cai em cima do aluno.
    // Mira UMA vez, onde ele vai estar (RumoPrevendo, PathAim.h); sem aluno,
    // cai reto.
    Vector2 rumo(0.0f, 1.0f);
    if (historia) {
        historia->DefinirBlindado(false);
        historia->DefinirPosto("");
        if (historia->HasPlayer()) {
            rumo = RumoPrevendo(p->GetPosition(), historia->GetPlayerPosition(),
                                historia->GetPlayerVelocity(), queda);
        }
    }
    rb->SetVelocity(rumo * queda);
    caiu = true;
}

void CursorDaSprintBehavior::update(Projectile* p, const float deltaTime) {
    t += deltaTime;
    if (!plano) { acabou = true; p->Kill(); return; }

    const auto gesto = Sprint::GestoDoCursor(*plano, t);
    if (gesto == Sprint::Gesto::Acabou) {
        // Kill() so marca o estado: quem limpa os behaviors e o pool, depois.
        acabou = true;
        p->Kill();
        return;
    }

    p->SetPosition(Sprint::CentroDoCursor(Sprint::PontaDoCursor(*plano, t), p->GetScale()));
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);

    const bool clica = (gesto == Sprint::Gesto::Soltando || gesto == Sprint::Gesto::Clicando);
    if (clica != clicando) {
        clicando = clica;
        const std::string& cara = clica ? clique : seta;
        if (!cara.empty()) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(cara);
        }
    }
}

void AncorarBehavior::update(Projectile* p, const float deltaTime) {
    auto* rb = p->GetComponent<RigidBodyComponent>();
    if (!rb) { soltou = true; return; }

    t += deltaTime;
    const Boss* mae = nullptr;
    if (const auto* batalha = dynamic_cast<Battle*>(p->GetScene()); batalha && t < duracao) {
        float maisPerto = 0.0f;
        for (const Boss* corpo : batalha->Corpos()) {
            if (!corpo) continue;
            const float d = (corpo->GetPosition() - p->GetPosition()).LengthSq();
            if (!mae || d < maisPerto) { mae = corpo; maisPerto = d; }
        }
    }

    // O TEMPO ACABOU, ou nao ha professor: a janela solta e cai.
    if (!mae) {
        rb->SetVelocity(Vector2(0.0f, queda));
        soltou = true;
        return;
    }

    p->SetPosition(Ancora::Aproximar(p->GetPosition(), mae->GetPosition() + offset, Ancora::kTaxa, deltaTime));
    rb->SetVelocity(Vector2::Zero);
}
