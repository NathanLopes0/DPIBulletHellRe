#!/usr/bin/env bash
# O JOGO PARA LINUX, num .tar.gz que roda sem instalar o SDL2 na maquina de quem joga.
#
#   bash Ferramentas/executavel-linux.sh
#
# Precisa de: cmake, ninja, gcc/g++, curl e os cabecalhos que o SDL2 usa para falar com
# o sistema (no Ubuntu: apt install libx11-dev libxext-dev libxrandr-dev libxcursor-dev
# libxi-dev libxss-dev libxkbcommon-dev libwayland-dev libdecor-0-dev libasound2-dev
# libpulse-dev libpipewire-0.3-dev libdbus-1-dev libudev-dev libgl-dev libegl-dev).
#
# Por que compilar o SDL2 aqui em vez de usar o da distribuicao: o SDL2 do Ubuntu puxa
# umas 80 bibliotecas (tiff, webp, fluidsynth, jack...). Este sai com o minimo - PNG
# pelo stb_image, OGG/MP3/FLAC/WAV pelos decodificadores embutidos do SDL_mixer, o
# freetype e o harfbuzz embutidos do SDL_ttf - e o SDL2 abre X11, Wayland, ALSA,
# PulseAudio e PipeWire so na hora (dlopen), com o que a maquina tiver.
# Na primeira vez baixa e compila tudo em build-pacotes/sdl-linux/ (uns minutos).
#
# Saida: dist/DPIBulletHellRe-linux.tar.gz, com
#   DPIBulletHellRe   o jogo (procura as bibliotecas em lib/, ao lado dele)
#   lib/              libSDL2, libSDL2_image, libSDL2_mixer, libSDL2_ttf
#   Assets/           os dados do jogo
# Roda em distribuicoes com a glibc igual ou mais nova que a da maquina que compilou.

set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OBRA="$RAIZ/build-pacotes"
DEPS="$OBRA/sdl-linux"
PREFIXO="$DEPS/instalado"
BUILD="$OBRA/linux"
DIST="$RAIZ/dist"
PACOTE="$DIST/DPIBulletHellRe-linux"

SDL=2.32.10; IMAGE=2.8.8; MIXER=2.8.1; TTF=2.24.0

compilar() { # repositorio pacote versao opcoes...
    local repo="$1" nome="$2" versao="$3"; shift 3
    [ -f "$DEPS/$nome-$versao.feito" ] && return
    echo "== $nome $versao"
    if [ ! -d "$DEPS/$nome-$versao" ]; then
        curl -fsSL --retry 4 -o "$DEPS/$nome.tar.gz" \
            "https://github.com/libsdl-org/$repo/releases/download/release-$versao/$nome-$versao.tar.gz"
        tar -xzf "$DEPS/$nome.tar.gz" -C "$DEPS"
        rm -f "$DEPS/$nome.tar.gz"
    fi
    cmake -S "$DEPS/$nome-$versao" -B "$DEPS/$nome-$versao/obra" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIXO" \
        -DCMAKE_PREFIX_PATH="$PREFIXO" -DCMAKE_INSTALL_LIBDIR=lib "$@" >/dev/null
    cmake --build "$DEPS/$nome-$versao/obra"
    cmake --install "$DEPS/$nome-$versao/obra" >/dev/null
    rm -rf "$DEPS/$nome-$versao/obra"
    touch "$DEPS/$nome-$versao.feito"
}

mkdir -p "$DEPS"
compilar SDL SDL2 "$SDL" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
compilar SDL_image SDL2_image "$IMAGE" -DSDL2IMAGE_BACKEND_STB=ON -DSDL2IMAGE_VENDORED=OFF \
    -DSDL2IMAGE_AVIF=OFF -DSDL2IMAGE_JXL=OFF -DSDL2IMAGE_TIF=OFF -DSDL2IMAGE_WEBP=OFF \
    -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_TESTS=OFF
compilar SDL_mixer SDL2_mixer "$MIXER" -DSDL2MIXER_VENDORED=OFF -DSDL2MIXER_SAMPLES=OFF \
    -DSDL2MIXER_VORBIS=STB -DSDL2MIXER_FLAC_LIBFLAC=OFF -DSDL2MIXER_FLAC_DRFLAC=ON \
    -DSDL2MIXER_MP3_MPG123=OFF -DSDL2MIXER_MP3_MINIMP3=ON -DSDL2MIXER_OPUS=OFF \
    -DSDL2MIXER_MOD=OFF -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF -DSDL2MIXER_WAVPACK=OFF \
    -DSDL2MIXER_GME=OFF
compilar SDL_ttf SDL2_ttf "$TTF" -DSDL2TTF_VENDORED=ON -DSDL2TTF_HARFBUZZ=ON \
    -DSDL2TTF_SAMPLES=OFF

echo "== compilando o jogo"
# RPATH so com $ORIGIN/lib (sem o caminho desta maquina), e RPATH em vez de RUNPATH
# (--disable-new-dtags) para valer tambem na libSDL2 que a libSDL2_image pede.
cmake -S "$RAIZ" -B "$BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$PREFIXO" \
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DCMAKE_INSTALL_RPATH='$ORIGIN/lib' \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,--disable-new-dtags" >/dev/null
cmake --build "$BUILD" --target DPIBulletHellRe

echo "== montando o pacote"
rm -rf "$PACOTE" "$PACOTE.tar.gz"
mkdir -p "$PACOTE/lib"
cp "$BUILD/DPIBulletHellRe" "$PACOTE/"
strip "$PACOTE/DPIBulletHellRe"
# So o nome que o executavel pede (o soname), com o arquivo de verdade por tras.
for b in libSDL2-2.0.so.0 libSDL2_image-2.0.so.0 libSDL2_mixer-2.0.so.0 libSDL2_ttf-2.0.so.0; do
    cp -L "$PREFIXO/lib/$b" "$PACOTE/lib/$b"
    strip --strip-unneeded "$PACOTE/lib/$b"
done
# Confere que nada mais do SDL veio do sistema.
if ldd "$PACOTE/DPIBulletHellRe" | grep -E "SDL|not found" | grep -v "$PACOTE/lib/"; then
    echo "!!! o executavel ainda pede bibliotecas de fora do pacote (acima)"; exit 1
fi
cp -r "$RAIZ/Assets" "$PACOTE/"
rm -f "$PACOTE/Assets/ideias.txt"
cat > "$PACOTE/LEIA-ME.txt" <<'TXT'
DPI Bullet Hell - Linux (64 bits)

Para jogar: ./DPIBulletHellRe  (ou dois cliques, se o gerenciador de arquivos
deixar executar). Mantenha as pastas Assets e lib ao lado do executavel.
O progresso fica na pasta Saves, criada ali mesmo na primeira vez.
TXT
tar -czf "$PACOTE.tar.gz" -C "$DIST" "DPIBulletHellRe-linux"
echo "== pronto: $PACOTE.tar.gz ($(du -h "$PACOTE.tar.gz" | cut -f1))"
