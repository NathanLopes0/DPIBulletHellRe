//
// A repescagem do Goulart: virtualizacao e nuvem.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// A repescagem do Goulart e a nuvem. O professor e o hipervisor, e as maquinas virtuais sao caixas
/// no campo, cada uma com o seu sistema convidado dentro:
///   - o convidado roda o kernel dele (atira em anel) e, de tempos em tempos, executa uma instrucao
///     privilegiada: e um trap, que voa ate o hipervisor, e o hipervisor emula a instrucao - atira
///     no aluno;
///   - na migracao ao vivo, uma VM escorrega de um host para outro sem parar de rodar (e, enquanto
///     o estado e copiado, o tiro nao a pega);
///   - na elasticidade, a carga sobe e a nuvem sobe mais VMs; a que o aluno derruba, ela repoe.
namespace Virtualizacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 3> kEtapas = {"Hipervisor", "Migracao", "Elasticidade"};
    constexpr std::array<float, 3> kDuracoes = {7.0f, 6.0f, 7.0f};
    constexpr const char* kGatilhoDaVm = "vm";
    constexpr const char* kGatilhoDoKernel = "kernel";
    constexpr const char* kGatilhoDoTrap = "trap";
    constexpr const char* kGatilhoDaEmulacao = "emulacao";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    /// Os hosts: seis lugares, em duas fileiras de tres, no meio do campo.
    constexpr int kHosts = 6;
    Vector2 Host(int k, float x, float y, float largura, float altura);
    /// O nome do host na tela: "A" a "F".
    char NomeDoHost(int k);

    /// O tamanho da caixa de uma VM.
    constexpr float kLarguraDaVm = 150.0f;
    constexpr float kAlturaDaVm = 86.0f;

    /// Quantas VMs a nuvem quer no instante 't': duas no hipervisor, tres na migracao e, na
    /// elasticidade, uma a mais a cada kEntreEscalas, ate o maximo do degrau.
    constexpr float kEntreEscalas = 1.6f;
    int VmsNoTempo(float t, int maximo);

    /// O primeiro host livre a partir de 'depois' (dando a volta); -1 se nenhum.
    int HostLivre(const std::vector<bool>& ocupado, int depois);

    /// A VM migrando: de 'de' a 'para', com 'u' de 0 a 1, devagar no comeco e no fim.
    Vector2 NaMigracao(const Vector2& de, const Vector2& para, float u);
    constexpr float kMigracao = 1.3f;          ///< segundos de uma migracao
    constexpr float kEntreMigracoes = 1.9f;

    constexpr float kEntreTraps = 1.3f;
    constexpr float kEntreKernels = 1.9f;
    constexpr float kVooDoTrap = 0.45f;        ///< o trap leva isto ate o hipervisor
    constexpr float kReposicao = 1.4f;         ///< a VM derrubada volta depois disto

    /// Os placares: "Hipervisor: 3 VMs em 6 hosts", "VM 2: instrução privilegiada → trap",
    /// "Migração ao vivo: VM 2, do host A para o E", "Carga subindo: 3 → 4 VMs".
    std::string Placar(int vms);
    std::string PlacarDoTrap(int vm);
    std::string PlacarDaMigracao(int vm, int de, int para);
    std::string PlacarDaEscala(int de, int para);

    /// O que o Dificil muda: mais VMs no fim, e convidados mais apressados.
    struct Degrau {
        int maisVms = 5;
        float cadencia = 1.0f;   ///< o que multiplica o tempo entre traps e entre kernels
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
