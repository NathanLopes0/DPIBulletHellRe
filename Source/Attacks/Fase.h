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
}
