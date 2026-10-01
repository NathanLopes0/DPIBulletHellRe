//
// Montagem e busca de caminhos de arquivo.
//

#include "Caminhos.h"

namespace {

    /// Troca '\' por '/'. As duas funcionam no Windows; uma so deixa o log legivel.
    std::string Normalizar(std::string s) {
        for (char& c : s) if (c == '\\') c = '/';
        return s;
    }

    bool EhBarra(const char c) { return c == '/' || c == '\\'; }

}

namespace Caminhos {

std::string Juntar(const std::string& base, const std::string& relativo) {

    if (base.empty())     return Normalizar(relativo);
    if (relativo.empty()) return Normalizar(base);

    std::string a = Normalizar(base);
    std::string b = Normalizar(relativo);

    while (!a.empty() && EhBarra(a.back()))   a.pop_back();
    while (!b.empty() && EhBarra(b.front()))  b.erase(b.begin());

    // Os dois podiam ser so barras: "/" + "/" nao tem o que juntar.
    if (a.empty()) return "/" + b;
    if (b.empty()) return a;

    return a + "/" + b;
}

std::string DiretorioPai(const std::string& caminho) {

    std::string s = Normalizar(caminho);
    while (!s.empty() && EhBarra(s.back())) s.pop_back();

    const auto corte = s.find_last_of('/');
    if (corte == std::string::npos) return "";   // nao ha nivel acima

    // A raiz e um caso proprio: o pai de "/a" e "/", e nao a string vazia, senao
    // a subida perderia a referencia de onde estava.
    if (corte == 0) return "/";

    return s.substr(0, corte);
}

std::string ProcurarSubindo(const std::string& partida, const int maxNiveis,
                            const Predicado& serve) {

    if (!serve) return "";

    std::string atual = Normalizar(partida);
    for (int nivel = 0; nivel <= maxNiveis; ++nivel) {
        if (atual.empty()) break;
        if (serve(atual)) return atual;

        const std::string pai = DiretorioPai(atual);
        if (pai == atual) break;   // chegou na raiz e nao sai mais de la
        atual = pai;
    }
    return "";
}

}
