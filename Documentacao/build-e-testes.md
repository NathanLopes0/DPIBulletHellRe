# Compilar, testar e investigar

---

## Compilar

Precisa de **CMake 3.26+**, um compilador **C++17** e as quatro bibliotecas da
SDL2: `SDL2`, `SDL2_image`, `SDL2_mixer`, `SDL2_ttf`.

```bash
cmake -S . -B build
cmake --build build -j8
```

Dois executáveis saem daí:

| | |
|---|---|
| `build/DPIBulletHellRe` | o jogo |
| `build/dpi_tests` | a suíte |

O jogo acha a pasta `Assets/` **subindo os diretórios** a partir de onde o
executável está, então rodar de dentro de `build/` ou da raiz dá na mesma. Os
saves ficam em `Saves/`, irmão de `Assets/`.

Se o CMake não acha a SDL: em Linux são os pacotes `libsdl2-dev`,
`libsdl2-image-dev`, `libsdl2-mixer-dev` e `libsdl2-ttf-dev`. No Windows, o
[tutorial de instalação](https://github.com/sagedemage/SDL2_windows_setup)
cobre o caminho.

### Executáveis para distribuir

| Script | Sai | Tem dentro |
|---|---|---|
| `Ferramentas/executavel-windows.sh` | `dist/DPIBulletHellRe-windows.zip` | `DPIBulletHellRe.exe`, as DLLs do SDL2 e `Assets/` |
| `Ferramentas/executavel-linux.sh` | `dist/DPIBulletHellRe-linux.tar.gz` | `DPIBulletHellRe`, `lib/` com o SDL2 e `Assets/` |

Os dois rodam no **Linux** e compilam em `Release`, em `build-pacotes/` (fora
do `build/` de desenvolvimento). Quem recebe o pacote não instala nada: abre o
executável, que acha o `Assets/` ao lado dele e cria o `Saves/` ali mesmo.

- **Windows**: compilação cruzada com o MinGW-w64 (`apt install mingw-w64`) e o
  arquivo de toolchain `Ferramentas/mingw64.cmake`. Na primeira vez o script
  baixa o SDL2 para MinGW das releases oficiais. O `.exe` sai com
  `DPI_SEM_CONSOLE=ON` (sem a janela preta do console) e com o runtime do C++
  dentro dele; ao lado vão as DLLs do SDL2 e a `libwinpthread-1.dll`.
- **Linux**: o script compila o próprio SDL2 (em `build-pacotes/sdl-linux/`,
  uns minutos na primeira vez) com o mínimo: imagens pelo stb_image, OGG, MP3,
  FLAC e WAV pelos decodificadores embutidos do SDL_mixer, e o freetype e o
  harfbuzz embutidos do SDL_ttf. O SDL2 abre X11, Wayland, ALSA, PulseAudio e
  PipeWire só na hora, com o que a máquina tiver. O executável procura as
  bibliotecas em `lib/` (RPATH `$ORIGIN/lib`). Roda em distribuições com a
  glibc igual ou mais nova que a da máquina que compilou: por isso o workflow
  compila no Ubuntu 22.04 (glibc 2.35).

O workflow `.github/workflows/executaveis.yml` roda os dois scripts no GitHub:
numa tag `v*` publica os pacotes numa Release; pela aba Actions (**Run
workflow**) deixa os pacotes nos *Artifacts* da execução.

O Windows avisa "o Windows protegeu o computador" em todo programa sem
assinatura digital: "Mais informações" e "Executar assim mesmo". O
`LEIA-ME.txt` do pacote diz isso a quem joga.

Para conferir o `.exe` sem um Windows, o Wine serve (o modo de simulação roda
igual; `SDL_AUDIODRIVER=dummy` se o Wine não tiver som):

```bash
cd dist/DPIBulletHellRe-windows
SDL_AUDIODRIVER=dummy xvfb-run -a wine64 ./DPIBulletHellRe.exe \
    --batalha INF213 --semente 3 --passo 0.016 --piloto segue --atira --sair-no-fim --fim-em 8
```

### Acrescentar um arquivo

`CMakeLists.txt` lista os fontes **à mão**, e um arquivo da camada pura aparece
em **dois** lugares: no alvo do jogo e no alvo dos testes.

```bash
grep -c "Source/Regras/Nota.cpp" CMakeLists.txt     # tem de dar 2
```

Esquecer o segundo dá erro de link só no `dpi_tests`, com uma mensagem que não
menciona o CMake.

---

## Rodar os testes

```bash
./build/dpi_tests
```

Algumas opções que poupam tempo:

```bash
./build/dpi_tests -tc="*nota*"          # só os casos com "nota" no nome
./build/dpi_tests -sf="*test_fases*"    # só os de um arquivo
./build/dpi_tests -ltc                  # lista os nomes sem rodar
./build/dpi_tests -s                    # mostra também as asserções que passaram
```

### O que a suíte cobre, e o que ela não cobre

Ela cobre **a parte do jogo que não depende de SDL nem de disco**: a aritmética
da nota, as regras de desbloqueio, a navegação da seleção de fases, a montagem
dos personagens, a leitura dos JSON, a planilha.

Isso é possível por causa da divisão em pares. Para cada assunto há um `X.cpp`
puro e um `XArquivo.cpp` que toca disco e SDL; só a metade pura entra no
`dpi_tests`. A metade impura é fina de propósito — quanto mais fina, menos
código fica sem teste.

Além disso, `test_arquivos_de_dados.cpp` **lê os `Assets/` de verdade**. É o que
transforma um erro de digitação num JSON em falha de suíte, em vez de numa
matéria que não abre. Também é onde moram os guardas que espelham o C++ — a
lista de fábricas de professor, por exemplo — e que falham de propósito quando
alguém acrescenta um professor pela metade.

O que **não** é coberto: desenho, áudio, entrada e o laço principal. Para esses
há a instrumentação da última seção.

---

## A disciplina de teste

> **Um teste verde não prova nada até ter sido visto falhando pelo motivo
> certo.**

Toda guarda deste projeto foi quebrada de propósito uma vez, e só foi aceita
depois de a falha dizer a coisa certa. Sem isso, um teste que nunca falhou pode
estar medindo o nada — comparando um valor consigo mesmo, ou nem chegando à
asserção.

Como fazer: estrague a linha que o teste protege, recompile, rode e **leia a
mensagem**. Ela nomeia o problema de verdade? Então desfaça e siga.

```bash
# exemplo real: baixar o expoente da curva da nota
sed -i 's/kExpoente = 1.5f/kExpoente = 1.0f/' Source/Regras/Nota.h
cmake --build build -j8 && ./build/dpi_tests -tc="*Nota*"
#   ERROR: CHECK( em99 < em90 / 2.0f ) is NOT correct!
#     values: CHECK( 0.22 <  0.2 )
sed -i 's/kExpoente = 1.0f/kExpoente = 1.5f/' Source/Regras/Nota.h
```

A mensagem acima é útil: diz qual propriedade se perdeu e com que números. Uma
que dissesse só `CHECK(false)` seria um teste pior, mesmo passando igual.

> **Quando um teste falha, pergunte primeiro se ele está certo.** Já aconteceu
> aqui de o teste estar errado e o código certo — uma asserção dizia que à
> esquerda do INF 332 estava o VISCCP, quando o INF 332 está noutra coluna. A
> falha foi a resposta, não o problema.

---

## Investigar o que não tem teste

Desenho, movimento e dificuldade não cabem em asserção, mas também não precisam
de chute. O jogo tem um **modo de simulação** (`Source/Simulacao/Simulacao.h`):
parâmetros de linha de comando que prendem a semente, fixam o passo de tempo,
põem um piloto no lugar do aluno, entram direto numa batalha e escrevem a nota.
Sem nenhum deles, o jogo é o de sempre — e o executável que se mede é o mesmo
que se joga.

Antes isto era feito copiando o repositório e remendando a cópia com ganchos de
variável de ambiente. Cada remendo procurava um trecho exato do código, e
qualquer refatoração podia derrubá-lo — uma vez sem avisar, quando a conta do
ganho mudou e o contador de acertos zerou. Morando no jogo, os ganchos passam
pelo compilador como o resto.

### Rodar sem tela

```bash
export SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy
./build/DPIBulletHellRe
```

O jogo roda inteiro, desenha e não abre janela. É o que permite medir num
servidor ou em paralelo.

### Os parâmetros

| Parâmetro | O que faz |
|---|---|
| `--batalha <n ou código>` | entra direto na batalha da matéria `n` de `materias.json` (`0` é INF110), ou de um código (`INF213`) |
| `--jogadores 1\|2` | solo ou dupla; os alunos são visitantes |
| `--orientador <código>` | o orientador do TCC, como se escolhido na tela |
| `--candidatos <códigos>` | abre a **tela que escolhe o orientador**, como se o aluno tivesse passado nessas matérias (`INF110,INF213,INF310`; a primeira com a maior nota). Com `--fotos`, é como se confere a prévia animada dos poderes |
| `--cena selecao\|boletim\|horas` | com `--candidatos`, abre a seleção de fases, o boletim ou as horas complementares no lugar da tela do orientador — a primeira matéria da lista com 100, e cada uma 8 a menos |
| `--atividades "virada,podio"` | com `--candidatos`, as horas complementares que o aluno já cumpriu — para fotografar a compra de uma optativa |
| `--dificil` | o modo Difícil; sem ele, Normal |
| `--estado <fase>` | o chefe começa nesta fase (`StateTwo`, `StateFinal`...) |
| `--semente <n>` | o sorteador global. Sem `--variacao`, é também a semente da batalha. A simulação nunca é a **prova da semana**: ela mede com a semente pedida, e não grava no ranking |
| `--variacao <n>` | a semente da batalha (`Battle::SementeDaBatalha`), separada da do sorteio |
| `--passo <s>` | segundos por quadro, sem esperar o relógio: `0.016` é igual a 62,5 quadros por segundo, só que na velocidade que a máquina der |
| `--piloto segue\|parado\|vaivem\|esquiva` | quem joga no lugar do aluno (os vieses de cada um estão em `Ferramentas/medir-dificuldade.sh`; o `esquiva` é o perfil das optativas, abaixo) |
| `--atira` | o piloto atira sempre que pode |
| `--poder` | no TCC, o orientador solta o poder sozinho quando ele serve |
| `--relatorio` | escreve a nota a cada 60 quadros de luta e a linha `FIM` |
| `--sair-no-fim` | fecha o jogo assim que a luta se decide, antes de gravar ficha ou tentativa |
| `--imortal` | a nota a zero não encerra a luta (para fotografar o fim de uma fase) |
| `--ajuste <n>` | o degrau do ajuste automático de dificuldade (`Ajuste.h`, de 0 a 2), no lugar do que sairia das reprovações seguidas. Sem ele, a simulação não tem reprovação nenhuma, e o degrau é 0 |
| `--teclas "Space@100,B@200"` | aperta teclas por roteiro: nome (do SDL) e quadro; cada uma fica apertada três quadros |
| `--fotos 380,470` e `--fotos-em <pasta>` | fotografa a tela nesses quadros (`foto_00380.bmp`...) e sai depois da última |
| `--janela 1920x1080` | a janela de verdade; o jogo continua posicionado em 1200×800 |
| `--fim-em <q>`, `--fim-aprovado`, `--fim-nota <n>` | encerra a luta no quadro `q`, aprovada ou não, com aquela nota |
| `--pausa-em <q>`, `--foco-em <q>` | abre a pausa, ou faz a janela perder o foco, no quadro `q` |

Um parâmetro de simulação que não se lê é dito no log e ignorado. Eles não vão
no script do gabinete. Com qualquer um deles, a tentativa não vai para os CSV da
pesquisa (`dados-das-tentativas.md`): uma foto que termina a luta com `--fim-em`
não vira dado do TCC.

### Medir

```bash
bash Ferramentas/medir-dificuldade.sh --batalha 1 --jogadores 2 --semente 10 --piloto segue
```

O script acrescenta o que toda medida usa (`--passo 0.016 --atira --relatorio
--sair-no-fim`, sem vídeo e sem som) e mostra o relatório:

```
q00060 nota=40.0000 sofridos=0
...
FIM nota=91.0038 acertos_sofridos=7 acertos_dados=489 aprovado=1
```

`q` é o quadro de luta, `sofridos` os acertos que a dupla levou; no `FIM`,
`acertos_dados` são os tiros que o professor ouviu. **Use várias sementes**: com
dois jogadores há mais colisões, e cada uma consome números do mesmo sorteador
— uma corrida mede o sorteio daquela corrida, e não a dificuldade.

### A linha de base

`Ferramentas/linha-de-base.sh` roda **74 batalhas** — as quinze matérias (as
optativas INF 425 e INF 429 inclusive), solo e
dupla, Normal e Difícil, e o TCC com cada um dos catorze orientadores — e compara
a nota, quadro a quadro, com a gravada em `Ferramentas/linha-de-base/`:

```bash
bash Ferramentas/linha-de-base.sh               # identicas=74 diferentes=0
bash Ferramentas/linha-de-base.sh --atualizar   # grava a de agora
```

**Uma refatoração tem de dar 74 idênticas, byte a byte.** Uma mudança de
balanceamento muda algumas de propósito: confira quais mudaram, e quanto — o
script mostra a linha `FIM` de antes e de agora —, e só então grave a nova com
`--atualizar`, no mesmo commit. Cada batalha ocupa uns 2,5 GB; `PARALELO=3` é o
padrão.

`Ferramentas/parado-reprova.sh` é o outro portão: o piloto parado não pode
aprovar em professor nenhum.

### O perfil das optativas

O piloto que segue o chefe é um mau termômetro para as optativas: elas pedem
que se pense na mecânica (a imagem que vem de baixo, os próprios tiros que
acendem a convolução, o neurônio que se derruba, a hora de ficar quieto), e ele
só anda atrás do professor, atirando. Para elas há um segundo perfil,
`--piloto esquiva`: o mesmo alvo do `segue`, mas a cada quadro ele olha os tiros
que machucam — meio segundo adiante — e escolhe o rumo mais seguro
(`Simulacao::Desviar`). É o teto de quem **só desvia**: vê tudo e reage na hora.
A linha de base continua com o `segue`; o `esquiva` é a segunda leitura de uma
optativa. Na INF 429, sozinho, com as sementes 101 a 103:

| Piloto | Normal | Difícil |
|---|---|---|
| `segue` | 27 a 39 (reprova; a fase 2 o derruba) | 0 a 2 |
| `esquiva` | 79 a 86 | 63 a 73 |

Quem joga de verdade fica entre os dois: desvia pior que o `esquiva`, mas pode
entender a fase — e é isso que a INF 429 cobra.

### Fotografar

```bash
./build/DPIBulletHellRe --batalha 1 --jogadores 2 --semente 11 --passo 0.016 \
    --piloto segue --atira --teclas "N@400" --fotos 404,414 --fotos-em /tmp/fotos
```

**Fotografe por instante de jogo, não por quadro de tela**: o carregamento dos
pools consome quadros antes de a batalha começar, então quadro não é tempo — mas,
com a semente e o passo presos, o mesmo quadro é sempre o mesmo instante. Para
ver uma tela que só aparece depois de uma tela de menu, o roteiro de teclas faz o
caminho: `--teclas "Space@100,8@200,9@215,..."` digita uma matrícula.

### Contar

Um `std::printf` com `std::fflush(stdout)` num ponto de passagem responde quase
qualquer pergunta de balanceamento. Foi assim que o expoente da curva da nota foi
escolhido: contando quantos tiros chegam no professor numa batalha inteira, com
o tiro preso, contra os quatro chefes. O relatório da simulação
(`Scenes/Battle/RelatorioDaSimulacao.h`) é um ouvinte da batalha: uma conta nova
que valha guardar vira um anúncio de `OuvinteDaBatalha` e uma linha nele.

> **Redirecione para arquivo, não para um `grep` numa pipe.** Se o `timeout`
> matar o processo, o `grep` pode perder a saída inteira por causa do buffer.
> `> saida.txt` e depois `grep saida.txt` nunca perde.

> **Um arnês que não reproduz o defeito não testou nada.** O primeiro arnês
> escrito aqui para um bug de troca de cena entrava na batalha por um caminho
> diferente do que tinha o defeito, e relatou que estava tudo bem. Antes de
> confiar numa medição, **veja o arnês falhar** com o código quebrado — a mesma
> disciplina dos testes.

### Comparar duas versões

Para ter certeza de que uma mudança não mexeu no que não devia, fotografe a
mesma cena antes e depois e compare pixel a pixel:

```python
from PIL import Image, ImageChops
a, b = Image.open("antes.bmp").convert("RGB"), Image.open("depois.bmp").convert("RGB")
print(sum(1 for p in ImageChops.difference(a, b).get_flattened_data() if p != (0,0,0)))
```

Zero é a resposta que se quer quando a mudança deveria ser invisível. Foi assim
que a correção da troca de vermelho por azul em `Font.cpp` foi liberada: ela
passa por todo texto do jogo, e o menu principal saiu com **0 pixels de
diferença**.
