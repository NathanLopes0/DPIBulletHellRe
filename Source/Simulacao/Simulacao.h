//
// O jogo rodando sozinho, para medir e fotografar.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

/// Semente presa, passo de tempo fixo, um piloto no lugar do aluno, a batalha direto e o relatorio da
/// nota - tudo pela linha de comando, e desligado por padrao: sem nenhum destes parametros, o jogo e o
/// de sempre. Antes isto era remendado numa copia do codigo a cada medicao (build-e-testes.md); morando
/// aqui, o executavel que se mede e o mesmo que se joga, e nenhuma refatoracao derruba a medida. Nao
/// conhece Game nem SDL.
namespace Simulacao {

    /// Quem joga no lugar do aluno. Nenhum: o teclado e o painel, como sempre.
    enum class Piloto {
        Nenhum,
        Segue,    ///< persegue o x do chefe (no TCC, o objetivo e o lugar da etapa)
        Parado,   ///< nao sai do lugar
        VaiVem,   ///< varre a largura de um lado ao outro, sem mirar
    };

    /// Uma tecla do roteiro: o nome dela no SDL ("Space", "B", "Return") e o quadro em que e apertada.
    struct Tecla {
        std::string nome;
        int quadro = 0;
    };

    /// Quantos quadros cada tecla do roteiro fica apertada: tres, para a borda das cenas pegar.
    constexpr int kQuadrosDeTecla = 3;

    struct Configuracao {
        // ---- A batalha direta ----
        std::string batalha;          ///< --batalha: o indice em materias.json ou o codigo ("INF213")
        int jogadores = 1;            ///< --jogadores 1|2
        std::string orientador;       ///< --orientador: o codigo da materia do orientador do TCC
        bool dificil = false;         ///< --dificil; sem ele, Normal
        std::string estado;           ///< --estado: a fase em que o chefe comeca ("StateTwo")

        // ---- O sorteio ----
        std::optional<unsigned> semente;    ///< --semente: o sorteador global; sem ela, o relogio
        std::optional<unsigned> variacao;   ///< --variacao: a semente da batalha; sem ela, a --semente

        // ---- O tempo ----
        float passo = 0.0f;           ///< --passo: segundos por quadro, sem esperar o relogio

        // ---- O piloto ----
        Piloto piloto = Piloto::Nenhum;     ///< --piloto segue|parado|vaivem
        bool atira = false;           ///< --atira: atira sempre que pode
        bool poder = false;           ///< --poder: no TCC, o orientador solta o poder quando ele serve

        // ---- O relatorio ----
        bool relatorio = false;       ///< --relatorio: "q..." a cada 60 quadros de luta, e "FIM ..."
        bool sairNoFim = false;       ///< --sair-no-fim: sai assim que a luta se decide, sem gravar nada
        bool imortal = false;         ///< --imortal: a nota a zero nao encerra a luta

        // ---- O roteiro e as fotos ----
        std::vector<Tecla> teclas;    ///< --teclas "Space@100,8@200"
        std::vector<int> fotos;       ///< --fotos 380,470: a tela nesses quadros; depois da ultima, sai
        std::string pastaDasFotos = ".";    ///< --fotos-em
        int larguraDaJanela = 0;      ///< --janela 1920x1080: a janela de verdade; o jogo segue em 1200x800
        int alturaDaJanela = 0;
        int fimNoQuadro = -1;         ///< --fim-em: a luta se encerra neste quadro
        bool fimAprovado = false;     ///< --fim-aprovado: e encerra aprovada
        std::optional<float> fimComNota;    ///< --fim-nota: com esta nota
        int pausaNoQuadro = -1;       ///< --pausa-em: a pausa abre neste quadro
        int focoNoQuadro = -1;        ///< --foco-em: a janela perde o foco neste quadro

        /// Algum parametro de simulacao veio? So para o log de quem abriu o jogo.
        [[nodiscard]] bool Ativa() const;
    };

    /// Le a linha de comando. O que nao e daqui e ignorado (Gabinete::Ler le o resto); o que e daqui
    /// e nao se le vai para 'problemas'.
    Configuracao Ler(const std::vector<std::string>& argumentos, std::vector<std::string>* problemas = nullptr);

    /// A lista de --fotos: "380,470".
    std::vector<int> LerQuadros(const std::string& lista, std::vector<std::string>* problemas = nullptr);

    /// O roteiro de --teclas: "Space@100,8@200" - nome e quadro, separados por virgula.
    std::vector<Tecla> LerTeclas(const std::string& roteiro, std::vector<std::string>* problemas = nullptr);

    /// A tecla esta apertada no quadro 'quadro'? Do quadro dela ate kQuadrosDeTecla depois.
    bool Apertada(const Tecla& tecla, int quadro);

    /// Para que lado o piloto anda num eixo, de 'atual' ate 'alvo': -1, 0 ou 1. A menos de 2 px,
    /// chegou.
    int Direcao(float atual, float alvo);

    /// O vai e vem: o sentido em que ele segue, estando em 'x' e indo em 'sentido' (1 ou -1). Vira
    /// nas bordas da faixa jogavel.
    float SentidoDoVaiVem(float x, float sentido);
    constexpr float kVaiVemEsquerda = 140.0f;
    constexpr float kVaiVemDireita = 1060.0f;
}
