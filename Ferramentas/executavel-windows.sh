#!/usr/bin/env bash
# O JOGO PARA WINDOWS, gerado no Linux: compila o DPIBulletHellRe.exe com o MinGW-w64
# e monta um .zip que roda em qualquer Windows 10/11 de 64 bits, sem instalar nada.
#
#   bash Ferramentas/executavel-windows.sh
#
# Precisa de: cmake, ninja, mingw-w64 (apt install mingw-w64), curl, zip.
# Baixa as bibliotecas do SDL2 para MinGW (as versoes abaixo) na primeira vez e as
# guarda em build-pacotes/sdl-windows/.
#
# Saida: dist/DPIBulletHellRe-windows.zip, com
#   DPIBulletHellRe.exe   o jogo (abre sem a janela preta do console)
#   *.dll                 o SDL2 e o que ele usa (imagens, som, fontes) e as threads do MinGW
#   Assets/               os dados do jogo
# O jogo acha o Assets ao lado do .exe e cria a pasta Saves/ ali mesmo.

set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OBRA="$RAIZ/build-pacotes"
DEPS="$OBRA/sdl-windows"
BUILD="$OBRA/windows"
DIST="$RAIZ/dist"
PACOTE="$DIST/DPIBulletHellRe-windows"

SDL=2.32.10; IMAGE=2.8.8; MIXER=2.8.1; TTF=2.24.0
ALVO=x86_64-w64-mingw32

baixar() { # repositorio pacote versao
    local arquivo="$2-devel-$3-mingw.tar.gz"
    if [ ! -d "$DEPS/$2-$3" ]; then
        echo "== baixando $arquivo"
        curl -fsSL --retry 4 -o "$DEPS/$arquivo" \
            "https://github.com/libsdl-org/$1/releases/download/release-$3/$arquivo"
        tar -xzf "$DEPS/$arquivo" -C "$DEPS"
        rm -f "$DEPS/$arquivo"
    fi
    PREFIXOS="${PREFIXOS:+$PREFIXOS;}$DEPS/$2-$3/$ALVO"
}

mkdir -p "$DEPS"
PREFIXOS=""
baixar SDL       SDL2       "$SDL"
baixar SDL_image SDL2_image "$IMAGE"
baixar SDL_mixer SDL2_mixer "$MIXER"
baixar SDL_ttf   SDL2_ttf   "$TTF"

echo "== compilando"
cmake -S "$RAIZ" -B "$BUILD" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$RAIZ/Ferramentas/mingw64.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$PREFIXOS" \
    -DDPI_SEM_CONSOLE=ON >/dev/null
cmake --build "$BUILD" --target DPIBulletHellRe

echo "== montando o pacote"
rm -rf "$PACOTE" "$PACOTE.zip"
mkdir -p "$PACOTE"
cp "$BUILD/DPIBulletHellRe.exe" "$PACOTE/"
x86_64-w64-mingw32-strip "$PACOTE/DPIBulletHellRe.exe"
# As DLLs do SDL2 e das bibliotecas que ele carrega (png, jpeg, ogg, mp3, freetype...).
for d in "$DEPS"/*/"$ALVO"/bin/*.dll; do cp "$d" "$PACOTE/"; done
# O libstdc++ ja entra no .exe (CMakeLists.txt); falta a biblioteca de threads do MinGW.
for d in libwinpthread-1.dll; do
    caminho="$(x86_64-w64-mingw32-g++-posix -print-file-name=$d)"
    [ -f "$caminho" ] || caminho="$(find /usr/lib/gcc/$ALVO /usr/$ALVO -name "$d" -path '*posix*' 2>/dev/null | head -1)"
    [ -f "$caminho" ] || caminho="$(find /usr/lib/gcc/$ALVO /usr/$ALVO -name "$d" 2>/dev/null | head -1)"
    [ -f "$caminho" ] || { echo "nao achei $d"; exit 1; }
    cp "$caminho" "$PACOTE/"
done
cp -r "$RAIZ/Assets" "$PACOTE/"
rm -f "$PACOTE/Assets/ideias.txt"
cat > "$PACOTE/LEIA-ME.txt" <<'TXT'
DPI Bullet Hell - Windows

Para jogar: de dois cliques em DPIBulletHellRe.exe.
Mantenha a pasta Assets ao lado do .exe. O progresso fica na pasta Saves,
criada ali mesmo na primeira vez (copie-a para levar o progresso junto).

Se o Windows avisar "o Windows protegeu o computador", clique em
"Mais informacoes" e depois em "Executar assim mesmo": o aviso aparece
para todo programa que nao tem assinatura digital paga.
TXT
(cd "$DIST" && zip -qr "DPIBulletHellRe-windows.zip" "DPIBulletHellRe-windows")
echo "== pronto: $PACOTE.zip ($(du -h "$PACOTE.zip" | cut -f1))"
