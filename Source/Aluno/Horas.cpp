//
// As horas complementares.
//

#include "Horas.h"

#include <algorithm>

#include "ProvaDaSemana.h"

namespace Horas {

    namespace {
        /// As materias em que uma atividade por materia pode ser cumprida: as que tem batalha.
        std::vector<int> ComBatalha(const Materias::Lista& materias) {
            std::vector<int> v;
            for (int i = 0; i < materias.Quantas(); ++i) {
                const Materias::Materia* m = materias.Por(i);
                if (m && !m->chefe.empty()) v.push_back(i);
            }
            return v;
        }

        const Dados* DaChave(const std::string& chave, std::string* materia = nullptr) {
            const size_t arroba = chave.find('@');
            const std::string base = chave.substr(0, arroba);
            for (const Dados& d : Todas()) {
                if (base != d.chave || d.porMateria != (arroba != std::string::npos)) continue;
                if (materia && arroba != std::string::npos) *materia = chave.substr(arroba + 1);
                return &d;
            }
            return nullptr;
        }
    }

    const std::vector<Dados>& Todas() {
        // \xc3\xa9 e o e com acento, \xc3\xa3 o a com til, \xc3\xb5 o o com til, \xc3\xa7 o c cedilha.
        static const std::vector<Dados> todas = {
            {Atividade::FaseLimpa, "fase-limpa", true, 2, "Fase limpa", "uma fase sem levar acerto"},
            {Atividade::PoucosAcertos, "poucos-acertos", true, 3, "Prova segura", "aprovado com at\xc3\xa9 10 acertos sofridos"},
            {Atividade::Veterano, "veterano", true, 3, "Monitoria", "3 aprova\xc3\xa7\xc3\xb5" "es na mat\xc3\xa9ria"},
            {Atividade::Virada, "virada", false, 6, "Virada", "aprovado depois de a nota cair abaixo de 25"},
            {Atividade::DeRaspao, "de-raspao", false, 4, "De rasp\xc3\xa3o", "15 rasp\xc3\xb5" "es numa prova"},
            {Atividade::PontoExtra, "ponto-extra", false, 4, "Ponto extra", "2 pontos extras usados numa prova"},
            {Atividade::Cobertura, "cobertura", false, 5, "Trabalho em grupo", "10 coberturas ao parceiro numa prova"},
            {Atividade::Poder, "poder", false, 5, "Orienta\xc3\xa7\xc3\xa3o", "3 poderes do orientador num TCC"},
            {Atividade::ProvaDaSemana, "prova-da-semana", false, 5, "Prova da semana", "aprovado na prova da semana"},
            {Atividade::Podio, "podio", false, 8, "P\xc3\xb3" "dio", "entre os 3 primeiros da prova da semana"},
        };
        return todas;
    }

    std::string Chave(const Atividade a, const std::string& materia) {
        for (const Dados& d : Todas()) {
            if (d.atividade == a) return d.porMateria ? std::string(d.chave) + "@" + materia : std::string(d.chave);
        }
        return "";
    }

    std::vector<std::string> Cumpridas(const Feito& f) {
        std::vector<std::string> chaves;
        if (!f.aprovado) return chaves;
        const auto se = [&](const bool cumpriu, const Atividade a) {
            if (cumpriu) chaves.push_back(Chave(a, f.materia));
        };
        se(f.fasesLimpas > 0, Atividade::FaseLimpa);
        se(f.acertosSofridos <= kPoucosAcertos, Atividade::PoucosAcertos);
        se(f.aprovacoesNaMateria >= kAprovacoesDoVeterano, Atividade::Veterano);
        se(f.menorNota < kNotaDaVirada, Atividade::Virada);
        se(f.raspoes >= kRaspoes, Atividade::DeRaspao);
        se(f.pontosExtras >= kPontosExtras, Atividade::PontoExtra);
        se(f.dupla && f.coberturas >= kCoberturas, Atividade::Cobertura);
        se(f.poderes >= kPoderes, Atividade::Poder);
        se(f.provaDaSemana, Atividade::ProvaDaSemana);
        se(f.provaDaSemana && f.posicaoNaSemana >= 1 && f.posicaoNaSemana <= ProvaDaSemana::kPodio, Atividade::Podio);
        return chaves;
    }

    int De(const std::set<std::string>& chaves) {
        int horas = 0;
        for (const std::string& c : chaves) {
            if (const Dados* d = DaChave(c)) horas += d->horas;
        }
        return horas;
    }

    int Possiveis(const Materias::Lista& materias) {
        const int comBatalha = static_cast<int>(ComBatalha(materias).size());
        int horas = 0;
        for (const Dados& d : Todas()) horas += d.horas * (d.porMateria ? comBatalha : 1);
        return horas;
    }

    std::string Nome(const std::string& chave, const Materias::Lista& materias) {
        std::string codigo;
        const Dados* d = DaChave(chave, &codigo);
        if (!d) return "";
        if (!d->porMateria) return d->titulo;
        const Materias::Materia* m = materias.Por(materias.IndiceDe(codigo));
        return std::string(d->titulo) + " em " + (m ? m->nome : codigo);
    }

    std::string FraseDasNovas(const std::vector<std::string>& novas, const Materias::Lista& materias) {
        int horas = 0;
        std::string nomes;
        for (const std::string& c : novas) {
            const Dados* d = DaChave(c);
            if (!d) continue;
            horas += d->horas;
            nomes += (nomes.empty() ? "" : ", ") + Nome(c, materias);
        }
        if (horas == 0) return "";
        return "Horas complementares: +" + std::to_string(horas) + " h (" + nomes + ")";
    }

    std::vector<Linha> Quadro(const std::set<std::string>& chaves, const Materias::Lista& materias) {
        std::vector<Linha> quadro;
        const int comBatalha = static_cast<int>(ComBatalha(materias).size());
        for (const Dados& d : Todas()) {
            Linha l;
            l.dados = d;
            l.possiveis = d.porMateria ? comBatalha : 1;
            for (const std::string& c : chaves) {
                if (DaChave(c) == &d) ++l.cumpridas;
            }
            l.cumpridas = std::min(l.cumpridas, l.possiveis);
            quadro.push_back(l);
        }
        return quadro;
    }
}
