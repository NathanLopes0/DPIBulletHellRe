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

### Acrescentar um arquivo

`CMakeLists.txt` lista os fontes **à mão**, e um arquivo da camada pura aparece
em **dois** lugares: no alvo do jogo e no alvo dos testes.

```bash
grep -c "Source/Nota.cpp" CMakeLists.txt     # tem de dar 2
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
sed -i 's/kExpoente = 1.5f/kExpoente = 1.0f/' Source/Nota.h
cmake --build build -j8 && ./build/dpi_tests -tc="*Nota*"
#   ERROR: CHECK( em99 < em90 / 2.0f ) is NOT correct!
#     values: CHECK( 0.22 <  0.2 )
sed -i 's/kExpoente = 1.0f/kExpoente = 1.5f/' Source/Nota.h
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
de chute. A técnica é sempre a mesma: **copiar o repositório, remendar a cópia
com ganchos de variável de ambiente, e rodar sem tela**.

A cópia importa — os remendos não entram no repositório, e um `git checkout`
distraído não desfaz medição nenhuma.

### Rodar sem tela

```bash
export SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy
./build/DPIBulletHellRe
```

O jogo roda inteiro, desenha e não abre janela. É o que permite medir num
servidor ou em paralelo.

### Entrar direto numa fase

O jogo começa pelo menu. Para cair direto numa batalha, um gancho em
`Game::Initialize`, onde está o `ChangeScene(Scene::SceneType::MainMenu)`:

```cpp
if (std::getenv("DPI_FASE")) {
    mSelectedStage = std::atoi(std::getenv("DPI_FASE"));
    ChangeScene(Scene::SceneType::Battle);
} else
ChangeScene(Scene::SceneType::MainMenu);
```

E para começar numa fase específica do professor, na fábrica dele:

```cpp
boss->SetInitialState(std::getenv("DPI_ESTADO") ? std::getenv("DPI_ESTADO") : "StateOne");
```

### Passo de tempo fixo

O laço principal espera 16 ms por quadro. Para uma batalha de 51 s rodar em
poucos segundos — e para a medição não depender da carga da máquina:

```cpp
if (!std::getenv("DPI_PASSO"))
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}
...
if (const char* p = std::getenv("DPI_PASSO")) deltaTime = static_cast<float>(std::atof(p));
```

Com `DPI_PASSO=0.016` o resultado é igual ao de 62,5 quadros por segundo, só que
na velocidade que a máquina der.

### Fotografar

```cpp
if (const char* arq = std::getenv("DPI_PRINT")) {
    if (SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, mWindowWidth, mWindowHeight,
                                                        32, SDL_PIXELFORMAT_ARGB8888)) {
        SDL_RenderReadPixels(mRenderer, nullptr, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch);
        SDL_SaveBMP(s, arq);
        SDL_FreeSurface(s);
    }
    mIsGameRunning = false;
}
```

Antes do `SDL_RenderPresent`. **Fotografe por instante de jogo, não por número de
quadro**: o carregamento dos pools consome quadros antes de a batalha começar,
então quadro não é tempo.

### Contar

Um `std::printf` com `std::fflush(stdout)` num ponto de passagem responde quase
qualquer pergunta de balanceamento. Foi assim que o expoente da curva da nota foi
escolhido: contando quantos tiros chegam no professor numa batalha inteira, com
o tiro preso, contra os quatro chefes.

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
