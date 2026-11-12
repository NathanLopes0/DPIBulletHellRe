//
// O relogio da fase: a capsula que esvazia e o reloginho, ao lado do professor.
//

#pragma once

class Actor;
class Boss;
class DrawRelogioDaFaseComponent;
class Scene;

/// A capsula do tempo da fase (DrawRelogioDaFaseComponent) e o reloginho embaixo dela, que seguem o
/// professor. Quem conta o tempo e o BattleHUD, que recebe a capsula; isto so a poe no lugar.
class RelogioDaFase {
public:
    /// A capsula tem a altura do desenho do 'chefe'.
    RelogioDaFase(Scene& cena, const Boss& chefe);

    [[nodiscard]] Actor* Capsula() const { return mCapsula; }
    [[nodiscard]] DrawRelogioDaFaseComponent* DesenhoDaCapsula() const { return mDesenhoDaCapsula; }

    /// Um quadro: a capsula e o relogio seguem o 'chefe', e o relogio pisca vermelho nos ultimos
    /// segundos da fase ('restante').
    void Seguir(const Boss& chefe, float restante);

private:
    Actor* mCapsula = nullptr;
    DrawRelogioDaFaseComponent* mDesenhoDaCapsula = nullptr;
    Actor* mRelogio = nullptr;
};
