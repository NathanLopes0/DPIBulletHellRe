//
// O fundo que as telas de menu compartilham.
//

#pragma once

class Scene;

/**
 * @brief Poe o CORREDOR DO DPI como fundo da cena, escurecido.
 *
 * A foto e a mesma do menu principal - o corredor de verdade do departamento,
 * que e onde o gabinete vai ficar. Aqui ela entra bem escura e com um veu por
 * cima: a foto tem tijolo vermelho e janela clara, e texto branco solto sobre
 * ela some em metade da largura.
 *
 * ESTA FUNCAO EXISTE PARA NAO HAVER SEIS COPIAS DA RECEITA. Sao seis telas com
 * o mesmo fundo (identificacao, criacao de personagem, selecao de fases,
 * ranking, opcoes e atracao), e a primeira versao disto era um metodo privado
 * da selecao de fases. Copiar valores de escurecimento para as outras cinco
 * seria o erro de sempre: no dia em que um deles mudasse, mudaria em uma.
 *
 * Chame no inicio do Load(): os dois atores usam ordem de desenho BAIXA, mas
 * entrar antes deixa a leitura da cena na mesma ordem em que ela aparece.
 */
void CriarFundoDoCorredor(Scene& cena);
