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

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

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
