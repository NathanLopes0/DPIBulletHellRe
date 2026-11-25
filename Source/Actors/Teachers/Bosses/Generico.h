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
    /// Cada acerto do aluno no professor conta para passar a etapa (Tutorial::Desempenho).
    void AoSerPerguntado() override;

    static constexpr const char* kFase = "StateOne";

protected:
    /// Na etapa da nota que sobe e desce, o leque sai mirado no aluno.
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

private:
    // ---- O relogio do tutorial ----

    /// Os segundos do tutorial, que saltam para a etapa seguinte quando o aluno ja mostrou o que a
    /// de agora ensina (Tutorial::PodeAvancar). O relogio da fase so anda; este, nao.
    float mTempo = 0.0f;
    Tutorial::Etapa mEtapa = Tutorial::Etapa::Chegada;
    Tutorial::Desempenho mDesempenho;
    float mProximoTiro = Tutorial::kPrimeiroTiro;
    int mSofridosAntes = -1;
    std::vector<int> mExtrasAntes;
    /// Anda o relogio e o desempenho, passa a etapa quando da, e puxa o gatilho do ritmo dela.
    void AndarORelogio(float deltaTime);
    void ComecarEtapa(Tutorial::Etapa e);

    // ---- O desenho (Boss::Linha e companhia) ----
    static constexpr int kOrdemDasLinhas = 120;
    /// O circulo do tutorial e mais redondo que o padrao.
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 20) { Boss::Circulo(centro, raio, cor, lados); }
    void Seta(Vector2 de, Vector2 ate, SDL_Color cor, float ponta = 9.0f);
    /// Um algarismo de sete segmentos (o 1, o 2 e o 4 dos botoes), de 'altura' pixels.
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
    /// O letreiro do comeco ("TUTORIAL") e o do fim ("TUTORIAL CONCLUIDO!"): sem eles, ninguem
    /// sabia que tinha entrado no tutorial, nem que tinha terminado.
    EtiquetaDeCodigo* mTitulo = nullptr;
    EtiquetaDeCodigo* mSubtitulo = nullptr;
    bool mTocouNoFim = false;
    void Letreiro(bool mostrar, const std::string& titulo = "", const std::string& subtitulo = "",
                  SDL_Color cor = SDL_Color{255, 255, 255, 255}, float y = 0.0f);
    /// Onde o Ponto Extra de cada aluno caiu, na etapa dele: a seta aponta para la.
    std::vector<Vector2> mOndeDoPontoExtra;
};
