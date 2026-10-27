//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

#include "../../../Attacks/AttackParameters/FlexAttackParams.h"

Lucas::Lucas(Scene* scene) : Boss(scene) {
}

void Lucas::SortearALuta() {
    mLutaSorteada = true;
    mSorteioDoFlex = SorteioDaFase(kFaseDoFlex);
}

void Lucas::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
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

    // O FLEXBOX SAI DE ONDE PAROU. Cada fileira nasce no arranjo em que a
    // anterior terminou e se rearranja para uma regra sorteada, nunca a mesma
    // (Flex::Proxima): o lugar seguro nunca e o mesmo duas rajadas seguidas, e
    // qual regra vem nao se decora - quem diz e o placar.
    auto* flex = dynamic_cast<FlexAttackParams*>(&params);
    if (!flex) return;

    if (!mGuardouOFlex) {
        mUltimoArranjo = flex->de;
        mGuardouOFlex = true;
    }



    flex->de = mUltimoArranjo;
    flex->para = Flex::Proxima(mSorteioDoFlex, mUltimoArranjo);


    mUltimoArranjo = flex->para;


    // E O PLACAR ESCREVE A REGRA DESTA FILEIRA. A fileira nasce no arranjo
    // velho e so muda depois de "aos" segundos: e o tempo de ler o quadro e
    // saber para onde ela vai se abrir.
    RegistrarAnuncio(stateName, "justify-content: " + Flex::Nome(flex->para));
}
