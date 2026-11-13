//
// A fase 1 do Daniel: os dados - a coleta e a preparacao.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A fase 1 do Daniel e o comeco de toda analise: juntar os dados e arruma-los. O campo e um grafico
/// de dispersao do catalogo dos monstrinhos - o x e o ataque, o y a defesa -, e cada dado e um tiro
/// que voa de uma das fontes (o sensor, a planilha, a API) ate o lugar dele no grafico, e fica:
///   - na coleta, o grafico enche: tres especies, uma cor cada, e no meio delas o ruido - o outlier,
///     longe de tudo, e o nulo, oco, sem valor;
///   - na limpeza, o outlier e achado pelo z-score e estoura, e o nulo e imputado: voa ate a media;
///   - na normalizacao, o grafico inteiro se transforma de uma vez - o min-max espreme tudo numa
///     caixa, a padronizacao abre tudo em volta do meio. Antes de cada uma, uma seta sai de cada dado
///     ate onde ele vai parar: quem joga olha para onde os pontos VAO, e nao para onde estao.
namespace Preparacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 3> kEtapas = {"Coleta", "Limpeza", "Normalizacao"};
    constexpr std::array<float, 3> kDuracoes = {7.0f, 6.0f, 7.0f};
    constexpr const char* kGatilhoDaFonte = "fonte";
    constexpr const char* kGatilhoDoDado = "dado";
    constexpr const char* kGatilhoDoEstouro = "estouro";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- O catalogo ----

    /// As especies do catalogo: as cores dos dados (fogo, agua e planta). As caras de mais duas
    /// (eletrico e pedra) servem aos grupos da fase 2.
    constexpr int kEspecies = 3;
    constexpr std::array<const char*, 5> kCaras = {"Fogo", "Agua", "Planta", "Eletrico", "Pedra"};
    constexpr const char* kCaraDoOutlier = "Outlier";
    constexpr const char* kCaraDoNulo = "Nulo";
    /// As tres fontes, na ordem: o sensor (a esquerda), a planilha (no alto) e a API (a direita).
    constexpr std::array<const char*, 3> kFontes = {"Sensor", "Planilha", "Api"};

    enum class Tipo { Comum, Outlier, Nulo };

    struct Dado {
        Vector2 onde;            ///< no grafico, de 0 a 1: x e o ataque, y a defesa
        Tipo tipo = Tipo::Comum;
        int especie = 0;
        int fonte = 0;
    };

    /// Os centros das especies no grafico, longe das bordas e um do outro.
    std::array<Vector2, kEspecies> Especies(Variacao::Sorteio& s);

    /// O espalhamento de cada especie em volta do centro dela (o desvio, no grafico).
    constexpr float kEspalhamento = 0.07f;

    /// Um dado coletado: perto do centro de uma especie sorteada. Uma fracao sai outlier - num canto
    /// do grafico, longe de todas - e outra, nula.
    Dado Coletar(Variacao::Sorteio& s, const std::array<Vector2, kEspecies>& especies, float fracaoOutlier,
                 float fracaoNula);

    // ---- As contas da preparacao ----

    Vector2 Media(const std::vector<Vector2>& ps);
    /// O desvio padrao de cada eixo (o da populacao). Sem pontos, zero.
    Vector2 Desvio(const std::vector<Vector2>& ps);

    struct Caixa {
        Vector2 de;
        Vector2 ate;
    };
    /// A menor caixa com todos os pontos.
    Caixa Limites(const std::vector<Vector2>& ps);

    /// O min-max: cada ponto vai para o mesmo lugar relativo, dentro de 'destino'. Um eixo sem
    /// largura vai para o meio do destino.
    std::vector<Vector2> MinMax(const std::vector<Vector2>& ps, const Caixa& destino);

    /// A padronizacao (o z-score): a media vai para 'centro', e um desvio passa a medir 'escala' em
    /// cada eixo.
    std::vector<Vector2> Padronizar(const std::vector<Vector2>& ps, Vector2 centro, Vector2 escala);

    /// Os outliers: quem tem o z-score maior que 'limite' em algum eixo.
    std::vector<bool> Outliers(const std::vector<Vector2>& ps, float limite);
    constexpr float kLimiteDoOutlier = 2.0f;

    /// A caixa do min-max num grafico de 0 a 1: o canto 'canto' (0: embaixo a esquerda, 1: embaixo a
    /// direita, 2: no alto a esquerda, 3: no alto a direita), com 'lado' de lado.
    Caixa CaixaNoCanto(int canto, float lado);
    /// O canto do chao em que o aluno esta (0 ou 1): e para la que o min-max manda o grafico.
    int CantoDoAluno(float xNoGrafico);
    /// O lado da caixa do min-max, no grafico: mais da metade - quem esta nela tem de sair dela.
    constexpr float kLadoDaCaixa = 0.55f;

    // ---- O ritmo ----

    /// A seta antes de cada transformacao, e o quanto ela demora.
    struct Degrau {
        float entreColetas = 0.12f;       ///< um dado a cada, das tres fontes juntas
        float velocidadeDaColeta = 280.0f;
        int maisDados = 56;
        int resistencia = 3;              ///< os tiros que apagam um dado (uma linha da tabela)
        float fracaoOutlier = 0.12f;
        float fracaoNula = 0.12f;
        float aviso = 0.75f;              ///< a seta, antes de cada transformacao
        float transformacao = 0.75f;      ///< o tempo de os pontos irem para o lugar novo
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// As transformacoes da normalizacao, na ordem, e quando cada uma comeca (a seta acende), em
    /// segundos desde o comeco da fase.
    enum class Transformacao { MinMax, Padronizacao };
    struct Passo {
        Transformacao qual = Transformacao::MinMax;
        float quando = 0.0f;
    };
    constexpr std::array<Passo, 4> kPassos = {{{Transformacao::MinMax, 13.2f},
                                               {Transformacao::Padronizacao, 14.9f},
                                               {Transformacao::MinMax, 16.6f},
                                               {Transformacao::Padronizacao, 18.3f}}};
    /// Na limpeza: quando os outliers estouram e quando os nulos sao imputados.
    constexpr float kEstouro = 7.6f;
    constexpr float kImputacao = 9.8f;
}
