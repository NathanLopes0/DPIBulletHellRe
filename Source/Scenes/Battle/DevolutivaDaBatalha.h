//
// A devolutiva da prova, anotada enquanto a luta acontece: onde a nota caiu, o que mais acertou o
// aluno e, na dupla, a parte de cada um.
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "OuvinteDaBatalha.h"
#include "../../Interface/Devolutiva.h"
#include "../../Pesquisa/Experimento.h"

/// Ouve a batalha (OuvinteDaBatalha) e anota no Caderno da devolutiva (Devolutiva.h) cada perda de
/// nota, com a fase e a causa; e, para a justica da dupla, os acertos no professor, as coberturas e
/// os acertos sofridos de cada aluno. A tela do resultado le daqui.
class DevolutivaDaBatalha : public OuvinteDaBatalha {
public:
    /// 'jogadores': quantos alunos lutam. A cobertura so entra na parte de quem cobriu quando a
    /// 'condicao' do experimento a paga.
    DevolutivaDaBatalha(std::size_t jogadores, const Experimento::Condicao& condicao);

    void AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) override;
    void AoPerderNota(Battle& b, const Player& aluno, const std::string& causa, float perda) override;
    void AoAcertarOProfessor(Battle& b, const Actor* quemAtirou, bool ouviu) override;
    void AoCobrir(Battle& b, const Player& quemCobriu, const Player& parceiro) override;

    [[nodiscard]] const Devolutiva::Caderno& Caderno() const { return mCaderno; }
    /// A parte de cada aluno, na ordem das posicoes do painel (sem o nome: quem monta o resumo o poe).
    [[nodiscard]] const std::vector<Devolutiva::Parte>& Partes() const { return mPartes; }

private:
    /// A parte do aluno 'ator', ou nula se ele nao e um aluno (o orientador).
    Devolutiva::Parte* ParteDe(const Battle& b, const Actor* ator);

    Devolutiva::Caderno mCaderno;
    std::vector<Devolutiva::Parte> mPartes;
    bool mContaCoberturas;
};
