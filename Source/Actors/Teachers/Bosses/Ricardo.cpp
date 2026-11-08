//
// O chefe da INF 250 - Organizacao de Computadores.
//

#include "Ricardo.h"

#include <algorithm>
#include <limits>

#include "../../../Motor/Game.h"
#include "../../../Motor/Random.h"
#include "../../../Attacks/AttackParameters/PalavraAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Attacks/Ancora.h"
#include "../../Player/Player.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

Ricardo::Ricardo(Scene* scene) : Boss(scene)
{
}

void Ricardo::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();

    // Fase 1: as portas, uma por etapa, nos fins que o arquivo deu.
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Logica::kFaseDasPortas);
        const Logica::Plano plano = Logica::Sortear(sorteio);
        auto etapas = EtapasDa(Logica::kFaseDasPortas);
        for (std::size_t k = 0; k < etapas.size() && k < plano.portas.size(); ++k) {
            etapas[k].second = Logica::Anuncio(plano.portas[k]);
        }
        if (!etapas.empty()) RegistrarEtapas(Logica::kFaseDasPortas, etapas);
    }
    // Fase 2: as mesmas frequencias, com o tempo de cada uma sorteado.
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Relogio::kFaseDoRelogio);
        mPlanoDoRelogio = Relogio::Sortear(sorteio);
        mDegrauDoRelogio = Relogio::DegrauDo(nivel);
        auto etapas = EtapasDa(Relogio::kFaseDoRelogio);
        float fim = 0.0f;
        for (std::size_t k = 0; k < etapas.size() && k < mPlanoDoRelogio.duracoes.size(); ++k) {
            fim += static_cast<float>(mPlanoDoRelogio.duracoes[k]);
            etapas[k].first = fim;
        }
        if (!etapas.empty()) RegistrarEtapas(Relogio::kFaseDoRelogio, etapas);
    }
    // Fase 3: as voltas do laco.
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Pipeline::kFaseDoPipeline);
        mPlanoDoPipeline = Pipeline::Sortear(sorteio);
        mDegrauDoPipeline = Pipeline::DegrauDo(nivel);
    }
    // A repescagem: o delay do sketch, pela metade a cada etapa.
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Blink::kFaseDoBlink);
        const Blink::Plano plano = Blink::Sortear(sorteio);
        mDegrauDoBlink = Blink::DegrauDo(nivel);
        auto etapas = EtapasDa(Blink::kFaseDoBlink);
        for (std::size_t k = 0; k < etapas.size(); ++k) {
            etapas[k].second = Blink::Anuncio(static_cast<int>(k), plano, mDegrauDoBlink);
        }
        if (!etapas.empty()) RegistrarEtapas(Blink::kFaseDoBlink, etapas);
    }
}

void Ricardo::OnUpdate(const float deltaTime) {
    // Antes de qualquer ataque: a primeira palavra ja e sorteada para a porta da luta.
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarPortas();
    AtualizarRelogio();
    AtualizarPipeline(deltaTime);
    AtualizarBlink();
}

