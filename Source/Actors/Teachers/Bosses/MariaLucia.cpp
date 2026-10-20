//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

#include "../../../Attacks/EscopoQueCresce.h"
#include "../../../Attacks/AttackParameters/AttackParams.h"

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void MariaLucia::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // O ESCOPO QUE CRESCE, e a unica coisa desta chefe que nao cabe em dados:
    // "quanto maior que a rajada ANTERIOR" depende da anterior, e fases.json
    // nao tem memoria. Mesmo motivo da fresta giratoria do Vitor.
    //
    // SO NA FASE 1, e por um motivo que vale escrever: CustomizeAttackParams
    // recebe o NOME DA FASE e nada mais - nao da para saber QUAL dos ataques
    // da fase esta disparando. Na fase 3 ha dois (o debito e o requisito) e
    // crescer os dois junto seria crescer o errado tambem. A fase 3 ja tem a
    // acumulacao dela, por outro mecanismo: o debito que para e fica.
    if (stateName != "StateOne") return;

    // GUARDA O QUE O ARQUIVO PEDIU, NA PRIMEIRA VEZ. Sem isto a conta sai
    // errada de um jeito que nao da para ver lendo: AttackParams e o MESMO
    // objeto a cada disparo (Boss::ExecuteAttack entrega sempre o mesmo), entao
    // "+= dois projeteis" soma sobre o disparo anterior e nao sobre o arquivo.
    // Medido na primeira versao: a fase terminava com 54 projeteis num leque de
    // 285 graus. Ver a nota em MariaLucia.h.
    if (mProjeteisDePartida < 0) {
        mProjeteisDePartida = params.numProjectiles;
        mLequeDePartida     = params.angle;
    }

    // ESCREVE O VALOR INTEIRO, e por isso guardar a partida importa: assim o
    // arquivo continua sendo o escopo DE PARTIDA em vez de virar texto morto -
    // a armadilha que "angulo" prega em CircleSpreadAttack, onde o leitor
    // aceita o campo e a estrategia o ignora.
    const auto a = Escopo::DaRajada(mSprint, kPassoDeProjeteis, kPassoDeLeque);
    params.numProjectiles = mProjeteisDePartida + a.projeteis;
    params.angle          = mLequeDePartida     + a.leque;

    mSprint = Escopo::SprintSeguinte(mSprint, kSprintsAteAEntrega);
}
