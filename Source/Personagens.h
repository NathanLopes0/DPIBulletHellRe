//
// O catalogo de pecas da personagem, e a aparencia escolhida por um aluno.
//

#pragma once

#include <map>
#include <string>
#include <vector>

/**
 * CAMADA PURA. Transforma texto em catalogo, responde o que e uma aparencia
 * valida e diz quais camadas compoem uma personagem; nao abre arquivo, nao
 * desenha pixel nenhum e nao conhece SDL.
 *
 * POR QUE ISTO EXISTE. A personagem nao e uma sprite pronta escolhida numa
 * lista: e um corpo template com camadas por cima - tom de pele, cabelo,
 * camisa, calca - cada uma com a sua cor. O que o aluno escolhe e um conjunto
 * de escolhas, e e esse conjunto que vai para o save.
 *
 * A REGRA de composicao vive aqui e e testavel sem subir uma janela: quais
 * camadas entram, em que ordem e com que cor. Quem desenha os pixels e a ponte
 * (Source/PersonagensArquivo), que fica de fora dos testes.
 */
namespace Personagens {

    struct Cor {
        std::string id;      ///< a CHAVE ESTAVEL, o que vai para o save
        std::string nome;    ///< o que aparece na tela
        unsigned char r = 255, g = 255, b = 255;
    };

    struct Peca {
        std::string id;      ///< a CHAVE ESTAVEL
        std::string nome;
        /// Caminho a partir de Assets/, SEM extensao: quem carrega acrescenta
        /// .png para a imagem e .json para o atlas.
        std::string arte;
    };

    struct Categoria {
        std::string id;
        std::string nome;

        /// Ordem de desenho, do fundo para a frente.
        int ordem = 0;

        std::vector<Peca> pecas;
        std::vector<Cor> cores;

        [[nodiscard]] const Peca* PecaPor(const std::string& id) const;
        [[nodiscard]] const Cor*  CorPor(const std::string& id) const;
    };

    /// Camada que todo mundo tem e ninguem escolhe, em cor final.
    struct CamadaFixa {
        std::string arte;
        int ordem = 0;
    };

    /// O que o aluno escolheu numa categoria.
    struct Escolha {
        std::string peca;
        std::string cor;
    };

    /**
     * @brief A aparencia de um aluno: uma escolha por categoria.
     *
     * Guardada por ID de categoria, e nunca por posicao - reordenar o catalogo
     * nao pode trocar o cabelo de ninguem.
     */
    struct Aparencia {
        std::map<std::string, Escolha> escolhas;

        [[nodiscard]] const Escolha* Por(const std::string& categoria) const;
        void Definir(const std::string& categoria, const Escolha& escolha);

        [[nodiscard]] bool Vazia() const { return escolhas.empty(); }
    };

    /// Uma combinacao pronta, para a primeira versao da tela de criacao.
    struct Predefinida {
        std::string id;
        std::string nome;
        Aparencia aparencia;
    };

    /// Uma camada ja resolvida, pronta para ser desenhada.
    struct Camada {
        std::string arte;
        int ordem = 0;

        /// FALSE para as camadas fixas (olho, sapato), que saem na cor em que
        /// foram desenhadas.
        bool tingida = false;
        unsigned char r = 255, g = 255, b = 255;
    };

    struct Catalogo {
        std::vector<Categoria> categorias;
        std::vector<CamadaFixa> fixas;
        std::vector<Predefinida> predefinidas;

        /// Frases prontas dizendo o que estava errado no arquivo.
        std::vector<std::string> problemas;

        [[nodiscard]] bool Vazio() const { return categorias.empty(); }
        [[nodiscard]] const Categoria* Por(const std::string& id) const;

        /**
         * @brief A aparencia padrao: a primeira peca e a primeira cor de cada
         *        categoria.
         *
         * E o que o visitante usa, e para onde cai quem tem save sem aparencia.
         */
        [[nodiscard]] Aparencia Padrao() const;

        /**
         * @brief Conserta uma aparencia vinda de um save.
         *
         * CONSERTA CAMADA POR CAMADA, e nao tudo ou nada: se o tipo de cabelo
         * sumiu do catalogo mas a cor dele continua valendo, so o tipo volta ao
         * padrao. Perder o cabelo nao pode custar a camisa.
         *
         * Escolha de categoria que nao existe mais e descartada. O que foi
         * trocado sai em `trocas`, quando ele nao e nulo.
         */
        [[nodiscard]] Aparencia Resolver(const Aparencia& pedida,
                                         std::vector<std::string>* trocas = nullptr) const;

        /**
         * @brief As camadas a compor, JA EM ORDEM de desenho.
         *
         * Passa a aparencia por Resolver antes, entao serve direto para uma
         * aparencia lida do disco.
         */
        [[nodiscard]] std::vector<Camada> Camadas(const Aparencia& aparencia) const;
    };

    /**
     * @brief Interpreta o texto de personagens.json.
     *
     * Nunca lanca. Uma peca com problema e descartada e as outras continuam
     * valendo; uma categoria que ficou sem peca ou sem cor e descartada inteira,
     * porque nao haveria o que escolher nela.
     *
     * Confere tambem o que nenhum compilador pegaria: ordens de desenho
     * repetidas (qual camada fica por cima viraria sorteio) e combinacoes
     * prontas citando peca ou cor que nao existe.
     */
    Catalogo LerCatalogo(const std::string& textoJson);

    /// @brief Se o texto serve como identificador (letras, numeros, - e _).
    bool IdServe(const std::string& id);

    /**
     * @brief Um canal de cor tingido por outro.
     *
     * A conta inteira do tingimento: a peca e desenhada em luminancia, e a cor
     * escolhida multiplica cada canal. Cinza 255 devolve a cor tal e qual;
     * cinza 128 devolve metade dela, que e a sombra.
     *
     * Vive na camada pura, e nao solta dentro do laco de pixels, porque e a
     * unica conta que o tingimento tem - e portanto a unica que pode estar
     * errada sem ninguem ver.
     */
    unsigned char Multiplicar(unsigned char valor, unsigned char cor);

    /**
     * @brief Um nome unico e estavel para uma aparencia.
     *
     * Serve de chave do cache de texturas: a mesma aparencia da sempre a mesma
     * chave, entao compor duas vezes reaproveita a textura em vez de montar
     * outra igual. Comeca com um prefixo que nenhum caminho de arquivo tem,
     * para nunca se confundir com um.
     */
    std::string ChaveDaAparencia(const Aparencia& aparencia);

    /// @brief Le "rrggbb" em componentes. FALSE quando o texto nao serve.
    bool LerCorHex(const std::string& hex, unsigned char& r, unsigned char& g, unsigned char& b);
}
