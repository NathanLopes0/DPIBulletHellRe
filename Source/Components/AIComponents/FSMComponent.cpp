//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include <iostream>
#include "FSMComponent.h"
#include "FSMState.h"

FSMComponent::FSMComponent(struct Actor *owner, int updateOrder)
    : Component(owner, updateOrder)
    ,mStateTime(.0f)
    ,mIsRunning(false)
    ,mCurrentState("start1")
{
    // Create a default start state
    new FSMState(this, "start1");
}

FSMComponent::~FSMComponent()
{
    for(const auto& state : mStates)
    {
        delete state.second;
    }

    mStates.clear();
}

void FSMComponent::Start(const std::string &startState)
{
    mIsRunning = true;
    SetState(startState);
}

void FSMComponent::SetState(const std::string &stateName)
{
    if(mStates.count(stateName) == 0) return;

    mStates[mCurrentState]->Exit();
    mStateTime = 0;
    mCurrentState = stateName;
    mStates[mCurrentState]->Start();

}

void FSMComponent::Update(float deltaTime)
{
    if (!mIsRunning) return;

    mStateTime += deltaTime;

    mStates[mCurrentState]->Update(deltaTime);
    mStates[mCurrentState]->HandleStateTransition(mStateTime);

}

void FSMComponent::AddState(std::string &stateName, FSMState *state)
{

    mStates.insert(std::make_pair(stateName,state));

}