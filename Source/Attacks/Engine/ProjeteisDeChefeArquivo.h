//
// A ponte entre os tipos de projetil lidos de arquivo e o motor.
//

#pragma once

#include <string>

class Boss;

/**
 * @brief Registra no chefe todos os projeteis do seu conjunto em
 * Assets/Attacks/projeteis.json.
 *
 * E A UNICA maneira de um chefe ganhar projeteis. As seis fabricas em C++ que
 * faziam isso (SallesProjectile1Factory e companhia) foram removidas: eram a
 * mesma classe copiada seis vezes, diferindo so nos campos que o arquivo agora
 * descreve.
 *
 *     void SallesFactory::ConfigureComponents(Boss* boss) {
 *         // ... desenho e colisor do chefe ...
 *         if (!RegistrarProjeteisDeArquivo(boss, "salles")) {
 *             SDL_Log("ERRO CRITICO: ...");   // o chefe fica sem projetil
 *         }
 *     }
 *
 * O arquivo e lido UMA vez, na primeira chamada.
 *
 * Devolve FALSE quando o conjunto nao existe ou ficou sem nenhum projetil
 * utilizavel. Um chefe sem projetil ainda monta e ainda anda, mas os ataques dele
 * nao encontram a fabrica que pedem e cada disparo vira uma linha de log - ou
 * seja, o chefe fica inofensivo. Quem impede isso de chegar ao jogo e
 * tests/test_arquivos_de_dados.cpp, que le este arquivo junto com fases.json e
 * com os atlas de verdade e quebra a suite se um ataque pedir um projetil que nao
 * existe, ou se uma animacao citar um quadro que a folha nao tem.
 *
 * O registro e PARCIAL de proposito, diferente do de fases: um projetil com
 * problema e descartado e os outros do mesmo chefe continuam valendo. Fases pela
 * metade deixariam o chefe congelado numa delas, o que parece bug de jogo; um
 * projetil a menos afeta apenas os ataques que pediam aquele nome.
 */
bool RegistrarProjeteisDeArquivo(Boss* boss, const std::string& nomeDoConjunto);
