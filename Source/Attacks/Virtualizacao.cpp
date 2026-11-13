//
// A repescagem do Goulart: virtualizacao e nuvem.
//

#include "Virtualizacao.h"

#include <algorithm>
#include <cmath>

namespace Virtualizacao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Vector2 Host(const int k, const float x, const float y, const float largura, const float altura) {
        const int i = ((k % kHosts) + kHosts) % kHosts;
        const float coluna = static_cast<float>(i % 3);
        const float fileira = static_cast<float>(i / 3);
        return Vector2(x + largura * (0.2f + 0.3f * coluna), y + altura * (0.42f + 0.38f * fileira));
    }

    char NomeDoHost(const int k) {
        return static_cast<char>('A' + ((k % kHosts) + kHosts) % kHosts);
    }

    int VmsNoTempo(const float t, const int maximo) {
        const float inicioDaElasticidade = kDuracoes[0] + kDuracoes[1];
        if (t < kDuracoes[0]) return std::min(2, maximo);
        if (t < inicioDaElasticidade) return std::min(3, maximo);
        const int mais = 1 + static_cast<int>((t - inicioDaElasticidade) / kEntreEscalas);
        return std::clamp(3 + mais, 0, std::min(maximo, kHosts));
    }

    int HostLivre(const std::vector<bool>& ocupado, const int depois) {
        const int n = static_cast<int>(ocupado.size());
        for (int i = 1; i <= n; ++i) {
            const int k = (((depois + i) % n) + n) % n;
            if (!ocupado[static_cast<size_t>(k)]) return k;
        }
        return -1;
    }

    void Quicar(Vector2& posicao, Vector2& velocidade, const Vector2& centro, const float largura, const float altura,
                const float folga) {
        const float meiaL = std::max(0.0f, largura / 2.0f - folga), meiaA = std::max(0.0f, altura / 2.0f - folga);
        if (posicao.x < centro.x - meiaL) { posicao.x = centro.x - meiaL; velocidade.x = std::fabs(velocidade.x); }
        if (posicao.x > centro.x + meiaL) { posicao.x = centro.x + meiaL; velocidade.x = -std::fabs(velocidade.x); }
        if (posicao.y < centro.y - meiaA) { posicao.y = centro.y - meiaA; velocidade.y = std::fabs(velocidade.y); }
        if (posicao.y > centro.y + meiaA) { posicao.y = centro.y + meiaA; velocidade.y = -std::fabs(velocidade.y); }
    }

    Vector2 NaMigracao(const Vector2& de, const Vector2& para, const float u) {
        const float v = std::clamp(u, 0.0f, 1.0f);
        const float suave = v * v * (3.0f - 2.0f * v);
        return de + (para - de) * suave;
    }

    std::string Placar(const int vms) {
        return "Hipervisor: " + std::to_string(vms) + (vms == 1 ? " VM" : " VMs") + " em " + std::to_string(kHosts) + " hosts";
    }

    std::string PlacarDoTrap(const int vm) {
        // \xc3\xa7\xc3\xa3 e "ca" com cedilha e til; \xe2\x86\x92 a seta.
        return "VM " + std::to_string(vm) + ": instru\xc3\xa7\xc3\xa3o privilegiada \xe2\x86\x92 trap no hipervisor";
    }

    std::string PlacarDaMigracao(const int vm, const int de, const int para) {
        return "Migra\xc3\xa7\xc3\xa3o ao vivo: VM " + std::to_string(vm) + ", do host " + NomeDoHost(de) + " para o " +
               NomeDoHost(para);
    }

    std::string PlacarDaEscala(const int de, const int para) {
        return "Carga subindo: " + std::to_string(de) + " \xe2\x86\x92 " + std::to_string(para) + " VMs";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.maisVms = 6;
        d.cadencia = 0.85f;
        return d;
    }
}
