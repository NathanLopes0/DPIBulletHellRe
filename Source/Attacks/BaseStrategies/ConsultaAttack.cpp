//
// A varredura de uma linha ou coluna da tabela.
//

#include "ConsultaAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Attacks/AttackParameters/ConsultaAttackParams.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Tabela.h"

ConsultaAttack::ConsultaAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> ConsultaAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Mesmo contrato da BaloonAttack: sem os campos proprios nao ha consulta
    // nenhuma a fazer, e disparar "alguma coisa" esconderia o erro de
    // configuracao atras de um ataque que quase funciona.
    const auto* consulta = dynamic_cast<const ConsultaAttackParams*>(&params);
    if (!consulta) {
        SDL_Log("ConsultaAttack: recebeu AttackParams comum, sem o bloco \"consulta\". "
                "Nada foi disparado.");
        return projectiles;
    }

    if (!Tabela::Valida(consulta->forma)) {
        SDL_Log("ConsultaAttack: tabela de %dx%d nao tem onde varrer.",
                consulta->forma.linhas, consulta->forma.colunas);
        return projectiles;
    }

    const auto battle = dynamic_cast<Battle*>(mOwner->GetScene());
    if (!battle) {
        SDL_Log("ConsultaAttack: sem cena Battle. Ataque cancelado.");
        return projectiles;
    }

    // A TABELA E O CAMPO DE JOGO, e nao a janela: GetPlayfieldBounds ja desconta
    // a faixa da nota no rodape. Usar a altura da janela poria a ultima linha
    // por baixo do HUD, onde o jogador nao pode ir - uma linha que so existe
    // para ser varrida, nunca para ser habitada.
    const SDL_FRect campo = battle->GetPlayfieldBounds();

    const bool porLinha = (consulta->eixo == ConsultaAttackParams::Eixo::Linha);

    // Onde a faixa fica, e quao grossa ela e.
    const int quantas   = porLinha ? consulta->forma.linhas : consulta->forma.colunas;
    const float origem  = porLinha ? campo.y : campo.x;
    const float extensao= porLinha ? campo.h : campo.w;

    const int indice = (consulta->indice < 0) ? 0
                     : (consulta->indice >= quantas) ? quantas - 1
                     : consulta->indice;

    const float inicioDaFaixa = Tabela::Inicio(origem, extensao, quantas, indice);
    const float espessura     = Tabela::Espessura(extensao, quantas);

    // A varredura corre no eixo PERPENDICULAR a faixa: uma linha e varrida da
    // esquerda para a direita, uma coluna de cima para baixo.
    const float comprimento = porLinha ? campo.w : campo.h;
    const float partida     = porLinha ? campo.x : campo.y;

    // DENTRO DA TELA, E NAO FORA DELA. A primeira versao disto nascia 48 px fora
    // da borda "para entrar deslizando", e com isso o telegrafo inteiro
    // acontecia onde ninguem podia ver: os projeteis esperavam o aviso
    // invisiveis e entravam ja disparados. O ataque ficava impossivel de ler, que
    // e exatamente o que o aviso existe para evitar.
    //
    // Encostados na borda eles sao visiveis a espera toda, e continuam fora do
    // caminho de quem joga no meio do campo.
    const float margem = 20.0f;
    const float ondeNasce = consulta->invertido ? (partida + comprimento - margem)
                                                : (partida + margem);

    const float sentido = consulta->invertido ? -1.0f : 1.0f;
    const Vector2 direcao = porLinha ? Vector2(sentido, 0.0f) : Vector2(0.0f, sentido);

    const int quantos = (params.numProjectiles > 0) ? params.numProjectiles : 1;
    projectiles.reserve(static_cast<size_t>(quantos));

    // Os projeteis se espalham pela ESPESSURA da faixa, nao pelo comprimento
    // dela: juntos, eles formam a parede que atravessa a tabela. Com um so, a
    // faixa teria um buraco por onde passar sem se mexer, e o ataque perderia o
    // sentido de "esta linha inteira foi lida".
    const float passo = (quantos > 1) ? espessura / static_cast<float>(quantos)
                                      : espessura / 2.0f;

    for (int i = 0; i < quantos; ++i) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // Meio do i-esimo quinhao da espessura: as pontas ficam a meio passo das
        // bordas, entao a parede cobre a faixa sem transbordar para a vizinha.
        const float atravessado = inicioDaFaixa + passo * (static_cast<float>(i) + 0.5f);

        projectile->SetPosition(porLinha ? Vector2(ondeNasce, atravessado)
                                         : Vector2(atravessado, ondeNasce));

        // Nasce PARADO e VISIVEL - este e o telegrafo. Ver o comentario da classe
        // sobre por que nao ha DeactivateBehavior aqui.
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2::Zero);
        }

        projectile->insertModifier<ActivateBehavior>(consulta->aviso,
                                                     direcao * params.projectileSpeed);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
