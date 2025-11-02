//
// Created by Lucas N. Ferreira on 26/10/23.
//

#pragma once

#include <string>

class FSMComponent;

/**
 * @interface FSMState
 * @brief Classe base para todos os "estados" de uma Máquina de Estados Finitos.
 * Define o contrato (Start, Update, Exit) que todo estado deve ter.
 */
class FSMState
{
public:
    /**
     * @brief Construtor do estado.
     * @param fsm O componente FSM "dono" deste estado (ponteiro observador).
     * @param name O nome de identificação deste estado (ex: "StateOne").
     */
    FSMState(FSMComponent *fsm, const std::string &name);
    /**
     * @brief Destrutor virtual.
     * Essencial para garantir que os destrutores das classes filhas
     * sejam chamados corretamente.
     */
    virtual ~FSMState() = default;

    /** @brief Chamado uma vez quando a FSM entra neste estado. */
    virtual void Start();

    /** @brief Chamado a cada frame enquanto este estado está ativo. */
    virtual void Update(float deltaTime);

    /** @brief Chamado uma vez quando a FSM sai deste estado. */
    virtual void Exit();

    /**
     * @brief Lógica interna do estado para checar se deve transicionar e o que fazer na transição.
     */
    virtual void HandleStateTransition(float stateTime);

    [[nodiscard]] const std::string &GetName() const { return mName; }

protected:
    FSMComponent *mFSM; // Um atalho para reconhecer seu componente dono
    std::string mName;
};
