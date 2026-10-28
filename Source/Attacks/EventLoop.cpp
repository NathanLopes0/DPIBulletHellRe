//
// A repescagem do Lucas: o event loop do JavaScript.
//

#include "EventLoop.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>

namespace EventLoop {

    std::vector<Linha> Gerar(Variacao::Sorteio& s) {
        static const std::vector<std::string> kLogs = {"oi", "carregando", "pronto", "42", "teste"};
        std::vector<Linha> script;
        int cbs = 0, ps = 0, rs = 0;
        const auto timeout = [&](const float atraso) { return Linha{Tipo::Timeout, atraso, "cb" + std::to_string(++cbs)}; };
        const auto then = [&]() { return Linha{Tipo::Then, 0.0f, "p" + std::to_string(++ps)}; };
        // A PEGADINHA, sempre: um log, o setTimeout(cb, 0) e, depois dele, o .then.
        script.push_back({Tipo::Log, 0.0f, s.Um(kLogs)});
        script.push_back(timeout(0.0f));
        script.push_back(then());
        // E MAIS DUAS OU TRES LINHAS, de qualquer tipo.
        const int mais = s.Inteiro(2, 3);
        for (int k = 0; k < mais; ++k) {
            switch (s.Inteiro(0, 3)) {
                case 0: script.push_back({Tipo::Log, 0.0f, s.Um(kLogs)}); break;
                case 1: script.push_back(timeout(s.Real(0.4f, 1.6f))); break;
                case 2: script.push_back(then()); break;
                default: script.push_back({Tipo::Fetch, s.Real(0.6f, 1.4f), "r" + std::to_string(++rs)}); break;
            }
        }
        // A ORDEM DO MEIO PARA A FRENTE E SORTEADA: o log e a pegadinha abrem.
        std::vector<Linha> resto(script.begin() + 3, script.end());
        s.Embaralhar(resto);
        std::copy(resto.begin(), resto.end(), script.begin() + 3);
        return script;
    }

    std::string Codigo(const Linha& l) {
        switch (l.tipo) {
            case Tipo::Log:     return "console.log(\"" + l.nome + "\")";
            case Tipo::Timeout: return "setTimeout(" + l.nome + ", " + std::to_string(static_cast<int>(std::lround(l.atraso * 1000.0f))) + ")";
            case Tipo::Then:    return "Promise.resolve().then(" + l.nome + ")";
            case Tipo::Fetch:   return "fetch(\"/api\").then(" + l.nome + ")";
        }
        return "";
    }

    std::string Placar(const Linha& l) {
        switch (l.tipo) {
            case Tipo::Log:     return Codigo(l) + "\nna pilha de chamadas";
            case Tipo::Timeout: return Codigo(l) + "\ntimer nas Web APIs";
            case Tipo::Then:    return Codigo(l) + "\n" + l.nome + " na fila de microtarefas";
            case Tipo::Fetch:   return Codigo(l) + "\nrequisi\xc3\xa7\xc3\xa3o nas Web APIs";
        }
        return "";
    }

    std::string PlacarDoCallback(const Linha& l) {
        if (l.tipo == Tipo::Log) return Codigo(l);
        return l.nome + "()\n" + (l.tipo == Tipo::Timeout ? "da fila de tarefas" : "da fila de microtarefas");
    }

    Agenda Simular(const std::vector<Linha>& script, const float porLinha, const float porCallback) {
        Agenda a;
        // O SCRIPT: uma linha por vez na pilha. Quem deixa algo para depois
        // registra agora.
        std::vector<int> deItem(script.size(), -1);
        for (size_t i = 0; i < script.size(); ++i) {
            const float t = static_cast<float>(i) * porLinha;
            a.execucoes.push_back({t, static_cast<int>(i), false, Fila::Nenhuma});
            const Linha& l = script[i];
            if (l.tipo == Tipo::Log) continue;
            Item it;
            it.linha = static_cast<int>(i);
            it.registrado = t;
            it.fila = (l.tipo == Tipo::Timeout) ? Fila::Tarefa : Fila::Micro;
            // O .then de uma promessa ja resolvida entra na fila na hora; o
            // timer e o fetch, quando acabam.
            it.naFila = (l.tipo == Tipo::Then) ? t : t + l.atraso;
            it.rodou = -1.0f;
            deItem[i] = static_cast<int>(a.itens.size());
            a.itens.push_back(it);
        }

        // O LOOP: pilha vazia, todas as microtarefas prontas; depois uma tarefa.
        float t = static_cast<float>(script.size()) * porLinha;
        const auto proxima = [&](const Fila f, const float agora) {
            int melhor = -1;
            for (size_t k = 0; k < a.itens.size(); ++k) {
                const Item& it = a.itens[k];
                if (it.fila != f || it.rodou >= 0.0f || it.naFila > agora + 1e-4f) continue;
                if (melhor < 0 || it.naFila < a.itens[static_cast<size_t>(melhor)].naFila) melhor = static_cast<int>(k);
            }
            return melhor;
        };
        for (;;) {
            int k = proxima(Fila::Micro, t);
            if (k < 0) k = proxima(Fila::Tarefa, t);
            if (k >= 0) {
                Item& it = a.itens[static_cast<size_t>(k)];
                it.rodou = t;
                a.execucoes.push_back({t, it.linha, true, it.fila});
                t += porCallback;
                continue;
            }
            // NADA PRONTO: o loop espera o proximo timer ou resposta.
            float espera = std::numeric_limits<float>::max();
            for (const Item& it : a.itens) if (it.rodou < 0.0f) espera = std::min(espera, it.naFila);
            if (espera == std::numeric_limits<float>::max()) break;
            t = std::max(t, espera);
        }
        a.fim = t;
        return a;
    }

