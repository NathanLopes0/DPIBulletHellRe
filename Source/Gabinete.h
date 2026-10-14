//
// Created by Claude on 08/10/2026.
//

#pragma once

#include <string>
#include <vector>

/**
 * @brief O que muda quando o jogo roda NO GABINETE do departamento em vez de na
 * janela de quem esta desenvolvendo.
 *
 * O gabinete nao tem teclado, nao tem mouse, fica ligado o dia inteiro num
 * corredor e ninguem fecha o jogo entre um aluno e o outro. Dai as tres regras
 * que este modulo descreve: a tela e cheia, ficar parado volta ao menu (senao o
 * proximo aluno encontra a tela do anterior, com o perfil dele carregado) e
 * sair do jogo exige uma combinacao segurada, porque "fechar a janela" nao
 * existe la.
 *
 * O PADRAO E A JANELA, de proposito: e assim que se desenvolve e e assim que o
 * CLion abre o jogo. Quem monta o gabinete passa --arcade no script que o abre.
 */
namespace Gabinete {

/// Segundos parado antes de voltar ao menu, quando --arcade nao diz outro valor.
constexpr float kOciosidadePadrao = 60.0f;

/// Segundos que o operador segura a combinacao de saida. Sair precisa ser
/// deliberado: um toque nao pode derrubar o jogo no meio do corredor.
constexpr float kSegurarParaSair = 2.0f;

/// O que veio da linha de comando.
struct Configuracao {
    bool arcade = false;

    /// Segundos parados ate voltar ao menu. ZERO DESLIGA - e o padrao fora do
    /// gabinete, senao o jogo voltaria ao menu sozinho enquanto se le codigo.
    float ociosidade = 0.0f;
};

/**
 * @brief Le a linha de comando.
 *
 * Recebe os argumentos JA SEM o nome do programa (argv[0]), para o teste poder
 * escrever Ler({"--arcade"}) sem um primeiro elemento de enfeite.
 *
 * Reconhece:
 *   --arcade              tela cheia, sem cursor, sem saida acidental
 *   --ociosidade <seg>    quanto tempo parado volta ao menu; 0 desliga
 *
 * O que nao for reconhecido e IGNORADO, e os argumentos seguintes continuam
 * valendo: um erro de digitacao no script do gabinete nao pode impedir o jogo
 * de abrir numa maquina sem teclado.
 */
Configuracao Ler(const std::vector<std::string>& argumentos);

/**
 * @brief Conta quanto tempo uma condicao fica verdadeira SEM INTERRUPCAO.
 *
 * As duas esperas do gabinete sao a mesma conta com a condicao trocada:
 * "ninguem mexeu" (volta ao menu) e "o operador esta segurando" (sai do jogo).
 * Uma classe so, para nao existirem duas contagens e so uma ser corrigida.
 */
class Contagem {
public:
    /// @param limite segundos ate esgotar. Zero ou negativo DESLIGA a contagem.
    explicit Contagem(const float limite = 0.0f) : mLimite(limite) {}

    /**
     * @brief Passa um quadro.
     * @param continua se a condicao vale NESTE quadro. Falso zera a conta.
     * @return true UMA vez, no quadro em que o tempo esgota.
     *
     * Ao esgotar, a contagem recomeca sozinha. Isso e o que faz quem chama nao
     * precisar reiniciar nada: sem o reinicio, uma espera esgotada responderia
     * true em todo quadro seguinte, e voltar ao menu seria pedido sessenta
     * vezes por segundo.
     */
    bool Passou(float deltaTime, bool continua);

    void Reiniciar() { mAcumulado = 0.0f; }

    /// @brief Se esta contagem pode esgotar. Limite zero nunca esgota.
    [[nodiscard]] bool Ligada() const { return mLimite > 0.0f; }

    [[nodiscard]] float Acumulado() const { return mAcumulado; }

private:
    float mLimite;
    float mAcumulado = 0.0f;
};

} // namespace Gabinete
