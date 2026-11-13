//
// A fase 1 do Goulart: o escalonador de processos.
//

#include "Escalonador.h"

#include <algorithm>
#include <cmath>

namespace Escalonador {

    RoundRobin::RoundRobin(const float quantum, const float troca)
        : mQuantum(quantum > 0.0f ? quantum : 1.0f)
        , mTroca(std::max(0.0f, troca))
    {
    }

    void RoundRobin::Chegar(const Processo& p) {
        if (mProcessos.count(p.pid)) return;
        mProcessos[p.pid] = p;
        mProntos.push_back(p.pid);
    }

    std::vector<Aviso> RoundRobin::Avancar(const float agora) {
        std::vector<Aviso> avisos;
        const float dt = agora - mRelogio;
        if (dt <= 0.0f) return avisos;
        mRelogio = agora;

        // A E/S que terminou e uma interrupcao: o processo volta para o fim da fila.
        for (auto it = mBloqueados.begin(); it != mBloqueados.end();) {
            const Processo& p = mProcessos.at(*it);
            if (agora < p.acordaEm) {
                ++it;
                continue;
            }
            mProntos.push_back(p.pid);
            avisos.push_back({Evento::Acordou, p.pid});
            it = mBloqueados.erase(it);
        }

        // O que roda gasta a CPU dele.
        if (mRodando) {
            const int pid = *mRodando;
            Processo& p = mProcessos.at(pid);
            p.falta -= dt;
            if (p.tipo == Tipo::EntradaSaida) p.naRajada += dt;
            if (p.falta <= 0.0f) {
                mProcessos.erase(pid);
                mRodando.reset();
                mOciosaAte = agora + mTroca;
                avisos.push_back({Evento::Terminou, pid});
            } else if (p.tipo == Tipo::EntradaSaida && p.naRajada >= p.rajada) {
                p.naRajada = 0.0f;
                p.acordaEm = agora + kTempoDeES;
                mBloqueados.push_back(pid);
                mRodando.reset();
                mOciosaAte = agora + mTroca;
                avisos.push_back({Evento::PediuES, pid});
            } else if (agora - mInicioDoQuantum >= mQuantum) {
                if (mProntos.empty()) {
                    mInicioDoQuantum = agora;   // ninguem esperando: ganha outro quantum
                } else {
                    mProntos.push_back(pid);
                    mRodando.reset();
                    mOciosaAte = agora + mTroca;
                    avisos.push_back({Evento::Preemptou, pid});
                }
            }
        }

        // A CPU livre, passada a troca de contexto, pega o primeiro da fila.
        if (!mRodando && agora >= mOciosaAte && !mProntos.empty()) {
            mRodando = mProntos.front();
            mProntos.pop_front();
            mInicioDoQuantum = agora;
            avisos.push_back({Evento::Despachou, *mRodando});
        }
        return avisos;
    }

    bool RoundRobin::Matar(const int pid) {
        if (!mProcessos.erase(pid)) return false;
        mProntos.erase(std::remove(mProntos.begin(), mProntos.end(), pid), mProntos.end());
        mBloqueados.erase(std::remove(mBloqueados.begin(), mBloqueados.end(), pid), mBloqueados.end());
        if (mRodando && *mRodando == pid) {
            mRodando.reset();
            mOciosaAte = mRelogio + mTroca;
        }
        return true;
    }

    const Processo* RoundRobin::Achar(const int pid) const {
        const auto it = mProcessos.find(pid);
        return it == mProcessos.end() ? nullptr : &it->second;
    }

    float RoundRobin::NoQuantum() const {
        if (!mRodando) return 0.0f;
        return std::clamp((mRelogio - mInicioDoQuantum) / mQuantum, 0.0f, 1.0f);
    }

    Tipo TipoDoFork(const int k) {
        static constexpr std::array<Tipo, 3> kRodizio = {Tipo::Cpu, Tipo::EntradaSaida, Tipo::Threads};
        return kRodizio[static_cast<size_t>(((k % 3) + 3) % 3)];
    }

    Processo Novo(const int pid, const Tipo tipo) {
        Processo p;
        p.pid = pid;
        p.tipo = tipo;
        switch (tipo) {
            case Tipo::Cpu:          p.falta = 2.6f; break;
            case Tipo::EntradaSaida: p.falta = 1.6f; p.rajada = 0.4f; break;
            case Tipo::Threads:      p.falta = 2.4f; break;
        }
        return p;
    }

    std::string Cara(const int pid) {
        return "P" + std::to_string(((std::max(pid, 1) - 1) % 9) + 1);
    }

    std::string Placar(const RoundRobin& rr) {
        const auto lista = [](const auto& pids) {
            if (pids.empty()) return std::string("-");
            std::string s;
            for (const int pid : pids) s += (s.empty() ? "" : " ") + Cara(pid);
            return s;
        };
        const int decimos = static_cast<int>(std::lround(rr.Quantum() * 10.0f));
        // \xc2\xb7 e o ponto do meio.
        return "Round Robin, q = " + std::to_string(decimos / 10) + "," + std::to_string(decimos % 10) +
               " s  \xc2\xb7  CPU: " + (rr.Rodando() ? Cara(*rr.Rodando()) : std::string("ociosa")) +
               "  \xc2\xb7  prontos: " + lista(rr.Prontos()) + "  \xc2\xb7  E/S: " + lista(rr.Bloqueados());
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.quantum = 1.0f;
        d.processos = 6;
        d.entreForks = 2.2f;
        d.cadencia = 0.85f;
        return d;
    }

    float EntreInstrucoes(const Tipo tipo) {
        switch (tipo) {
            case Tipo::Cpu:          return 0.34f;
            case Tipo::EntradaSaida: return 0.2f;
            case Tipo::Threads:      return 0.32f;
        }
        return 0.3f;
    }
}
