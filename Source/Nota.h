//
// Como a nota cresce durante uma batalha.
//

#pragma once

/**
 * CAMADA PURA. So aritmetica de nota; nao conhece batalha, chefe, SDL nem disco.
 *
 * POR QUE ISTO EXISTE. A nota e a unica pontuacao do jogo, entao ela e o que um
 * ranking vai ordenar. Com ganho constante, chegar a 100 era so questao de
 * tempo de tela, e um ranking em que todo mundo tem 100 nao ordena nada.
 *
 * SAO DOIS FREIOS DIFERENTES, de proposito:
 *
 *   A CURVA torna cada ponto acima de 60 mais caro que o anterior. Ela nao
 *   impede o 100 - so cobra caro por ele.
 *
 *   O TETO POR DANO impede. Quem foi atingido vezes demais nao chega a 100
 *   naquela batalha por mais tempo que fique em tela.
 *
 * Separar os dois e o que permite que 100 signifique algo preciso - "subiu a
 * curva inteira E quase nao foi atingido" - em vez de ser um numero que ninguem
 * alcanca. Uma curva que chegasse a zero em 100 faria a nota TENDER a 100 sem
 * nunca chegar, e entao a comemoracao de nota cheia nunca aconteceria.
 */
namespace Nota {

    /// Abaixo disto o ganho e cheio. E a nota de aprovacao, e a faixa ate ela
    /// continua tao facil quanto era antes da curva existir.
    inline constexpr float kFaixaFacil = 60.0f;

    /// A nota em que o ganho CHEGARIA a zero. Fica ACIMA de 100 de proposito:
    /// em 100 ainda sobra ganho, pequeno, e por isso a nota cheia e alcancavel.
    inline constexpr float kAssintota = 110.0f;

    /// Quao rapido o ganho mingua acima de kFaixaFacil.
    ///
    /// ESTE NUMERO FOI MEDIDO, NAO ESCOLHIDO. Uma batalha dura 51s e o tiro sai
    /// a cada 0,12s, entao o teto absoluto e 425 tiros. Rodando o jogo com o
    /// tiro preso e mira automatica contra os quatro chefes, o que chega no
    /// chefe fica entre 268 e 382 acertos. Com 1,5, a nota cheia custa 256
    /// acertos: cabe na pior das batalhas boas, e so nelas.
    ///
    /// Com 2,0 custaria 392 - mais do que qualquer batalha medida entrega, e
    /// entao a nota cheia seria inalcancavel e a comemoracao dourada nunca
    /// aconteceria. Com 1,0 custaria 179, alcancado em 21 dos 51 segundos.
    inline constexpr float kExpoente = 1.5f;

    /// O maximo de quem levou dano demais. NAO e 100, e esse e o ponto: a nota
    /// cheia fica reservada a quem quase nao foi atingido.
    inline constexpr float kTetoComDano = 99.99f;

    /// A partir de quantos acertos sofridos o teto passa a valer.
    inline constexpr int kAcertosQuePerdemACheia = 3;

    /// A nota de aprovacao.
    ///
    /// VALE 60 COMO kFaixaFacil, E ISSO E COINCIDENCIA. Aquele e o joelho da
    /// curva de ganho; este e a regra academica. Usar um no lugar do outro faria
    /// mexer na dificuldade mover a aprovacao junto, em silencio.
    inline constexpr float kNotaAprovacao = 60.0f;

    /// O piso da faixa de exame: daqui ate a aprovacao, a batalha segue para a
    /// fase final em vez de terminar.
    ///
    /// E a nota em que toda fase comeca (Progresso::kNotaInicial), e tem de
    /// continuar sendo: a faixa de exame significa "nao perdeu terreno e nao
    /// passou". test_nota.cpp prende os dois numeros um ao outro.
    inline constexpr float kNotaDeExame = 40.0f;

    inline constexpr float kNotaMaxima = 100.0f;
    inline constexpr float kNotaMinima = 0.0f;

    /**
     * @brief Quanto de um ganho bruto sobra, dada a nota atual.
     *
     * Abaixo de kFaixaFacil devolve o ganho inteiro. Acima, multiplica por um
     * fator que mingua conforme a nota sobe, e que vale exatamente 1 em
     * kFaixaFacil - a curva nao tem degrau ali.
     */
    float Ganho(float notaAtual, float ganhoBruto);

    /// @brief Se este numero de acertos sofridos ja custou a nota cheia.
    bool PerdeuACheia(int acertosSofridos);

    /**
     * @brief Se esta nota E a nota cheia - a que vale o dourado e o teste final.
     *
     * Existe para que o limiar nao seja escrito a mao em cada lugar que pinta ou
     * anuncia alguma coisa. Com o teto em 99,99, um ">= 99.9" perdido numa tela
     * acenderia o dourado para quem nao chegou la, e o jogador leria nota cheia
     * sem ter tirado - exatamente o que o teto existe para impedir.
     */
    bool ECheia(float nota);

    /// O que acontece ao fim da terceira fase de um chefe.
    enum class Desfecho {
        Reprovado,     ///< abaixo de 40: a batalha acaba e o aluno nao passou
        Aprovado,      ///< 60 ou mais, sem ser a nota cheia: acaba e passou
        VaiParaFinal   ///< a batalha continua na StateFinal
    };

    /**
     * @brief Para onde a batalha vai quando a terceira fase termina.
     *
     * DUAS NOTAS MUITO DIFERENTES LEVAM A MESMA FASE, de proposito:
     *
     *   40 a 59  e o exame. Quem ficou na faixa da recuperacao ganha mais uma
     *            chance de chegar aos 60.
     *
     *   100      e o teste final. Quem subiu a curva inteira enfrenta a fase
     *            final como premio, e nao como resgate.
     *
     * Entre as duas - de 60 a 99,99 - a batalha acaba aprovada ali mesmo.
     *
     * O QUE ISTO CUSTA AO JOGADOR DE 100: a nota NAO fica guardada ao entrar.
     * Um acerto durante o teste final derruba os 6 pontos de sempre, e a partir
     * do terceiro acerto o teto de 99,99 impede a volta. Quem chega a 100 cedo
     * tem mais tempo exposto do que quem chega no ultimo segundo. E uma escolha
     * de design, nao um efeito colateral - ver o comentario em BossAttackState.
     */
    Desfecho AposTerceiraFase(float nota);

    /// @brief O maior valor que a nota pode atingir com este tanto de dano.
    float TetoCom(int acertosSofridos);

    /**
     * @brief A nota depois de somar um ganho bruto.
     *
     * Junta a curva, o teto por dano e os limites de 0 a 100 num lugar so, para
     * nao haver versao do calculo que esqueceu um dos tres.
     */
    float Somar(float notaAtual, float ganhoBruto, int acertosSofridos);

    /**
     * @brief A nota depois de uma perda.
     *
     * A perda NAO passa pela curva: perder e perder. Fazer o dano minguar junto
     * com o ganho deixaria quem esta perto de 100 quase imune, que e o oposto
     * do que a curva quer.
     */
    float Subtrair(float notaAtual, float perda);
}
