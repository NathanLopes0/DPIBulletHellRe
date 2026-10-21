//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

#include "../../../Attacks/AttackParameters/FlexAttackParams.h"

Lucas::Lucas(Scene* scene) : Boss(scene) {
}

void Lucas::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Lucas::OnProjetilDerrubado() {
    ++mParaReiniciar;
}

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O SUPERVISOR DEVOLVE OS PROCESSOS QUE CAIRAM, na rodada seguinte da fase
    // 3 e na mesma rajada dos novos. Escrito como TOTAL, a partir do numero do
    // arquivo: params e o mesmo objeto a cada disparo (Boss.h), e somar sobre o
    // anterior faria a rodada crescer para sempre.
    if (stateName == "StateThree") {
        if (mProcessosDoArquivo < 0) mProcessosDoArquivo = params.numProjectiles;
        const int voltam = (mParaReiniciar < kReiniciosPorRodada) ? mParaReiniciar
                                                                  : kReiniciosPorRodada;
        params.numProjectiles = mProcessosDoArquivo + voltam;
        mParaReiniciar -= voltam;
        return;
    }

    // O FLEXBOX VAI E VOLTA. Uma fileira se espalha, a seguinte se junta: o
    // lugar seguro nunca e o mesmo duas rajadas seguidas, e decorar nao serve.
    auto* flex = dynamic_cast<FlexAttackParams*>(&params);
    if (!flex) return;

    if (!mGuardouOFlex) {
        mDeDoArquivo = flex->de;
        mParaDoArquivo = flex->para;
        mGuardouOFlex = true;
    }

    const bool deVolta = (mFileirasDoFlex % 2) == 1;
    flex->de   = deVolta ? mParaDoArquivo : mDeDoArquivo;
    flex->para = deVolta ? mDeDoArquivo   : mParaDoArquivo;
    ++mFileirasDoFlex;

    // E O PLACAR ESCREVE A REGRA DESTA FILEIRA. A fileira nasce no arranjo
    // velho e so muda depois de "aos" segundos: e o tempo de ler o quadro e
    // saber para onde ela vai se abrir.
    RegistrarAnuncio(stateName, "justify-content: " + Flex::Nome(flex->para));
}
