//
// A gramatica livre de contexto da fase 2 do Vladimir.
//

#include "Gramatica.h"

namespace Gramatica {

    namespace {
        /// O epsilon e a seta, em UTF-8.
        constexpr const char* kEpsilon = "\xCE\xB5";
        constexpr const char* kSeta = " \xE2\x86\x92 ";
    }

    int SortearDerivacoes(const std::uint32_t sorteio) {
        const int faixa = kMaisDerivacoes - kMenosDerivacoes + 1;
        return kMenosDerivacoes + static_cast<int>(sorteio % static_cast<std::uint32_t>(faixa));
    }

    std::string Derivar(const std::string& forma) {
        const auto s = forma.find('S');
        if (s == std::string::npos) return forma;
        return forma.substr(0, s) + "aSb" + forma.substr(s + 1);
    }

    std::string Esvaziar(const std::string& forma) {
        const auto s = forma.find('S');
        if (s == std::string::npos) return forma;
        return forma.substr(0, s) + forma.substr(s + 1);
    }

    std::string Passo(const std::string& antes, const std::string& depois) {
        return (antes.empty() ? std::string(kEpsilon) : antes) + kSeta
             + (depois.empty() ? std::string(kEpsilon) : depois);
    }

    std::string Palavra(const int n) {
        return std::string(static_cast<size_t>(n < 0 ? 0 : n), 'a')
             + std::string(static_cast<size_t>(n < 0 ? 0 : n), 'b');
    }
}
