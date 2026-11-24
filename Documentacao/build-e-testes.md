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
| `--cena selecao\|boletim\|horas\|entrada\|personagem\|comojogar\|troca\|comeco` | com `--candidatos`, abre a seleção de fases (com o cursor na matéria de `--batalha`), o boletim, as horas complementares, a página das menções que a matrícula nova vê antes da seleção (`entrada`, que leva à seleção) a criação de personagem do aluno 1 (`personagem`) o "Como Jogar" do menu (`comojogar`) a troca de aluno(s) do Botão 4 da seleção (`troca`) ou a escolha entre personalizar e jogar agora (`comeco`) no lugar da tela do orientador — a primeira matéria da lista com 100, e cada uma 8 a menos |
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

`Ferramentas/linha-de-base.sh` roda **84 batalhas** — as dezessete matérias (as
optativas INF 425, INF 429 e INF 491 e a secreta INF 216 inclusive), solo e
dupla, Normal e Difícil, e o TCC com cada um dos dezesseis orientadores — e compara
a nota, quadro a quadro, com a gravada em `Ferramentas/linha-de-base/`:

```bash
bash Ferramentas/linha-de-base.sh               # identicas=84 diferentes=0
bash Ferramentas/linha-de-base.sh --atualizar   # grava a de agora
```

**Uma refatoração tem de dar 84 idênticas, byte a byte.** Uma mudança de
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
optativa. Na INF 429, sozinho, com as sementes 101 a 106 no `esquiva` e 101 a
103 no `segue`:

| Piloto | Normal | Difícil |
|---|---|---|
| `segue` | 30 a 35 (reprova) | 0 (zera na fase 2) |
| `esquiva` | 67 a 82 (aprova nas seis) | 31 a 66 (aprova em três, raspando; uma vai à repescagem e reprova nela) |

Quem joga de verdade fica entre os dois: desvia pior que o `esquiva`, mas pode
entender a fase — e é isso que a INF 429 cobra. Antes, o `esquiva` passava sem
ser acertado nas fases 2 e 3 (82 a 88 no Normal, 70 a 72 no Difícil): a rede só
propagava tiros soltos e lentos. Agora o neurônio que dispara manda **pulsos**
— trens de tiros rápidos, depois do aviso da ligação acesa —, e o dropout os
corta; no Difícil, a convolução tem o **filtro de borda**, que dispara ao lado
de onde se atirou, e o vetor do pooling sai em dois anéis. O `esquiva` não para
de atirar na convolução (acende os dois mapas), só derruba os neurônios que
estão em cima dele e não se esconde do detector: o que ele perde, perde por não
entender a fase. Em dupla, o `esquiva` tira 75 a 83 no Normal e aprova em duas
de três no Difícil (62 e 63); na terceira, os dois sobem até o professor na rede
e a segmentação do MaVIlab os prende contra a parede de cima — quem joga sai da
célula que enche, e o `esquiva` não a vê encher.

A INF 491, da Sabrina, é mais dura que a INF 429 nos dois modos. Sozinho, com
as sementes 101 a 106 no `esquiva` e 101 a 103 no `segue`:

| Piloto | Normal | Difícil |
|---|---|---|
| `segue` | 9 a 21 (reprova; as fases 1 e 2 o derrubam) | 0 |
| `esquiva` | 60 a 81 (aprova nas seis) | 26 a 64 (aprova em uma; quatro vão à repescagem e reprovam nela) |

O `esquiva` não carrega a base que pareia (não pega as cápsulas), não se
encaixa no sítio ativo e não lê o guia do CRISPR: a parede do genoma o acerta a
cada fita. São três coisas que quem entende a fase faz, e o número dele é o de
quem só desvia.

Na INF 425, do Daniel, o `esquiva` vai melhor que o `segue`. Sozinho, com as
sementes 101 a 103:

| Piloto | Normal | Difícil |
|---|---|---|
| `segue` | 72 a 77 (aprova nas três) | 30 a 74 (aprova em duas) |
| `esquiva` | 78 a 88 (aprova nas três) | 84 a 94 (aprova nas três) |

A luta é feita de tiros com tipo, e o `esquiva` desvia de todos (ele só sabe
que o do próprio tipo passa: não absorve nada). O `segue` fica embaixo do
professor, no caminho dos leques e dos tiros que o modelo da fase 3 manda para
onde ele estava indo. Nenhum dos dois faz o que a fase pede: absorver o que o
seu tipo vence, derrubar os outliers e quebrar os núcleos na validação, com o
tipo certo.

A INF 216, do Lucas Ferreira — a secreta —, é a mais dura de todas, e de
longe. Sozinho, com as sementes 101 a 103:

| Piloto | Normal | Difícil |
|---|---|---|
| `segue` | 0 (zera na fase 2) | 0 (zera na fase 1) |
| `esquiva` | 0 (zera na fase 3 nas três) | 0 (zera na fase 3 nas três) |

Na fase 2 o jogo vira de plataforma, e os dois pilotos andam nela. O `segue`
vai pelo chão atrás do professor, sem pular, e as pilhas de caixas e as lanças
o pegam até zerar. O `esquiva` simula, a cada quadro, seis escolhas — ficar ou
andar para um lado ou para o outro, com e sem pulo — com a física do próprio
aluno (`Plataforma::Passo`), e fica com a mais segura (`Simulacao::Saltar`).
É a fase em que ele vai melhor: no Normal, sobe de 10 a 21 pontos e apanha de
0 a 2 vezes.