    std::array<Retangulo, 4> Caixas(const float largura) {
        const float meio = largura / 2.0f;
        return {{
            {meio - 45.0f, kYDoLoop - 45.0f, 90.0f, 100.0f},   // a pilha, no meio do loop
            {40.0f, 70.0f, 320.0f, 110.0f},                    // as Web APIs
            {40.0f, 360.0f, 380.0f, 60.0f},                    // a fila de microtarefas
            {largura - 420.0f, 360.0f, 380.0f, 60.0f},         // a fila de tarefas
        }};
    }

    Ponto Posicao(const Lugar lugar, const int posicao, const float largura) {
        const auto caixas = Caixas(largura);
        constexpr float kEntreFichas = 52.0f;
        const auto p = static_cast<float>(posicao);
        switch (lugar) {
            case Lugar::Pilha: {
                const Retangulo& r = caixas[0];
                return {r.x + r.w / 2.0f, r.y + r.h / 2.0f};
            }
            case Lugar::Apis: {
                const Retangulo& r = caixas[1];
                return {r.x + 50.0f + p * 72.0f, r.y + 50.0f};
            }
            case Lugar::Micro: {   // a cabeca a direita, perto da pilha
                const Retangulo& r = caixas[2];
                return {r.x + r.w - 35.0f - p * kEntreFichas, r.y + r.h / 2.0f};
            }
            case Lugar::Tarefa: {  // a cabeca a esquerda, perto da pilha
                const Retangulo& r = caixas[3];
                return {r.x + 35.0f + p * kEntreFichas, r.y + r.h / 2.0f};
            }
            default:
                return {largura / 2.0f, kYDoLoop};
        }
    }

    Estado Onde(const std::vector<Linha>& script, const Agenda& a, const int linha, const float t,
                const float porLinha, const float porCallback) {
        Estado e;
        const float inicio = static_cast<float>(linha) * porLinha;
        if (t < inicio) return e;
        if (t < inicio + porLinha) { e.lugar = Lugar::Pilha; return e; }
        const Item* item = nullptr;
        for (const Item& it : a.itens) if (it.linha == linha) item = &it;
        if (!item) return e;   // o console.log nao deixa nada
        if (t >= item->rodou + porCallback) return e;
        if (t >= item->rodou) { e.lugar = Lugar::Pilha; return e; }
        if (t < item->naFila) {
            // O TIMER CORRE NAS WEB APIS; a vaga e a ordem de registro.
            e.lugar = Lugar::Apis;
            for (const Item& it : a.itens) {
                if (it.linha < linha && script[static_cast<size_t>(it.linha)].tipo != Tipo::Then) ++e.posicao;
            }
            const float atraso = std::max(item->naFila - item->registrado, 1e-3f);
            e.falta = std::clamp((item->naFila - t) / atraso, 0.0f, 1.0f);
            return e;
        }
        // NA FILA: na frente, quem o loop vai buscar antes e ja esta la. O
        // .then entra na fila quando a linha dele roda, mas a ficha so chega
        // quando a linha sai da pilha - e e a ficha que conta a vaga.
        e.lugar = (item->fila == Fila::Micro) ? Lugar::Micro : Lugar::Tarefa;
        for (const Item& it : a.itens) {
            if (it.fila != item->fila || it.linha == linha) continue;
            const bool antes = it.naFila < item->naFila || (it.naFila == item->naFila && it.linha < linha);
            const float chegou = std::max(it.naFila, static_cast<float>(it.linha + 1) * porLinha);
            if (antes && chegou <= t && it.rodou > t) ++e.posicao;
        }
        return e;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.porLinha = Dificuldade::Escolher(n, kPorLinha, 0.35f);
        d.porCallback = Dificuldade::Escolher(n, kPorCallback, 0.42f);
        return d;
    }
}
