#!/usr/bin/env bash
# O JOGO PARA O GABINETE: um .tar.gz que roda no Ubuntu 20.04 do arcade, sem internet e sem
# instalar nada.
#
#   bash Ferramentas/executavel-gabinete.sh
#
# RODE NUM UBUNTU 20.04 (uma maquina, um container ou um chroot feito com debootstrap focal):
# o executavel usa a glibc de quem compilou, e roda nela ou numa mais nova - compilado num
# Ubuntu mais novo, ele nao abre no gabinete.
#
# Precisa de: cmake 3.26 ou mais novo (o do 20.04 e velho: pip3 install cmake ninja), ninja,
# g++-10 e os cabecalhos que o SDL2 usa para falar com o sistema:
#   apt install g++-10 libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev
#       libxss-dev libxfixes-dev libxinerama-dev libxkbcommon-dev libwayland-dev
#       wayland-protocols libegl-dev libgl-dev libgles-dev libasound2-dev libpulse-dev
#       libdbus-1-dev libudev-dev libjpeg-dev libpng-dev
# (o libjpeg-dev e o libpng-dev so para o CMake do SDL2_image achar o que ele procura: as
# imagens saem pelo stb_image, e o jogo nao leva nenhuma das duas)
#
# A diferenca para o executavel-linux.sh: este compila com DPI_GABINETE (CMakeLists.txt) -
# o jogo ja abre no modo gabinete (tela cheia, sem cursor, sem saida acidental, volta ao
# menu sozinho), sem o modo de simulacao, sem as fotos e sem as teclas de desenvolvedor -, e
# o pacote leva so o que o jogo le, mais o jogar.sh, que reabre o jogo se ele cair.
# O SDL2 e compilado aqui do mesmo jeito minimo do executavel-linux.sh.
#
# Saida: dist/DPIBulletHellRe-gabinete.tar.gz, com
#   jogar.sh          abre o jogo e o reabre se cair (Ctrl + Esc segurado sai de vez)
#   instalar.sh       o icone na area de trabalho e no menu; --abrir-ao-ligar abre o jogo no login
#   DPIBulletHellRe   o jogo
#   lib/              libSDL2, libSDL2_image, libSDL2_mixer, libSDL2_ttf
#   Assets/           os dados do jogo

set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
OBRA="$RAIZ/build-pacotes"
DEPS="$OBRA/sdl-gabinete"
PREFIXO="$DEPS/instalado"
BUILD="$OBRA/gabinete"
DIST="$RAIZ/dist"
NOME="DPIBulletHellRe-gabinete"
PACOTE="$DIST/$NOME"

SDL=2.32.10; IMAGE=2.8.8; MIXER=2.8.1; TTF=2.24.0

if ! grep -q 'VERSION_ID="20.04"' /etc/os-release 2>/dev/null; then
    echo "!!! isto nao e um Ubuntu 20.04: o executavel pode nao abrir no gabinete (ver o cabecalho)"
fi
export CC="${CC:-gcc-10}" CXX="${CXX:-g++-10}"

