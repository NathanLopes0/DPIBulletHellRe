//
// O chefe do TCC - Trabalho de Conclusao de Curso.
//

#pragma once

#include "../Boss.h"

/**
 * @class Hugo
 * @brief O professor do TCC, a ultima materia do curso.
 *
 * E A UNICA BATALHA QUE O ALUNO NAO LUTA SOZINHO: antes dela ele escolhe um
 * ORIENTADOR entre os professores das materias em que foi melhor
 * (Orientacao.h), e o orientador luta do lado dele.
 *
 * AS FASES SAO AS ETAPAS DE UM TCC - proposta, desenvolvimento, defesa e
 * correcoes -, em Assets/Attacks/hugo/fases.json. Por enquanto elas sao
 * PROVISORIAS, feitas so com as estrategias que ja existem: as monografias e
 * as perguntas de banca de verdade vem depois, e o que nao couber em dados
 * mora aqui.
 */
class Hugo : public Boss {
public:
    explicit Hugo(Scene* scene);
    ~Hugo() override = default;
};
