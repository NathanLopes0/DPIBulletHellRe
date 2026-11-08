//
// O catalogo de pecas da personagem, e a aparencia escolhida por um aluno.
//

#pragma once

#include <map>
#include <string>
#include <vector>

/// Transforma texto em catalogo, responde o que e uma aparencia valida e diz quais camadas compoem
/// uma personagem.
namespace Personagens {

    struct Cor {
        std::string id;      ///< A chave estavel, o que vai para o save.
        std::string nome;    ///< o que aparece na tela
        unsigned char r = 255, g = 255, b = 255;
    };

    struct Peca {
        std::string id;      ///< A chave estavel.
        std::string nome;
        /// Caminho a partir de Assets/, sem extensao: quem carrega acrescenta .png para a imagem e
        /// .json para o atlas.
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

    /// A aparencia de um aluno: uma escolha por categoria.
    struct Aparencia {
        std::map<std::string, Escolha> escolhas;

        [[nodiscard]] const Escolha* Por(const std::string& categoria) const;
        void Definir(const std::string& categoria, const Escolha& escolha);

        [[nodiscard]] bool Vazia() const { return escolhas.empty(); }
    };

    struct Predefinida {
        std::string id;
        std::string nome;
        Aparencia aparencia;
    };

    /// Uma camada ja resolvida, pronta para ser desenhada.
    struct Camada {
        std::string arte;
        int ordem = 0;

        /// False para as camadas fixas (olho, sapato), que saem na cor em que foram desenhadas.
        bool tingida = false;
        unsigned char r = 255, g = 255, b = 255;
    };

    struct Catalogo {
        std::vector<Categoria> categorias;
        std::vector<CamadaFixa> fixas;
        std::vector<Predefinida> predefinidas;

        /// Frases prontas dizendo o que estava errado no arquivo.
        std::vector<std::string> problemas;

        [[nodiscard]] const Categoria* Por(const std::string& id) const;

        /// Conserta uma aparencia vinda de um save.
        [[nodiscard]] Aparencia Resolver(const Aparencia& pedida,
                                         std::vector<std::string>* trocas = nullptr) const;

        /// As camadas a compor, ja em ordem de desenho.
        [[nodiscard]] std::vector<Camada> Camadas(const Aparencia& aparencia) const;
    };

    /// Interpreta o texto de personagens.json. Nunca lanca.
    Catalogo LerCatalogo(const std::string& textoJson);

    /// Se o texto serve como identificador (letras, numeros, - e _).
    bool IdServe(const std::string& id);

    /// Um canal de cor tingido por outro. A conta inteira do tingimento: a peca e desenhada em
    /// luminancia, e a cor escolhida multiplica cada canal.
    unsigned char Multiplicar(unsigned char valor, unsigned char cor);

    /// Um nome unico e estavel para uma aparencia.
    std::string ChaveDaAparencia(const Aparencia& aparencia);

    /// Le "rrggbb" em componentes. False quando o texto nao serve.
    bool LerCorHex(const std::string& hex, unsigned char& r, unsigned char& g, unsigned char& b);
}
