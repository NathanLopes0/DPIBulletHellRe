//
// A fabrica de projetil de chefe, unica, dirigida por dados.
//

#pragma once

#include <memory>

#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/ProjetilDeChefe.h"
#include "../../../../Attacks/ProjeteisDeChefe.h"

/**
 * @class FabricaDeProjetil
 * @brief Monta projeteis de chefe conforme uma DescricaoDeProjetil.
 *
 * SUBSTITUIU SEIS FABRICAS (Salles capivara, Salles lista duplamente, Julio
 * dados, Ricardo arduino, Andre grafos, Andre baloes). As seis eram a mesma
 * classe copiada: Acquire, Release e Prewarm identicos linha por linha - cerca de
 * setenta linhas cada - e um createProjectile que diferia apenas nos campos que
 * hoje estao em Assets/Attacks/projeteis.json.
 *
 * Copia nao e so volume: cada uma era uma chance de esquecer uma linha, e TRES
 * esqueceram. A dos baloes registrava tres animacoes e nao escolhia nenhuma,
 * entao todo projetil criado pelo Prewarm era desenhado com animacao vazia; a da
 * capivara apontava para um atlas que nao existe (passou despercebido porque o
 * campo nunca era lido); duas declaravam mSpritePath/mDataPath proprios, que
 * SOMBREAVAM os da classe base. Com um lugar so, um conserto conserta todos.
 *
 * O POOL continua um por tipo de projetil, como antes: mPool e membro desta
 * instancia, e o chefe cria uma instancia por nome registrado. Nao era preciso um
 * tipo C++ distinto para isso.
 *
 * Cada instancia guarda a DESCRICAO POR VALOR. Isso importa: as descricoes vivem
 * num mapa global na ponte, lido uma vez do arquivo, e uma referencia para dentro
 * dele seria um ponteiro para algo que o chefe nao controla.
 */
class FabricaDeProjetil : public ProjectileFactory {

public:

    /**
     * @param descricao Como montar este projetil. Os caminhos de sprite e atlas
     * dentro dela precisam vir JA RESOLVIDOS para caminho absoluto - quem faz
     * isso e a ponte, que conhece Caminhos::Asset. Esta classe nao resolve
     * caminho para nao precisar conhecer o sistema de arquivos.
     * @param nome O nome do projetil, so para as mensagens de log dizerem qual
     * deles falhou. Sem ele, quatro chefes com problema dariam a mesma frase.
     */
    FabricaDeProjetil(DescricaoDeProjetil descricao, std::string nome);

    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

    [[nodiscard]] const DescricaoDeProjetil& Descricao() const { return mDescricao; }

private:

    DescricaoDeProjetil mDescricao;
    std::string mNome;

    ProjectilePool<ProjetilDeChefe> mPool;
};