A fase 1 cobra 3 acertos no Normal: a barra varre o campo
mais rápido do que o `esquiva` enxerga, e o inimigo gigante o pega uma vez. Ele
não pega o power-up nem mira no olho — o que vira o jogo contra o gigante —,
então o número dele é o de quem só foge. Sem o recuo depois do acerto
(`IaDoInimigo::kRecuo`), o gigante prendia o `esquiva` na parede e o acertava
a cada fim de invencibilidade: quatro acertos seguidos, e o Difícil zerava na
fase 2 nas três sementes.
Na repescagem, a frota de bugs põe o professor em god mode enquanto houver bug
(`Ferreira::AceitaDano`): o `esquiva`, que atira para cima atrás do professor,
mata os bugs da coluna dele sem querer e às vezes zera a frota, mas não gasta o
Ponto Extra. Medindo só a repescagem (`--estado StateFinal`, começando com 40),
ele fica em 45 a 52 no Normal e 10 a 23 no Difícil (eram 45 a 63 e 5 a 27, com
o professor sempre aberto ao tiro); gastando um Ponto Extra na chegada da
frota (`--teclas "B@1270"`), a semente 101 passa de 52 para 62 e aprova.

A abertura não machuca: o professor reescreve a nota para 15
(`HudHostil::NumeroEm`, `kNotaDaAbertura`) e só então a luta começa. Antes ela
era um golpe de três anéis que pegava quase todo mundo; sem ele, a fase 1 custa
ao `esquiva` 3 ou 4 acertos, e ele chega às fases 2 e 3 com 14 a 18.

A brincadeira da barra: a onda do Ponto Extra que dissolve um segmento da barra
em campo (`EfeitosDaBatalha::DissolvidoPelaOnda`, que o `Ferreira::Sanear`
consulta antes de esquecer o segmento morto) a quebra. Os cacos
(`HudHostil::Cacos`, sem sorteio) caem por `kQuebraDaBarra` (1,6 s) enquanto a
nota escorre (`NotaNaQuebra`, que só é zero no fim), os acertos não contam, e
então `Battle::DestruirANota` zera a nota sem pagar a fase limpa, encerra a luta
reprovada e põe a frase (`Resultado::Resumo::fraseDoFim`) no lugar da devolutiva.
Para ver: `--batalha 16 --semente 101 --piloto esquiva --atira --teclas "B@560"`
termina com `FIM nota=0.0000`. Os pilotos não usam o Ponto Extra, e as linhas
de base não mudam.

O modo deus do Lucas Ferreira no TCC (`Poder::Tipo::ModoDeus`) não crava mais a
nota em 100: a cada `kEntreRaios` (0,06 s) um raio apaga o tiro do Hugo mais
perto de um aluno (`Poder::MaisPerto`, um aluno por vez na dupla) e paga
`kNotaPorRaio` (0,30) por `Battle::Conquistar`. O rastro é o
`DrawAnimatedComponent::DefinirRastro` (cópias do desenho somadas na cor de
`Hud::ArcoIris`), e o acerto não carrega o poder em curso
(`Poder::CarregaDuranteOPoder`). Na linha de base `tcc_INF216` (semente 11,
`segue`), o primeiro modo deus leva a nota de 44 a 91, e a luta termina em
87,99 (antes, 99,74 com a nota cravada).

Na fase 3, a inversão: cada dúvida que chega ao professor custa 1,3, e os dois
pilotos atiram sem parar. Quem lê o placar ("INVERTIDO: pegue as moedas e NÃO
atire") para de atirar e cata as moedas. O `esquiva` também não pega as moedas
da chuva nem mira o botão ESPERAR do crash: são os pontos que a prova dá a quem
entende o jogo. As fases duram 40 s, o dobro das outras (a das regras, 48 s), e
cada acerto no professor vale 0,55 do de uma matéria comum
(`JogosDigitais::kFatorDoGanho`): sem isso, o tempo dobrado dobraria a nota de
quem só atira.

A etapa do FPS, na fase 3, é a que mais separa quem entende de quem só desvia:
os pilotos nunca seguram o foco (`Player::LerManche` o desliga para eles), então
o jogo fica no teto — 144 FPS no Normal, 240 no Difícil —, e a espiral vermelha
e o fantasma, feitos sem deltaTime, andam 2,4 vezes (ou 4 vezes) o que está
escrito neles. O `esquiva` apanha ali de 5 a 9 vezes, quase sempre do
fantasma, e foi isso que o levou de 0 a 71 para 0 a 22 no Normal e de 32 a 38
para 0 no Difícil; começando com 15 (a abertura), ele zera na fase 3 nos dois. Segurando o foco, o vermelho e o fantasma andam à metade, e
só a parede azul — com deltaTime — pede para soltá-lo (e soltar tem um respiro
de 0,4 s, `RegrasTrocadas::kFolgaAoSoltar`, antes de o FPS voltar a subir, e um
impulso de velocidade, `kImpulso`, para quem segurou o foco ao menos 0,3 s): o vão novo fica longe o
bastante para não ser alcançado com foco e perto o bastante para ser alcançado
sem ele (`RegrasTrocadas::VaoAlcancavel`). Para ver o foco numa foto, rode sem
piloto e segure a tecla com o roteiro — uma a cada 3 quadros, que é o que uma
tecla do roteiro dura (`Simulacao::kQuadrosDeTecla`). A fase tutorial é a
`--batalha 17` (INF 191): com `--piloto segue --atira`, ela acaba sozinha perto
de 58 s de fase (`Tutorial::Acabou`) com `FIM ... aprovado=1`, e o jogo segue para
a página das menções.

```bash
TECLAS=$(python3 -c "print(','.join(f'Left Shift@{q}' for q in range(1500, 1800, 3)))")
./build/DPIBulletHellRe --batalha 16 --estado StateThree --imortal --passo 0.016 \
    --teclas "$TECLAS" --fotos 1560,1900 --fotos-em /tmp/fotos --fim-em 1950
```

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
