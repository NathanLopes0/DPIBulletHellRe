//
// O codigo secreto do menu inicial.
//

#include "CodigoSecreto.h"

#include <algorithm>

namespace CodigoSecreto {

    bool Sequencia::Tocar(const Toque toque) {
        mToques.push_back(toque);
        while (mToques.size() > kCodigo.size()) mToques.pop_front();
        if (mToques.size() == kCodigo.size() && std::equal(mToques.begin(), mToques.end(), kCodigo.begin())) {
            mToques.clear();
            return true;
        }
        return false;
    }

    int Sequencia::Certos() const {
        // O maior fim dos toques que e um comeco do codigo.
        const int n = static_cast<int>(mToques.size());
        for (int k = std::min(n, static_cast<int>(kCodigo.size()) - 1); k > 0; --k) {
            if (std::equal(mToques.end() - k, mToques.end(), kCodigo.begin())) return k;
        }
        return 0;
    }
}
