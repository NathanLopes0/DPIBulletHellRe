// From Game Programming in C++ by Sanjay Madhav Copyright (C) 2017 Sanjay Madhav. All rights
// reserved.

#include "Random.h"

#include "../Regras/Variacao.h"

void Random::Init()
{
	// O random_device sozinho pode se repetir (Variacao::SementeNova).
	Random::Seed(Variacao::SementeNova());
}

void Random::Seed(unsigned int seed)
{
	sGenerator.seed(seed);
}

float Random::GetFloatRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(sGenerator);
}

int Random::GetIntRange(int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(sGenerator);
}

std::mt19937 Random::sGenerator;
