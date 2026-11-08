
#pragma once

#include "../Component.h"
#include <map>
#include <string>
#include <memory>
#include <functional>

class FSMState;

/// Gerencia e executa uma Máquina de Estados Finitos. O componente é "dono" de todos os estados
/// (StateOne, StateTwo, etc.) que são registrados nele.
class FSMComponent : public Component {

public:
    explicit FSMComponent(Actor* owner, int updateOrder = 50);
    ~FSMComponent() override;

    /// Inicia a FSM em um estado específico.
    void Start(const std::string &startState);

    /// Atualiza o estado ativo atual.
    void Update(float deltaTime) override;

    /// Registra um novo estado na FSM, transferindo a posse.
    void RegisterState(std::unique_ptr<FSMState> state);

    /// Tenta transicionar para um novo estado.
    void SetState(const std::string &stateName);

    // ---- Getters (Sinalizadores para o Boss) ----

    /// Retorna o nome do estado ativo atual.
    [[nodiscard]] const std::string& GetStateName() const { return mCurrentState; }

    /// Retorna o tempo (em seg) decorrido no estado atual.
    [[nodiscard]] float GetStateTime() const { return mStateTime; }

    /// Registra um callback para ser chamado toda vez que o estado for trocado com sucesso.
    void SetOnStateChanged(std::function<void(float newDuration)> callback);

private:
    // ---- Membros Internos ----

    /// Busca um estado no mapa pelo nome. Retorna nullptr se não achar.
    [[nodiscard]] FSMState* GetState(const std::string& stateName);

    bool mIsRunning;
    std::string mCurrentState;
    float mStateTime;

    // O componente é dono de seus estados guardados.
    std::map<std::string, std::unique_ptr<FSMState>> mStates;

    // O callback que será chamado na troca de estado.
    std::function<void(float newDuration)> mOnStateChanged;

};