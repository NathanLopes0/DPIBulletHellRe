//
// A repescagem da Sabrina: o sequenciamento - PCR, shotgun e CRISPR.
//

#include "Sequenciamento.h"

#include <algorithm>
#include <cmath>

namespace Sequenciamento {

    using Pareamento::Base;

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Ciclo CicloNoTempo(const float t, const float periodo) {
        Ciclo c;
        if (periodo <= 0.0f || t < 0.0f) return c;
        c.numero = static_cast<int>(std::floor(t / periodo));
        const float u = t / periodo - static_cast<float>(c.numero);
        if (u < kFimDaDesnaturacao) {
            c.passo = Passo::Desnatura;
            c.temperatura = 95.0f;
        } else if (u < kFimDoAnelamento) {
            c.passo = Passo::Anela;
            c.temperatura = 55.0f;
        } else {
            c.passo = Passo::Estende;
            c.temperatura = 72.0f;
        }
        return c;
    }

    int Copias(const int moldes, const int ciclos, const int teto) {
        long long n = std::max(0, moldes);
        for (int k = 0; k < ciclos && n < teto; ++k) n *= 2;
        return static_cast<int>(std::min<long long>(n, teto));
    }

    std::vector<Vector2> Contig(const Vector2 centro, const int atras, const int frente, const float espacamento,
                                const float giro) {
        std::vector<Vector2> contas;
        const Vector2 u(Math::Cos(giro), Math::Sin(giro));
        for (int k = -std::max(0, atras); k <= std::max(0, frente); ++k) {
            contas.push_back(centro + u * (espacamento * static_cast<float>(k)));
        }
        return contas;
    }

    float DistanciaAoSegmento(const Vector2 p, const Vector2 a, const Vector2 b) {
        const Vector2 ab = b - a;
        const float l2 = ab.LengthSq();
        if (l2 < 1e-8f) return (p - a).Length();
        const float u = std::clamp(Vector2::Dot(p - a, ab) / l2, 0.0f, 1.0f);
        return (p - (a + ab * u)).Length();
    }

    int Encosta(const Vector2 p, const Vector2 centro, const int atras, const int frente, const float espacamento,
                const float giro, const float alcance) {
        const Vector2 u(Math::Cos(giro), Math::Sin(giro));
        const Vector2 tras = centro - u * (espacamento * static_cast<float>(std::max(0, atras)));
        const Vector2 ponta = centro + u * (espacamento * static_cast<float>(std::max(0, frente)));
        if (DistanciaAoSegmento(p, tras, ponta) > alcance) return 0;
        // A ponta mais perto do toque, pela projecao na reta.
        return Vector2::Dot(p - centro, u) >= 0.0f ? 1 : -1;
    }

    bool Pam(const std::vector<Base>& fita, const int i) {
        if (i < 0 || i + 2 >= static_cast<int>(fita.size())) return false;
        return fita[static_cast<size_t>(i + 1)] == Base::G && fita[static_cast<size_t>(i + 2)] == Base::G;
    }

    int Procurar(const std::vector<Base>& fita, const std::vector<Base>& guia, int* quantas) {
        int primeira = -1, n = 0;
        const int g = static_cast<int>(guia.size());
        for (int i = 0; g > 0 && i + g + 2 < static_cast<int>(fita.size()); ++i) {
            bool casa = true;
            for (int k = 0; k < g && casa; ++k) casa = fita[static_cast<size_t>(i + k)] == guia[static_cast<size_t>(k)];
            if (!casa || !Pam(fita, i + g)) continue;
            if (primeira < 0) primeira = i;
            ++n;
        }
        if (quantas) *quantas = n;
        return primeira;
    }

    std::vector<Base> Guia(Variacao::Sorteio& s, const int n) {
        std::vector<Base> g;
        for (int k = 0; k < n; ++k) g.push_back(Pareamento::kDna[static_cast<size_t>(s.Inteiro(0, 3))]);
        // Um guia so de G casaria com o proprio PAM: pelo menos uma base nao e G.
        if (!g.empty() && std::all_of(g.begin(), g.end(), [](const Base b) { return b == Base::G; })) g.front() = Base::A;
        return g;
    }

    Genoma Montar(Variacao::Sorteio& s, const int n, const std::vector<Base>& guia, const int iscas) {
        Genoma gen;
        const int g = static_cast<int>(guia.size());
        const int trecho = g + 3;   // o guia e o PAM
        if (g <= 0 || n < trecho + 2) return gen;
        for (int tentativa = 0; tentativa < 40; ++tentativa) {
            gen = Genoma{};
            gen.fita.clear();
            for (int k = 0; k < n; ++k) gen.fita.push_back(Pareamento::kDna[static_cast<size_t>(s.Inteiro(0, 3))]);
            // Os lugares: o alvo e as iscas, sem se encostar.
            std::vector<int> livres;
            for (int i = 1; i + trecho < n; ++i) livres.push_back(i);
            s.Embaralhar(livres);
            std::vector<int> usados;
            const auto cabe = [&](const int i) {
                for (const int u : usados) if (std::abs(u - i) < trecho + 1) return false;
                return true;
            };
            for (const int i : livres) {
                if (static_cast<int>(usados.size()) >= 1 + iscas) break;
                if (cabe(i)) usados.push_back(i);
            }
            if (usados.empty()) continue;
            const auto escrever = [&](const int i, const int erro) {
                for (int k = 0; k < g; ++k) {
                    Base b = guia[static_cast<size_t>(k)];
                    if (k == erro) b = Pareamento::Outra(s, b, false);
                    gen.fita[static_cast<size_t>(i + k)] = b;
                }
                gen.fita[static_cast<size_t>(i + g)] = Pareamento::kDna[static_cast<size_t>(s.Inteiro(0, 3))];
                gen.fita[static_cast<size_t>(i + g + 1)] = Base::G;
                gen.fita[static_cast<size_t>(i + g + 2)] = Base::G;
            };
            gen.alvo = usados.front();
            escrever(gen.alvo, -1);
            for (size_t k = 1; k < usados.size(); ++k) {
                escrever(usados[k], s.Inteiro(0, g - 1));
                gen.iscas.push_back(usados[k]);
            }
            int quantas = 0;
            if (Procurar(gen.fita, guia, &quantas) == gen.alvo && quantas == 1) return gen;
        }
        return gen;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.periodo = 0.85f;
        d.moldes = 3;
        d.teto = 52;
        d.rapidezMinima = 85.0f;
        d.rapidezMaxima = 150.0f;
        d.resistenciaDoPrimer = 2;
        d.pontaInicial = 3;
        d.pontaMaxima = 19;
        d.giro = 1.15f;
        d.entreRajadas = 0.36f;
        d.tirosDaRajada = 8;
        d.velocidadeDoRead = 220.0f;
        d.tamanhoDoGuia = 5;
        d.iscas = 4;
        d.descida = 185.0f;
        d.entreFitas = 3.0f;
        d.resistenciaDoAlvo = 2;
        d.tirosForaDoAlvo = 7;
        d.velocidadeForaDoAlvo = 200.0f;
        return d;
    }
}
