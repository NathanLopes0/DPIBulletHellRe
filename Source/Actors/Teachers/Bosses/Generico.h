//
// O professor generico da fase tutorial (INF 191 - Introducao a Ciencia da Computacao).
//

#pragma once

#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Tutorial.h"

class DrawLinhasComponent;
class EtiquetaDeCodigo;

/// A primeira prova de quem acabou de criar o perfil. Sem texto: o que cada etapa ensina e desenho
/// (Tutorial.h) - o manche ao lado do aluno enquanto a chuva cai, o Botao 1 e a linha ate o
/// professor, o "+" que voa ate a barra da nota a cada acerto e o "-" a cada tiro levado, o Botao 4
/// segurado nos aneis apertados. No fim, o visto verde, e a prova se encerra sozinha (Battle: o
/// tutorial nao tem tela de resultado nem vai para a ficha).
class Generico : public Boss {
public:
    explicit Generico(Scene* scene);
    ~Generico() override = default;

    void OnUpdate(float deltaTime) override;
    void AoTerminarABatalha() override;

    static constexpr const char* kFase = "StateOne";

protected:
    /// Na etapa da nota que sobe e desce, o leque sai mirado no aluno.
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    [[nodiscard]] float Tempo() const;

    // ---- O desenho ----
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 120;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 20);
    void Disco(Vector2 centro, float raio, SDL_Color cor);
    void Seta(Vector2 de, Vector2 ate, SDL_Color cor, float ponta = 9.0f);
    /// Um algarismo de sete segmentos (o 1 e o 4 dos botoes), de 'altura' pixels.
    void Algarismo(char d, Vector2 centro, float altura, SDL_Color cor);

    /// O manche, com a bola apontando para 'lado'.
    void Manche(Vector2 centro, Vector2 lado);
    /// Um botao do painel com o numero dele, aceso quando apertado.
    void Botao(Vector2 centro, char numero, bool apertado);
    /// O "+" verde ou o "-" vermelho.
    void Sinal(Vector2 centro, bool mais, float tamanho);
    void Visto(Vector2 centro, float tamanho);

    // ---- A nota que sobe e desce ----
    struct Voo {
        Vector2 de;
        bool mais = true;
        float idade = 0.0f;
    };
    std::vector<Voo> mVoos;
    float mNotaAntes = -1.0f;
    /// O ultimo sinal na barra: a seta para cima ou para baixo ao lado da ponta dela.
    bool mUltimoMais = true;
    float mSetaNaBarra = 0.0f;
    void AcompanharANota(float deltaTime);

    // ---- As teclas, so fora do gabinete ----
    std::vector<EtiquetaDeCodigo*> mTeclas;
    EtiquetaDeCodigo* Tecla(size_t k);
    void EsconderTeclas();
    void EscreverTecla(size_t& k, const std::string& texto, Vector2 onde);

    bool mEtapasSemTexto = false;
    bool mEncerrou = false;
    bool mLimpouNoFim = false;
};
