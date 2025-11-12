
#pragma once

#include "../Component.h"
#include <map>
#include <string>
#include <memory>
#include <functional>

class FSMState;

/**
 * @class FSMComponent
 * @brief Gerencia e executa uma Máquina de Estados Finitos.
 * O componente é "dono" de todos os estados (StateOne, StateTwo, etc.)
 * que são registrados nele.
 */
class FSMComponent : public Component {

public:
    explicit FSMComponent(Actor* owner, int updateOrder = 50);
    ~FSMComponent() override;

    /** @brief Inicia a FSM em um estado específico. */
    void Start(const std::string &startState);

    /** @brief Atualiza o estado ativo atual. */
    void Update(float deltaTime) override;

    /**
     * @brief Registra um novo estado na FSM, transferindo a posse.
     * @param state Um ponteiro único para o estado (ex: std::make_unique<StateOne>(...))
     */
    void RegisterState(std::unique_ptr<FSMState> state);

    /**
     * @brief Tenta transicionar para um novo estado.
     * @param stateName O nome do estado para o qual transicionar.
     */
    void SetState(const std::string &stateName);

    // --- Getters (Sinalizadores para o Boss) ---

    /** @brief Retorna o nome do estado ativo atual. */
    [[nodiscard]] const std::string& GetStateName() const { return mCurrentState; }

    /** @brief Checa se a FSM está no estado X. */
    [[nodiscard]] bool IsInState(const std::string &stateName) const { return mCurrentState == stateName; }

    /** @brief Retorna o tempo (em seg) decorrido no estado atual. */
    [[nodiscard]] float GetStateTime() const { return mStateTime; }


    /**
     * @brief Registra um callback para ser chamado
     * toda vez que o estado for trocado com sucesso.
     * @param callback A função a ser chamada, que receberá a DURAÇÃO do novo estado.
     */
    void SetOnStateChanged(std::function<void(float newDuration)> callback);

private:
    // --- Membros Internos ---

    /** @brief Busca um estado no mapa pelo nome. Retorna nullptr se não achar. */
    [[nodiscard]] FSMState* GetState(const std::string& stateName);

    bool mIsRunning;
    std::string mCurrentState;
    float mStateTime;

    // O componente agora é DONO de seus estados guardados.
    std::map<std::string, std::unique_ptr<FSMState>> mStates;

    // O callback que será chamado na troca de estado.
    std::function<void(float newDuration)> mOnStateChanged;

};