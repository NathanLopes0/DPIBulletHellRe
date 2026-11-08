//
// Regras de configuracao de projetil, lidas de texto JSON.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "PathAim.h"

/// Este arquivo nao abre arquivo, nao escreve log, nao conhece Projectile nem SDL. Ele so
/// transforma texto em descricoes.

/// O que inserir num projetil, descrito em dados.
struct DescricaoDeBehavior {
    std::string tipo;               ///< "Path", "Tracking", "SlowDown", ...

    std::string forma;              ///< Path: nome da forma (codigo ou arquivo)

    /// Path: nome de uma familia de formas.
    std::string formaPorIndice;
    std::string mira;               ///< Path: AlinharComVelocidade | MirarNoJogador | MirarPrevendo
    float parametroDaMira = 0.0f;   ///< Path: segundos de antecipacao, para MirarPrevendo

    float velocidade = 0.0f;        ///< Path
    float atraso = 0.0f;            ///< todos

    int   pararNoPonto = -1;        ///< Path: indice do waypoint
    float pararPor = 0.0f;          ///< Path: segundos parado
    float forca = 0.0f;             ///< Tracking
    float duracao = 0.0f;           ///< Tracking, Wobble
    float amplitude = 0.0f;         ///< Wobble: graus
    float frequencia = 0.0f;        ///< Wobble

    /// Ancorar: o lugar da janela em relacao ao corpo do chefe mais perto, em pixels.
    float dx = 0.0f;
    float dy = 0.0f;
    float fator = 1.0f;             ///< Accelerate, SlowDown
    Vector2 velocidadeInicial;      ///< Activate

    /// Amarrar e Promessa: a animacao que o projetil passa a ter.
    std::string escreve;

    /// Promessa: a parte das promessas que resolvem, de 0 a 1.
    float resolve = 0.5f;

    /// Amarrar: o comprimento da corda, em pixels.
    float comprimento = 0.0f;
    /// Amarrar: "horario" ou "antihorario", como se ve na tela.
    std::string sentido = "horario";

    RitmoDeCiclos ritmo;            ///< MiraPeriodica, PulsoDeVelocidade
    float moduloInvestida = 0.0f;   ///< PulsoDeVelocidade
    float moduloPausa = 0.0f;       ///< PulsoDeVelocidade
};

/// Uma regra: a quem se aplica, e o que fazer com quem se encaixa. O formato espelha a divisao da
/// fase 1 - `motion` no singular, `modifiers` no plural.
struct Regra {
    enum Condicao {
        Sempre,    ///< vale para todo projetil da rajada
        Chance,    ///< sorteio por projetil
        Indices,   ///< posicoes especificas dentro da rajada
        Pares,     ///< indices 0, 2, 4...
        Impares,   ///< indices 1, 3, 5...

        /// So vale enquanto o aluno esta acorrentado. E a unica condicao que nao olha para a
        /// rajada, e sim para o estado.
        AlvoAcorrentado
    };

    Condicao condicao = Sempre;
    float chance = 1.0f;
    std::vector<int> indices;

    bool temMotion = false;
    DescricaoDeBehavior motion;
    std::vector<DescricaoDeBehavior> modifiers;

    std::string animacao;   ///< vazio = nao mexe na animacao

    /// Se o sprite gira para acompanhar a direcao do voo. Ausente = nao mexe, e o projetil fica com
    /// o que a fabrica dele pediu em projeteis.json.
    std::optional<bool> rotacionar;

    /// Multiplicador de tamanho do sprite E do colisor. Ausente = nao mexe.
    std::optional<float> escala;

    /// O posto em que o projetil entra ao nascer (PostoExiste, BossProjectile::DefinirPosto). Vazio
    /// = nao mexe.
    std::string posto;
};

/// Conjuntos de regras lidos de um arquivo, cada um com um nome.
struct RegrasLidas {
    std::map<std::string, std::vector<Regra>> conjuntos;
    std::vector<std::string> problemas;
};

/// O tipo nomeado e uma Motion? (Path, Tracking, Wobble, MiraPeriodica, Amarrar)
bool EhMotion(const std::string& tipo);

/// O tipo nomeado e um Modifier? (Accelerate, SlowDown, Activate, Deactivate, PulsoDeVelocidade,
/// Promessa)
bool EhModifier(const std::string& tipo);

/// O posto nomeado existe?
bool PostoExiste(const std::string& nome);

/// Todos os postos. A lista com quem ocupa cada um esta em Documentacao/arquivos-de-dados.md, e um
/// teste confere que ela nao esquece nenhum.
const std::vector<std::string>& NomesDosPostos();

/// Le conjuntos de regras a partir do texto de um arquivo JSON. Pura.
RegrasLidas LerRegras(const std::string& textoJson);

/// Le uma lista de regras a partir do texto de um array JSON. Pura.
std::vector<Regra> LerListaDeRegras(const std::string& textoDaLista,
                                    const std::string& onde,
                                    std::vector<std::string>& problemas);

/// A regra vale para o projetil de indice 'indice'? Pura.
bool RegraSeAplica(const Regra& regra, int indice, float sorteio,
                   bool alvoAcorrentado = false);
