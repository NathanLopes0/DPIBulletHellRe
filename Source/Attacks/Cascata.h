//
// O modelo cascata da fase 1 da Maria Lucia.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// A fase 1 da Maria Lucia e o modelo cascata: cinco caixas em escada, uma por etapa, e o que uma
/// etapa entrega desce para a seguinte como agua.
///   - A cachoeira: da caixa da etapa da vez cai, o tempo todo, o que ela produz - post-its nos
///     requisitos, classes no projeto, codigo na implementacao, bugs nos testes, divida na
///     manutencao -, cada vez mais rapido.
///   - A mudanca de escopo: de tempos em tempos o cliente pede outra coisa. Na cascata nao se volta
///     atras, entao o pedido vai para a primeira caixa e desce a escada inteira, caixa por caixa,
///     virando classe, codigo, bug e divida, e em cada caixa que passa paga o custo de muda-la ali -
///     1, 2, 3, 5 e 8 tiros, a curva de Boehm: o erro achado tarde custa mais. Na caixa da etapa da
///     vez ele estoura. O aluno pode derrubar o pedido no caminho: quanto mais cedo, mais barato.
///
/// Nada disso depende do que esta em campo: o Ponto Extra limpa a tela, e a cachoeira e os pedidos
/// continuam vindo no tempo deles.
namespace Cascata {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaCascata = "StateOne";
    constexpr std::array<const char*, 5> kEtapas = {"Requisitos", "Projeto", "Implementacao", "Testes", "Manutencao"};
    constexpr const char* kGatilhoDaPlaca = "placa";
    constexpr const char* kGatilhoDaQueda = "queda";
    constexpr const char* kGatilhoDoPedido = "pedido";
    constexpr const char* kGatilhoDoCusto = "custo";
    constexpr const char* kGatilhoDoEstouro = "estouro";

    /// A duracao de cada etapa: a mesma das "etapas" de fases.json e dos degraus da escada (um teste
    /// confere).
    constexpr float kDuracaoDaEtapa = Fase::kDuracao / static_cast<float>(kEtapas.size());

    /// Em que etapa se esta no instante 't' da fase: 0 a 4, e 5 depois da ultima.
    int EtapaNoTempo(float t);

    /// O que a etapa produz - a animacao do artefato em projeteis.json: "Pedido", "Classe",
    /// "Codigo", "Bug" ou "Debito".
    const char* CaraDaEtapa(int etapa);

    // ---- A cachoeira ----

    /// Segundos entre duas quedas da caixa da etapa: a cascata acelera ladeira abaixo.
    float EntreQuedas(int etapa, Dificuldade::Nivel n);

    /// A velocidade de queda do artefato da etapa, em px/s.
    float VelocidadeDaQueda(int etapa);

    /// A agua escorre pelas bordas da caixa: duas cortinas, a kBordaDaQueda do meio dela (mais ate
    /// kEspalhoDaQueda), e um vao no meio - embaixo da professora, para quem quer atirar nela.
    constexpr float kBordaDaQueda = 62.0f;
    constexpr float kEspalhoDaQueda = 22.0f;

    /// Quantos tiros o anel do custo solta na caixa 'etapa': o custo, ate kMaisNoAnel.
    constexpr int kMaisNoAnel = 6;
    int TirosDoCusto(int etapa);

    // ---- A mudanca de escopo ----

    constexpr float kPrimeiraMudanca = 1.0f;
    constexpr float kVelocidadeDoPedido = 340.0f;
    /// Quanto a mudanca leva de uma caixa a seguinte, descendo a escada.
    constexpr float kPorCaixa = 0.55f;

    /// O custo de mudar na etapa 'etapa' o que nasceu nos requisitos: 1, 2, 3, 5, 8 - a curva de
    /// Boehm, nos numeros do planning poker.
    int Custo(int etapa);

    /// O placar quando a mudanca estoura: "Mudança nos Testes: custo ×5".
    std::string PlacarDaMudanca(int etapa);

    /// De onde vem um pedido: de um dos lados do campo, numa altura sorteada da metade de baixo - o
    /// cliente esta do lado de fora.
    Vector2 OrigemDoPedido(std::uint32_t sorteio, float largura, float altura);

    /// A caixa de cada etapa fica kAbaixoDoDegrau abaixo do degrau dela: longe o bastante para a
    /// professora, de pe no degrau, nao a cobrir.
    constexpr float kAbaixoDoDegrau = 70.0f;

    /// O que o Dificil muda.
    struct Degrau {
        float entreMudancas = 2.4f;
        float quedaMaisRapida = 1.0f;   ///< o que multiplica a velocidade da cachoeira
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
