//
// A fase 2 do Vitor: as nuvens e o controle de congestionamento do TCP.
//

#include "Tcp.h"

#include <algorithm>
#include <cmath>

namespace Tcp {

    namespace {
        // \xc2\xb7 e o ponto do meio, \xc3\xa7\xc3\xa3 o "cao" de evitacao: o
        // MSVC nao le acento nem simbolo em literal.
        const char* const kPonto = " \xc2\xb7 ";
    }

    Janela Enviar(Conexao& c) {
        Janela j;
        const int total = std::max(1, c.cwnd);
        j.retransmitidos = std::min(c.aRetransmitir, total);
        j.novos = total - j.retransmitidos;
        j.perdidos = std::max(0, total - c.capacidade);
        c.aRetransmitir = 0;
        ++c.rtt;
        return j;
    }

    Desfecho Reconhecer(Conexao& c, const Janela& j) {
        // ACKS DUPLICADOS: o que se perdeu volta primeiro na janela seguinte,
        // e ela cai a metade (o Reno - sem voltar ao slow start).
        if (j.perdidos > 0) {
            c.ssthresh = std::max(kMenorSsthresh, c.cwnd / 2);
            c.cwnd = c.ssthresh;
            c.estado = Estado::Evitacao;
            c.aRetransmitir = j.perdidos;
            return Desfecho::Cortou;
        }
        if (c.estado == Estado::SlowStart) {
            c.cwnd *= 2;
            if (c.cwnd >= c.ssthresh) {
                c.cwnd = c.ssthresh;
                c.estado = Estado::Evitacao;
            }
            return Desfecho::Dobrou;
        }
        c.cwnd += 1;
        return Desfecho::Cresceu;
    }

    void Timeout(Conexao& c) {
        c.ssthresh = std::max(kMenorSsthresh, c.cwnd / 2);
        c.cwnd = kJanelaInicial;
        c.estado = Estado::SlowStart;
        c.aRetransmitir = 0;
    }

    Ponto Mira(const Ponto de, const Ponto aluno, const Ponto velocidade, const float rapidez, const float quanto) {
        if (rapidez <= 0.0f) return aluno;
        const float dx = aluno.x - de.x;
        const float dy = aluno.y - de.y;
        const float voo = std::sqrt(dx * dx + dy * dy) / rapidez;
        return {aluno.x + velocidade.x * voo * quanto, aluno.y + velocidade.y * voo * quanto};
    }

    float Antecipacao(const int i, const bool retransmitido) {
        if (retransmitido) return 1.0f;
        return (i % 2 == 0) ? 0.0f : 1.0f;
    }

    Ponto Roteador(const float largura, const float fracao) {
        return {largura * fracao, kAlturaDoRoteador};
    }

    float GiroDoDescarte(const int k, const int anel) {
        // Um terco do espaco entre dois cacos a cada anel: tres perdas seguidas
        // fecham o circulo.
        return static_cast<float>(k % 3) * (360.0f / static_cast<float>(std::max(1, anel))) / 3.0f;
    }

    Retangulo Grafico(const float largura) {
        return {largura - kLarguraDoGrafico - 16.0f, kTopoDoGrafico, kLarguraDoGrafico, kAlturaDoGrafico};
    }

    Ponto NoGrafico(const int rtt, const int cwnd, const float largura) {
        const Retangulo g = Grafico(largura);
        constexpr float kFolga = 12.0f;
        const int r = std::clamp(rtt, 0, kRttsNoGrafico - 1);
        const int w = std::clamp(cwnd, 0, kMaiorNoGrafico);
        return {g.x + kFolga + static_cast<float>(r) / static_cast<float>(kRttsNoGrafico - 1) * (g.w - 2.0f * kFolga),
                g.y + g.h - kFolga - static_cast<float>(w) / static_cast<float>(kMaiorNoGrafico) * (g.h - 2.0f * kFolga)};
    }

    std::string Placar(const Conexao& c) {
        const std::string estado = c.estado == Estado::SlowStart ? "slow start" : "evita\xc3\xa7\xc3\xa3o";
        return "cwnd = " + std::to_string(c.cwnd) + kPonto + estado;
    }

    std::string PlacarDaPerda(const Conexao& c) {
        return "3 ACKs duplicados" + std::string(kPonto) + "cwnd = " + std::to_string(c.cwnd);
    }

    std::string PlacarDoTimeout(const Conexao& c) {
        return "timeout" + std::string(kPonto) + "cwnd = " + std::to_string(c.cwnd);
    }

    std::string Fila(const int recebidos, const int capacidade) {
        return "roteador" + std::string(kPonto) + std::to_string(std::clamp(recebidos, 0, capacidade)) + "/" +
               std::to_string(capacidade);
    }

    Plano PlanoDeSempre() { return {}; }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.capacidade = kCapacidades[s.Inteiro(0, 2)];
        p.ssthreshInicial = kSsthreshs[s.Inteiro(0, 2)];
        p.fracaoDoRoteador = s.Real(kMenorFracaoDoRoteador, kMaiorFracaoDoRoteador);
        return p;
    }

    Conexao Abrir(const Plano& p) {
        Conexao c;
        c.capacidade = p.capacidade;
        c.ssthresh = p.ssthreshInicial;
        return c;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreEncaminhados = Dificuldade::Escolher(n, kEntreEncaminhados, 0.045f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.25f);
        d.anelDoDescarte = Dificuldade::Escolher(n, kAnelDoDescarte, 10);
        d.menosCapacidade = Dificuldade::Escolher(n, 0, 2);
        return d;
    }

    Plano NoDegrau(Plano p, const Degrau& d) {
        p.capacidade = std::max(kMenorCapacidade, p.capacidade - d.menosCapacidade);
        return p;
    }
}
