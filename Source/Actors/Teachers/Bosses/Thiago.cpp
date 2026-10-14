//
// Thiago - professor de Banco de Dados (INF 220)
//

#include "Thiago.h"

#include "../../../Attacks/AttackParameters/ConsultaAttackParams.h"
#include "../../../Random.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Tabela.h"
#include "../../Player/Player.h"

Thiago::Thiago(Scene* scene) : Boss(scene)
{
}

void Thiago::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

int Thiago::FaixaDoJogador(const ConsultaAttackParams& consulta) {

    const auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle || !battle->GetPlayer()) return 0;

    const SDL_FRect campo = battle->GetPlayfieldBounds();
    const Vector2 onde = battle->GetPlayer()->GetPosition();

    // A MESMA CONTA que a ConsultaAttack usa para posicionar a varredura, de
    // Tabela. Se este arquivo fizesse a propria divisao, bastaria um
    // arredondamento diferente para o chefe anunciar uma linha e varrer a
    // vizinha - e o jogador seria atingido por um ataque que leu certo.
    return (consulta.eixo == ConsultaAttackParams::Eixo::Linha)
               ? Tabela::FaixaDe(campo.y, campo.h, consulta.forma.linhas,  onde.y)
               : Tabela::FaixaDe(campo.x, campo.w, consulta.forma.colunas, onde.x);
}

void Thiago::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Fases que nao usam ConsultaAttack caem fora daqui e seguem com o que o
    // arquivo disser - e o caso dos ataques comuns misturados as consultas.
    auto* consulta = dynamic_cast<ConsultaAttackParams*>(&params);
    if (!consulta) return;

    const int quantas = (consulta->eixo == ConsultaAttackParams::Eixo::Linha)
                            ? consulta->forma.linhas
                            : consulta->forma.colunas;
    if (quantas <= 0) return;

    if (stateName == "StateOne") {
        // SELECT SEM WHERE. Percorre a tabela inteira, em ordem, sem olhar para
        // o jogador. E a fase que ENSINA a tabela: depois de duas voltas o
        // jogador ja sabe quantas linhas existem e quanto tempo tem entre uma e
        // a seguinte. Aleatorio aqui pareceria igual e ensinaria nada.
        consulta->indice = mProximaFaixa % quantas;
        ++mProximaFaixa;
    }
    else if (stateName == "StateTwo") {
        // WHERE. A consulta encontra o jogador. O aviso continua valendo, entao
        // ficar parado e que mata - nao a consulta em si.
        consulta->indice = FaixaDoJogador(*consulta);
    }
    else if (stateName == "StateThree") {
        // A TRANSACAO. Metade das consultas vai na faixa do jogador e metade na
        // que ele acabou de deixar: a varredura anterior "volta". Quem aprendeu
        // na fase 2 a pular para o lado e esperar descobre que o espaco limpo e
        // justamente para onde o desfazimento vem.
        const int aquiAgora = FaixaDoJogador(*consulta);

        if (mProximaFaixa % 2 == 0) {
            consulta->indice = aquiAgora;
        }
        else {
            // A vizinha, alternando o lado para nao virar um padrao de um lado so.
            const int lado = (Random::GetIntRange(0, 1) == 0) ? -1 : 1;
            int vizinha = aquiAgora + lado;
            if (vizinha < 0) vizinha = 1;
            if (vizinha >= quantas) vizinha = quantas - 2;
            consulta->indice = (vizinha < 0) ? 0 : vizinha;
        }
        ++mProximaFaixa;
    }
    else if (stateName == "StateFinal") {
        // CROSS JOIN. Aqui NAO ha escolha de faixa: o arquivo dispara varias
        // consultas por rajada, cada uma com o proprio indice fixo, e juntas
        // elas formam o produto cartesiano. Mexer no indice aqui desmontaria a
        // treliça que o arquivo descreve.
    }
}