void Ricardo::AtualizarBlink() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noBlink = fsm && fsm->GetStateName() == Blink::kFaseDoBlink && !EhClone();
    if (!noBlink) {
        mAntesNoBlink = -1.0f;
        return;
    }
    if (mAntesNoBlink < 0.0f) mAntesNoBlink = 0.0f;

    const float agora = fsm->GetStateTime();
    const float delay = Blink::LerDelay(AnuncioDaFase(Blink::kFaseDoBlink)).value_or(1.0f);
    const bool esquerda = Blink::EsquerdaAcesa(agora, delay);

    // O led de cada placa: aceso no lado da vez.
    std::vector<BossProjectile*> placas = NoPosto(Blink::kPostoDasPlacas);
    for (BossProjectile* acesa : NoPosto(Blink::kPostoDasAcesas)) placas.push_back(acesa);
    for (BossProjectile* placa : placas) {
        // A placa que se soltou esta desplugada: cai apagada, e nao atira mais.
        if (placa->GetPosition().y > GetPosition().y + kSoltaAbaixo) {
            placa->DefinirPosto("");
            if (auto* anim = placa->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Blink::kAnimacaoApagada);
            continue;
        }
        const bool daEsquerda = placa->GetPosition().x < GetPosition().x;
        const bool acesa = (daEsquerda == esquerda);
        placa->DefinirPosto(acesa ? Blink::kPostoDasAcesas : Blink::kPostoDasPlacas);
        const char* cara = acesa ? Blink::kAnimacaoAcesa : Blink::kAnimacaoApagada;
        if (auto* anim = placa->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    // Acendeu: o lado da vez atira.
    if (Blink::Trocou(mAntesNoBlink, agora, delay)) Disparar(Blink::kGatilhoDoLed, -1, mDegrauDoBlink.rapidez);
    mAntesNoBlink = agora;
}

std::vector<Vector2> Ricardo::CentrosDosEstagios() const {
    std::vector<Vector2> centros;
    for (const BossProjectile* moldura : NoPosto(Pipeline::kPostoDosEstagios)) centros.push_back(moldura->GetPosition());
    std::sort(centros.begin(), centros.end(), [](const Vector2& a, const Vector2& b) { return a.x < b.x; });
    if (static_cast<int>(centros.size()) != Pipeline::kEstagios) centros.clear();
    return centros;
}

void Ricardo::DispararDoEstagio(const std::string& gatilho, const int estagio, const bool mirar) {
    const auto centros = CentrosDosEstagios();
    if (centros.empty() || gatilho.empty()) return;
    mDisparandoDoEstagio = true;
    mMirandoDoEstagio = mirar;
    mDisparoDoEstagio = centros[static_cast<size_t>(estagio)];
    Disparar(gatilho, -1, mFormandoEstagios ? 1.0f : mDegrauDoPipeline.rapidez);
    mDisparandoDoEstagio = false;
    mMirandoDoEstagio = false;
}

void Ricardo::AtualizarPipeline(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noPipeline = fsm && fsm->GetStateName() == Pipeline::kFaseDoPipeline && !EhClone();

    if (!noPipeline) {
        // A fase acabou: o pipeline se desfaz, e o placar volta ao nome dela.
        if (mNoPipeline) {
            for (BossProjectile* chip : NoPosto(Pipeline::kPostoDoPipeline)) chip->Kill();
            for (BossProjectile* moldura : NoPosto(Pipeline::kPostoDosEstagios)) moldura->Kill();
            mChips.fill(nullptr);
            if (!mNomeDaFaseDoPipeline.empty()) RegistrarAnuncio(Pipeline::kFaseDoPipeline, mNomeDaFaseDoPipeline);
            mNoPipeline = false;
        }
        return;
    }
    if (!mNoPipeline) {
        mNoPipeline = true;
        mEstagiosFormados = false;
        mPipeline = Pipeline::Estado{};
        mPipeline.voltas = mPlanoDoPipeline.voltas;
        mChips.fill(nullptr);
        mTempoNoPipeline = 0.0f;
        mProximoCiclo = Pipeline::kPrimeiroCiclo;
        if (mNomeDaFaseDoPipeline.empty()) mNomeDaFaseDoPipeline = AnuncioDaFase(Pipeline::kFaseDoPipeline);
    }
    mTempoNoPipeline += deltaTime;

    if (!mEstagiosFormados && mTempoNoPipeline >= kFormaAsPortas) {
        mFormandoEstagios = true;
        Disparar(Pipeline::kGatilhoDosEstagios);
        mFormandoEstagios = false;
        mEstagiosFormados = true;
    }
    const auto centros = CentrosDosEstagios();
    if (centros.empty()) return;

    // O nome de cada estagio, pela ordem da esquerda para a direita.
    if (mScene && mScene->GetGame()) {
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        for (BossProjectile* moldura : NoPosto(Pipeline::kPostoDosEstagios)) {
            const char* nome = Pipeline::NomeDoEstagio(Logica::Coluna(moldura->GetPosition().x, largura, Pipeline::kEstagios));
            if (auto* anim = moldura->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != nome) {
                anim->SetAnimation(nome);
            }
        }
    }

    // O chip derrubado a tiro vira bolha no estagio em que estava, e nao faz o ataque dele.
    for (int s = 0; s < Pipeline::kEstagios; ++s) {
        BossProjectile*& chip = mChips[static_cast<size_t>(s)];
        if (chip && (chip->GetState() != ActorState::Active || chip->Posto() != Pipeline::kPostoDoPipeline)) {
            Pipeline::Estourar(mPipeline, s);
            chip = nullptr;
        }
    }

    if (mTempoNoPipeline >= mProximoCiclo) {
        mProximoCiclo += mDegrauDoPipeline.ciclo;

        // O que estava no wb sai, e todo mundo anda um estagio.
        if (BossProjectile* saindo = mChips[Pipeline::WB]) saindo->Kill();
        for (int s = Pipeline::WB; s > Pipeline::IF; --s) mChips[static_cast<size_t>(s)] = mChips[static_cast<size_t>(s - 1)];
        mChips[Pipeline::IF] = nullptr;
        const auto ciclo = Pipeline::Avancar(mPipeline);
        const auto& programa = Pipeline::Programa();

        // A busca: o chip da instrucao nasce no if.
        if (mPipeline.estagio[Pipeline::IF] != Pipeline::kBolha) {
            mBuscada = nullptr;
            DispararDoEstagio(Pipeline::kGatilhoDaBusca, Pipeline::IF, false);
            if (mBuscada) {
                const auto& instrucao = programa[static_cast<size_t>(mPipeline.estagio[Pipeline::IF])];
                if (auto* anim = mBuscada->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Pipeline::Animacao(instrucao.tipo));
                mChips[Pipeline::IF] = mBuscada;
            }
        }

        // O flush: o que foi buscado pelo caminho errado vira bolha.
        if (ciclo.flush) {
            mBuscada = nullptr;
            DispararDoEstagio(Pipeline::kGatilhoDaBusca, Pipeline::IF, false);
            if (mBuscada) mChips[Pipeline::IF] = mBuscada;
            for (const int s : {Pipeline::IF, Pipeline::ID}) {
                if (BossProjectile* chip = mChips[static_cast<size_t>(s)]) {
                    if (auto* anim = chip->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Pipeline::kAnimacaoDaBolha);
                    chip->DefinirEnfeite(true);
                }
            }
        }

        // Os ataques: quem chegou ao estagio do seu ataque o faz, de la.
        for (const int s : {Pipeline::EX, Pipeline::MEM}) {
            const int i = mPipeline.estagio[static_cast<size_t>(s)];
            if (i == Pipeline::kBolha) continue;
            const auto tipo = programa[static_cast<size_t>(i)].tipo;
            DispararDoEstagio(Pipeline::Gatilho(tipo, s), s, tipo == Pipeline::Tipo::Add || tipo == Pipeline::Tipo::Addi);
        }

        // O placar diz o que esta no ex - ou o flush.
        const int noEX = mPipeline.estagio[Pipeline::EX];
        std::string placar = "EX: " + (noEX == Pipeline::kBolha ? std::string("bolha")
                                                                : programa[static_cast<size_t>(noEX)].texto);
        if (ciclo.flush) placar += " \xC2\xB7 flush";   // O ponto do meio, em UTF-8.
        RegistrarAnuncio(Pipeline::kFaseDoPipeline, placar);
    }

    // Cada chip anda ate o seu estagio.
    for (int s = 0; s < Pipeline::kEstagios; ++s) {
        BossProjectile* chip = mChips[static_cast<size_t>(s)];
        if (!chip) continue;
        chip->SetPosition(Ancora::Aproximar(chip->GetPosition(), centros[static_cast<size_t>(s)], kRapidezDoChip, deltaTime));
        if (auto* rb = chip->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

std::vector<BossProjectile*> Ricardo::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

Logica::Porta Ricardo::PortaEm(const float t) const {
    if (const auto porta = Logica::LerPorta(AnuncioDaFaseEm(Logica::kFaseDasPortas, t), Logica::kBits)) {
        return *porta;
    }
    return mPorta;
}

void Ricardo::AtualizarPortas() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasPortas = fsm && fsm->GetStateName() == Logica::kFaseDasPortas && !EhClone();
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    if (nasPortas) {
        mPorta = PortaEm(fsm->GetStateTime());
        if (!mPortasFormadas && fsm->GetStateTime() >= kFormaAsPortas) {
            mDisparandoPortas = true;
            Disparar(Logica::kGatilhoDasPortas);
            mDisparandoPortas = false;
            mPortasFormadas = true;
        }
    }

    // A cara de cada porta: a operacao da etapa e o B da coluna dela.
    const auto portas = NoPosto(Logica::kPostoDasPortas);
    for (BossProjectile* porta : portas) {
        const int coluna = Logica::Coluna(porta->GetPosition().x, largura, Logica::kBits);
        const std::string cara = Logica::AnimacaoDaPorta(mPorta, coluna, Logica::kBits);
        if (auto* anim = porta->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    // A travessia.
    if (!portas.empty()) {
        const float altura = portas.front()->GetPosition().y;
        for (BossProjectile* bit : NoPosto(Logica::kPostoDaPalavra)) {
            if (bit->GetPosition().y < altura) continue;
            const int coluna = Logica::Coluna(bit->GetPosition().x, largura, Logica::kBits);
            const bool sai = Logica::Saida(mPorta.operacao, !bit->Enfeite(), Logica::Bit(mPorta.b, coluna, Logica::kBits));
            if (auto* anim = bit->GetComponent<DrawAnimatedComponent>()) {
                anim->SetAnimation(sai ? Logica::kAnimacaoDoUm : Logica::kAnimacaoDoZero);
            }
            bit->DefinirEnfeite(!sai);
            bit->DefinirPosto("");
        }
    }

    // Fora da fase as portas ficam ate a ultima palavra passar por elas.
    if (!nasPortas && mPortasFormadas && NoPosto(Logica::kPostoDaPalavra).empty()) {
        for (BossProjectile* porta : portas) porta->Kill();
        mPortasFormadas = false;
    }
}

void Ricardo::MoverRegistradores(const float velocidade) {
    for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) {
        const auto direcao = mDirecoes.find(r);
        if (direcao == mDirecoes.end()) continue;   // ainda nao acordou
        if (auto* rb = r->GetComponent<RigidBodyComponent>()) rb->SetVelocity(direcao->second * velocidade);
    }
}

void Ricardo::AtualizarRelogio() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noRelogio = fsm && fsm->GetStateName() == Relogio::kFaseDoRelogio && !EhClone();

    if (!noRelogio) {
        if (mAntesNoRelogio >= 0.0f) {
            MoverRegistradores(mDegrauDoRelogio.passo * mFrequencia);
            for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) r->DefinirPosto("");
            mDirecoes.clear();
            mAntesNoRelogio = -1.0f;
            mFimDoPasso = -1.0f;
        }
        return;
    }
    if (mAntesNoRelogio < 0.0f) {
        mAntesNoRelogio = 0.0f;
        mContador = mPlanoDoRelogio.inicioDoContador;
    }

    const float agora = fsm->GetStateTime();
    mFrequencia = Relogio::LerFrequencia(AnuncioDaFase(Relogio::kFaseDoRelogio)).value_or(1.0f);

    // O registrador que acabou de acordar anda na direcao do leque.
    for (auto it = mDirecoes.begin(); it != mDirecoes.end();) {
        if (it->first->GetState() != ActorState::Active || it->first->Posto() != Relogio::kPostoDosRegistradores) {
            it = mDirecoes.erase(it);
        } else {
            ++it;
        }
    }
    for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) {
        if (mDirecoes.count(r)) continue;
        auto* rb = r->GetComponent<RigidBodyComponent>();
        if (!rb || rb->GetVelocity().LengthSq() <= 0.0f) continue;
        Vector2 direcao = rb->GetVelocity();
        direcao.Normalize();
        mDirecoes[r] = direcao;
        rb->SetVelocity(Vector2::Zero);
    }

    if (Relogio::Subiu(mAntesNoRelogio, agora, 1.0f)) Disparar(Relogio::kGatilhoDoContador);

    // A borda de subida: todo registrador da um passo de kPasso pixels...
    if (Relogio::Subiu(mAntesNoRelogio, agora, mFrequencia)) {
        const float duracao = Relogio::DuracaoDoPasso(mFrequencia);
        MoverRegistradores(mDegrauDoRelogio.passo / duracao);
        mFimDoPasso = agora + duracao;
    }
    // ...e para, ate a proxima.
    else if (mFimDoPasso >= 0.0f && agora >= mFimDoPasso) {
        MoverRegistradores(0.0f);
        mFimDoPasso = -1.0f;
    }
    mAntesNoRelogio = agora;
}

void Ricardo::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                   const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // A busca do pipeline: o chip que nasceu e o da instrucao buscada.
    if (stateName == Pipeline::kFaseDoPipeline && mDisparandoDoEstagio && !projectiles.empty()) {
        if (auto* chip = dynamic_cast<BossProjectile*>(projectiles.front().get());
            chip && chip->Posto() == Pipeline::kPostoDoPipeline) {
            mBuscada = chip;
        }
        return;
    }
    if (stateName != Relogio::kFaseDoRelogio) return;

    // O leque e o valor do contador: um bit por tiro, o mais alto a esquerda de quem o recebe.
    const int n = static_cast<int>(projectiles.size());
    const std::uint32_t valor = Relogio::Johnson(mContador, n);
    for (int i = 0; i < n; ++i) {
        auto* bit = dynamic_cast<BossProjectile*>(projectiles[static_cast<size_t>(i)].get());
        if (!bit) continue;
        mDirecoes.erase(bit);
        const bool um = Logica::Bit(valor, n - 1 - i, n);
        if (auto* anim = bit->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(um ? Logica::kAnimacaoDoUm : Logica::kAnimacaoDoZero);
        }
        bit->DefinirEnfeite(!um);
    }
    ++mContador;
}

