//
// Created by Claude on 08/10/2026.
//

#pragma once

#include <string>
#include <vector>

/// O que muda quando o jogo roda no gabinete do departamento em vez de na janela de quem esta
/// desenvolvendo.
namespace Gabinete {

/// Segundos parado antes de voltar ao menu, quando --arcade nao diz outro valor.
constexpr float kOciosidadePadrao = 60.0f;

/// Segundos que o operador segura a combinacao de saida. Sair precisa ser
/// deliberado: um toque nao pode derrubar o jogo no meio do corredor.
constexpr float kSegurarParaSair = 2.0f;

/// O que veio da linha de comando.
struct Configuracao {
    bool arcade = false;

    /// Segundos parados ate voltar ao menu. Zero desliga - e o padrao fora do gabinete, senao o
    /// jogo voltaria ao menu sozinho enquanto se le codigo.
    float ociosidade = 0.0f;

    /// O texto de --condicao, como veio: as mecanicas que um experimento desliga
    /// (Experimento::Ler). Vazio: tudo ligado.
    std::string condicao;
};

/// Le a linha de comando.
Configuracao Ler(const std::vector<std::string>& argumentos);

/// Conta quanto tempo uma condicao fica verdadeira sem interrupcao.
class Contagem {
public:
    explicit Contagem(const float limite = 0.0f) : mLimite(limite) {}

    /// Passa um quadro.
    bool Passou(float deltaTime, bool continua);

    void Reiniciar() { mAcumulado = 0.0f; }

    /// Se esta contagem pode esgotar. Limite zero nunca esgota.
    [[nodiscard]] bool Ligada() const { return mLimite > 0.0f; }

    [[nodiscard]] float Acumulado() const { return mAcumulado; }

private:
    float mLimite;
    float mAcumulado = 0.0f;
};

} // namespace Gabinete
