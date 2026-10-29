//
// Quanto dura uma fase.
//

#pragma once

/**
 * CAMADA PURA.
 *
 * TODA FASE DE TODO CHEFE DURA kDuracao SEGUNDOS - as tres e a repescagem.
 * Era 17 ate o commit 159; virou 20 quando as fases ficaram dinamicas o
 * bastante para pedir mais tempo.
 *
 * O NUMERO MORA AQUI, E NAO SO EM CADA fases.json, porque ha quem conte com
 * ele: o leitor (o padrao de uma fase sem "duracao"), o relogio do HUD, e os
 * modulos que dividem a fase em pedacos - os comandos do fluxograma do Andre
 * (Condicional.h), as etapas da cascata da Maria Lucia (Cascata.h), os
 * problemas da prova no BOCA (Prova.h), a janela da capivara do Salles
 * (Capivara.h). Um teste confere que toda fase de todo fases.json dura isto, e
 * que as etapas de quem tem etapas cobrem a fase inteira.
 */
namespace Fase {
    constexpr float kDuracao = 20.0f;

    /// A EXCECAO: o TCC do Hugo e uma batalha maior. Cada etapa tem um escudo
    /// a abrir (Objetivo.h) antes de o Hugo tomar dano, e 20 s davam uma
    /// janela so. Nenhum modulo do Hugo corta a fase em pedacos, e o relogio do
    /// HUD le a duracao de cada fase (Battle::OnFaseTrocada).
    constexpr float kDuracaoDoTcc = 30.0f;
}
