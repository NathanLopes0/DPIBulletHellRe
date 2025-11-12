// FSMComponent.cpp (VERSÃO REFATORADA COM RAII E unique_ptr)

#include "FSMComponent.h"
#include "FSMState.h"
#include "../../Actors/Actor.h"
#include "SDL_log.h"

// 2. Construtor "limpo". Não cria estados, apenas inicializa.
FSMComponent::FSMComponent(Actor *owner, int updateOrder)
    : Component(owner, updateOrder),
      mIsRunning(false),
      mCurrentState(std::string()), // Começa sem estado
      mStateTime(0.0f)
{
    // A Factory será responsável por registrar os estados.
}

//    Temos States e forward declaration de State no .h, então colocar o destrutor aqui:
//    Como inclui FSMState.h, o compilador sabe como
//    destruir o std::map<string, unique_ptr<FSMState>> corretamente.
FSMComponent::~FSMComponent() = default;

/**
 * @brief Registra um novo estado, transferindo a posse para o mapa.
 */
void FSMComponent::RegisterState(std::unique_ptr<FSMState> state)
{
    std::string stateName = state->GetName();
    // Usa std::move para transferir a posse do unique_ptr para o mapa
    mStates[stateName] = std::move(state);
}

/**
 * @brief Inicia a FSM no estado fornecido.
 */
void FSMComponent::Start(const std::string &startState)
{
    mIsRunning = true;
    SetState(startState);

}

/**
 * @brief Tenta transicionar para um novo estado.
 */
void FSMComponent::SetState(const std::string &stateName)
{
    // 1. Tenta encontrar o estado de destino
    auto newState = GetState(stateName);
    if (newState == nullptr)
    {
        SDL_Log("FSMComponent::SetState: Nao foi possivel encontrar o estado '%s'", stateName.c_str());
        return;
    }

    // 2. Se já estivermos em um estado, chama o Exit() dele
    if (!mCurrentState.empty())
    {
        if (auto oldState = GetState(mCurrentState))
        {
            oldState->Exit();
        }
    }

    // 3. Define o novo estado
    mCurrentState = stateName;
    mStateTime = 0.0f;

    if (mOnStateChanged) {
        // Pega a duração do novo estado
        const float duration = newState->GetDuration();

        // Envia a duração para o "escutador"
        mOnStateChanged(duration);
    }

    newState->Start(); // Chama o Start() do novo estado
}

/**
 * @brief Atualiza o estado ativo.
 */
void FSMComponent::Update(float deltaTime)
{
    if (!mIsRunning || mCurrentState.empty())
    {
        return;
    }

    // Encontra o ponteiro para o estado atual
    if (auto currentState = GetState(mCurrentState))
    {
        mStateTime += deltaTime;
        currentState->Update(deltaTime);
        currentState->HandleStateTransition(mStateTime);
    }
    else
    {
        // Failsafe: se o estado atual for inválido, para a FSM
        SDL_Log("FSMComponent::Update: Estado atual '%s' e invalido!", mCurrentState.c_str());
        mIsRunning = false;
    }
}

/**
 * @brief Helper privado para buscar um estado no mapa com segurança.
 */
FSMState* FSMComponent::GetState(const std::string& stateName)
{
    auto iter = mStates.find(stateName);
    if (iter != mStates.end())
    {
        // .get() retorna o ponteiro bruto de dentro do unique_ptr
        return iter->second.get();
    }
    return nullptr;
}

void FSMComponent::SetOnStateChanged(std::function<void(float newDuration)> callback) {
    mOnStateChanged = std::move(callback);
}