void Ricardo::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // No pipeline o ataque sai do estagio, e nao do professor.
    if (stateName == Pipeline::kFaseDoPipeline) {
        if (mFormandoEstagios || !mDisparandoDoEstagio) return;
        params.firePosition = mDisparoDoEstagio;
        if (mMirandoDoEstagio) {
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - mDisparoDoEstagio;
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
            const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f;
            params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
        }
        if (auto* palavra = dynamic_cast<PalavraAttackParams*>(&params)) {
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            palavra->palavra = Logica::Sortear(Logica::Porta{Logica::Operacao::Nao, 0u}, params.numProjectiles, sorteio);
        }
        return;
    }

    // O leque do contador sai mirado no aluno, com um bit apontado para ele.
    if (stateName == Relogio::kFaseDoRelogio) {
        const Vector2 d = GetDirectionToPlayer();
        const float meioPasso = params.numProjectiles > 1
                                    ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1)
                                    : 0.0f;
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
        return;
    }

    // A fileira de portas nao e palavra: quem lhe da a cara e AtualizarPortas.
    if (mDisparandoPortas) return;
    if (stateName != Logica::kFaseDasPortas) return;
    auto* palavra = dynamic_cast<PalavraAttackParams*>(&params);
    if (!palavra) return;

    // Justa (Logica::Justa): parede com porta antes e depois, e a porta muda alguma coisa.
    const auto* fsm = GetComponent<FSMComponent>();
    float chegada = fsm ? fsm->GetStateTime() : 0.0f;
    const auto portas = NoPosto(Logica::kPostoDasPortas);
    const float de = palavra->altura >= 0.0f ? palavra->altura : params.firePosition.y;
    if (!portas.empty() && params.projectileSpeed > 0.0f) {
        chegada += std::max(0.0f, portas.front()->GetPosition().y - de) / params.projectileSpeed;
    }
    const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    palavra->palavra = Logica::Sortear(PortaEm(chegada), params.numProjectiles, sorteio);
}