compilar() { # repositorio pacote versao opcoes...
    local repo="$1" nome="$2" versao="$3"; shift 3
    [ -f "$DEPS/$nome-$versao.feito" ] && return
    echo "== $nome $versao"
    if [ ! -d "$DEPS/$nome-$versao" ]; then
        # O codigo baixado pelo executavel-linux.sh serve: e o mesmo.
        if [ -f "$OBRA/sdl-linux/$nome-$versao/CMakeLists.txt" ]; then
            cp -r "$OBRA/sdl-linux/$nome-$versao" "$DEPS/"
        else
            curl -fsSL --retry 4 -o "$DEPS/$nome.tar.gz" \
                "https://github.com/libsdl-org/$repo/releases/download/release-$versao/$nome-$versao.tar.gz"
            tar -xzf "$DEPS/$nome.tar.gz" -C "$DEPS"
            rm -f "$DEPS/$nome.tar.gz"
        fi
    fi
    rm -rf "$DEPS/$nome-$versao/obra"
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

echo "== compilando o jogo (DPI_GABINETE)"
# RPATH so com $ORIGIN/lib, como no executavel-linux.sh.
cmake -S "$RAIZ" -B "$BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DDPI_GABINETE=ON \
    -DCMAKE_PREFIX_PATH="$PREFIXO" \
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DCMAKE_INSTALL_RPATH='$ORIGIN/lib' \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,--disable-new-dtags" >/dev/null
cmake --build "$BUILD" --target DPIBulletHellRe

echo "== montando o pacote"
rm -rf "$PACOTE" "$PACOTE.tar.gz"
mkdir -p "$PACOTE/lib"
cp "$BUILD/DPIBulletHellRe" "$PACOTE/"
strip "$PACOTE/DPIBulletHellRe"
for b in libSDL2-2.0.so.0 libSDL2_image-2.0.so.0 libSDL2_mixer-2.0.so.0 libSDL2_ttf-2.0.so.0; do
    cp -L "$PREFIXO/lib/$b" "$PACOTE/lib/$b"
    strip --strip-unneeded "$PACOTE/lib/$b"
done
if ldd "$PACOTE/DPIBulletHellRe" | grep -E "SDL|not found" | grep -v "$PACOTE/lib/"; then
    echo "!!! o executavel ainda pede bibliotecas de fora do pacote (acima)"; exit 1
fi
# So o que o jogo le: sem o arquivo do Aseprite e sem anotacoes.
cp -r "$RAIZ/Assets" "$PACOTE/"
find "$PACOTE/Assets" -type f \( -name '*.ase' -o -name '*.aseprite' -o -name 'ideias.txt' \) -delete

cat > "$PACOTE/jogar.sh" <<'SH'
#!/bin/sh
# Abre o DPI Bullet Hell no modo gabinete e o reabre se ele cair.
# Para sair de vez: Ctrl + Esc, segurado por 2 segundos (num teclado ligado a maquina).
cd "$(dirname "$(readlink -f "$0")")" || exit 1
mkdir -p Saves
while true; do
    ./DPIBulletHellRe "$@"
    codigo=$?
    # Saida pedida pelo operador (ou sem tela para abrir): nao reabre.
    [ "$codigo" -eq 0 ] && exit 0
    echo "$(date '+%Y-%m-%d %H:%M:%S') o jogo fechou com o codigo $codigo; reabrindo" >> Saves/quedas.log
    sleep 2
done
SH

cat > "$PACOTE/instalar.sh" <<'SH'
#!/bin/sh
# Poe o DPI Bullet Hell no menu de aplicativos e na area de trabalho.
#   ./instalar.sh                    o icone
#   ./instalar.sh --abrir-ao-ligar   o icone, e o jogo abre sozinho quando a sessao comeca
# Nao precisa de internet nem de senha de administrador.
set -e
AQUI="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
chmod +x "$AQUI/jogar.sh" "$AQUI/DPIBulletHellRe"
ENTRADA="[Desktop Entry]
Type=Application
Name=DPI Bullet Hell
Comment=O jogo do gabinete do DPI
Exec=\"$AQUI/jogar.sh\"
Path=$AQUI
Icon=$AQUI/Assets/DPIBHTitleMainMenu.png
Terminal=false
Categories=Game;
X-GNOME-Autostart-enabled=true"
mkdir -p "$HOME/.local/share/applications"
printf '%s\n' "$ENTRADA" > "$HOME/.local/share/applications/dpibullethell.desktop"
DESKTOP="$(xdg-user-dir DESKTOP 2>/dev/null || echo "$HOME/Desktop")"
if [ -d "$DESKTOP" ]; then
    printf '%s\n' "$ENTRADA" > "$DESKTOP/dpibullethell.desktop"
    chmod +x "$DESKTOP/dpibullethell.desktop"
    # O GNOME do 20.04 so abre o atalho da area de trabalho marcado como confiavel.
    gio set "$DESKTOP/dpibullethell.desktop" metadata::trusted true 2>/dev/null || true
fi
if [ "${1:-}" = "--abrir-ao-ligar" ]; then
    mkdir -p "$HOME/.config/autostart"
    printf '%s\n' "$ENTRADA" > "$HOME/.config/autostart/dpibullethell.desktop"
    echo "O jogo vai abrir sozinho quando a sessao comecar."
fi
echo "Pronto: procure \"DPI Bullet Hell\" no menu ou na area de trabalho."
SH
chmod +x "$PACOTE/jogar.sh" "$PACOTE/instalar.sh" "$PACOTE/DPIBulletHellRe"

cat > "$PACOTE/LEIA-ME.txt" <<'TXT'
DPI Bullet Hell - gabinete (Ubuntu 20.04, 64 bits, sem internet)

Para abrir:     ./jogar.sh
                (abre em tela cheia, no modo gabinete, e reabre o jogo se ele cair)
Para o icone:   ./instalar.sh
Abrir ao ligar: ./instalar.sh --abrir-ao-ligar
                (a sessao do usuario precisa entrar sozinha: Configuracoes > Usuarios >
                Login automatico)
Para sair:      Ctrl + Esc, segurado por 2 segundos, num teclado ligado a maquina.

Mantenha as pastas Assets e lib ao lado do executavel. O progresso, o ranking e os
registros de tentativas ficam na pasta Saves, criada aqui na primeira vez: para
levar os dados para outra maquina, copie a pasta Saves.
As teclas que o painel precisa mandar estao em Documentacao/gabinete.md, no codigo.
TXT

tar -czf "$PACOTE.tar.gz" -C "$DIST" "$NOME"
echo "== pronto: $PACOTE.tar.gz ($(du -h "$PACOTE.tar.gz" | cut -f1))"
