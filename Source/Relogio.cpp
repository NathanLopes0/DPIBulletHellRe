//
// A hora do sistema, isolada num lugar so.
//

#include "Relogio.h"

#include <cstdio>

namespace Relogio {

std::string Formatar(const std::time_t instante) {

    std::tm local{};
#ifdef _WIN32
    if (localtime_s(&local, &instante) != 0) return "";
#else
    if (localtime_r(&instante, &local) == nullptr) return "";
#endif

    char buffer[32];
    const int escritos = std::snprintf(buffer, sizeof(buffer),
                                       "%04d-%02d-%02d %02d:%02d:%02d",
                                       local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                                       local.tm_hour, local.tm_min, local.tm_sec);

    if (escritos <= 0 || escritos >= static_cast<int>(sizeof(buffer))) return "";
    return buffer;
}

std::string Agora() {
    return Formatar(std::time(nullptr));
}

}
