//
// O progresso academico do jogador: o recorde por materia e o ponto de retomada.
//

#pragma once

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Nota.h"

/// Nao conhece Game, Scene nem SDL: so guarda notas e responde perguntas sobre elas.
class Progresso {
public:

    /// Nota com que uma batalha comeca quando a materia nunca foi jogada.
    static constexpr float kNotaInicial = 40.0f;

    /// A partir de quanto a materia conta como aprovada.
    static constexpr float kNotaDeAprovacao = Nota::kNotaAprovacao;

    /// Sozinho ou em dupla: cada modo tem o proprio recorde, e o proprio ranking.
    enum class Modo { Solo, Dupla };

    /// Um recorde: a nota, e - na dupla - com quem e quando foi feito.
    struct Marca {
        float nota = 0.0f;
        std::string parceiro;
        /// Quando a dupla fez o recorde: e ele que data a linha do ranking de duplas. No solo fica
        /// vazio - "quando jogou" e Entrada::quando.
        std::string quando;
    };

    /// Registra o resultado de uma batalha jogada sozinho.
    void RegistrarNota(int materia, float nota, Dificuldade::Nivel nivel, const std::string& quando = "");

    /// Registra o resultado de uma batalha jogada em dupla.
    void RegistrarNotaDeDupla(int materia, float nota, Dificuldade::Nivel nivel,
                              const std::string& parceiro, const std::string& quando = "");

    /// A maior nota que o jogador ja tirou nesta materia, em qualquer modo e nivel. Zero quando
    /// nunca jogou.
    [[nodiscard]] float MelhorNota(int materia) const;

    /// O recorde de um modo e nivel. Nota zero quando nunca jogou assim.
    [[nodiscard]] const Marca& Recorde(int materia, Modo modo, Dificuldade::Nivel nivel) const;

    /// O jogador passou nesta materia? Olha o recorde, nao a ultima nota.
    [[nodiscard]] bool Aprovado(int materia) const;

    /// Quando o jogador passou nesta materia pela primeira vez ("2026-10-02 14:30:00"). Vazio se nunca
    /// passou, ou se passou num save de antes de a data existir (versao 8 da ficha).
    [[nodiscard]] const std::string& AprovadaEm(int materia) const;

    /// Quantas provas desta materia o aluno terminou, e quantas reprovou (Entrada::tentativas).
    [[nodiscard]] int Tentativas(int materia) const;
    [[nodiscard]] int Reprovacoes(int materia) const;
    /// As reprovacoes seguidas nesta materia, em qualquer modo e nivel: aprovar zera (o ajuste de
    /// dificuldade, Ajuste.h).
    [[nodiscard]] int ReprovacoesSeguidas(int materia) const;

    /// O professor desta materia orientou um TCC em que o aluno foi aprovado: o curriculo de
    /// orientacoes. So conta em materia aprovada - e dela que sai o candidato a orientador.
    void RegistrarOrientacao(int materia);
    /// Quantos TCCs aprovados do aluno o professor desta materia orientou.
    [[nodiscard]] int Orientacoes(int materia) const;

    /// As atividades complementares cumpridas (Horas.h), pela chave. Devolve se a chave e nova.
    bool RegistrarAtividade(const std::string& chave) { return !chave.empty() && mAtividades.insert(chave).second; }
    /// Tira uma atividade (a tecla de desenvolvedor que desfaz as matriculas). Devolve se havia.
    bool EsquecerAtividade(const std::string& chave) { return mAtividades.erase(chave) > 0; }
    [[nodiscard]] const std::set<std::string>& Atividades() const { return mAtividades; }
    void RestaurarAtividades(std::set<std::string> chaves) { mAtividades = std::move(chaves); }

    /// Quantas das materias listadas estao aprovadas.
    [[nodiscard]] int QuantasAprovadas(const std::vector<int>& materias) const;

    /// Quantas materias tem registro. So para diagnostico e teste.
    [[nodiscard]] size_t QuantasRegistradas() const { return mPorMateria.size(); }

    /// Tudo o que se sabe de uma materia, numa linha so: o que se grava e o que se le de volta.
    struct Entrada {
        int materia = 0;

        /// A nota da ultima tentativa, em qualquer modo e nivel. Nao ha uma.
        float retomada = 0.0f;

        /// Quando o aluno jogou esta materia pela ultima vez. Vazio quando o save e anterior a este
        /// campo existir.
        std::string quando;

        std::array<Marca, 4> recordes{};

        /// As provas terminadas nesta materia, em qualquer modo e nivel, e quantas delas reprovaram
        /// (o boletim). Zero num save de antes da contagem existir.
        int tentativas = 0;
        int reprovacoes = 0;
        /// As reprovacoes desde a ultima aprovacao (versao 11 da ficha).
        int seguidas = 0;

        /// Quando a materia foi aprovada pela primeira vez: o mural dos formados mostra a do TCC.
        /// Vazio se nunca foi, ou num save de antes deste campo (versao 8 da ficha).
        std::string aprovadaEm;

        /// Quantos TCCs aprovados do aluno o professor desta materia orientou (versao 9 da ficha).
        int orientacoes = 0;

        [[nodiscard]] Marca& De(Modo modo, Dificuldade::Nivel nivel) { return recordes[Indice(modo, nivel)]; }
        [[nodiscard]] const Marca& De(Modo modo, Dificuldade::Nivel nivel) const {
            return recordes[Indice(modo, nivel)];
        }

        /// O maior dos quatro recordes.
        [[nodiscard]] float MaiorRecorde() const;
    };

    /// Tudo que esta guardado, em ordem de materia. Existe para a persistencia e para a planilha.
    [[nodiscard]] std::vector<Entrada> Entradas() const;

    /// Repoe um estado lido de arquivo, substituindo o que havia.
    void Restaurar(const std::vector<Entrada>& entradas);

private:
    /// A posicao de um modo e nivel em Entrada::recordes.
    static constexpr size_t Indice(const Modo modo, const Dificuldade::Nivel nivel) {
        return (modo == Modo::Dupla ? 2u : 0u) + (nivel == Dificuldade::Nivel::Dificil ? 1u : 0u);
    }

    /// O que as duas formas de registrar tem em comum.
    void Registrar(int materia, float nota, Modo modo, Dificuldade::Nivel nivel,
                   const std::string& parceiro, const std::string& quando);

    /// Um registro por materia, e nao mapas paralelos.
    std::map<int, Entrada> mPorMateria;

    /// As chaves das atividades complementares cumpridas: do aluno, e nao de uma materia.
    std::set<std::string> mAtividades;

    /// O registro de uma materia, ou um vazio.
    [[nodiscard]] const Entrada& Ver(int materia) const;
};
