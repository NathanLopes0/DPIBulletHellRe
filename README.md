# DPI Bullet Hell

Um *bullet hell* em que o jogador é um aluno de Ciência da Computação da UFV e os
chefes são os professores do DPI. No lugar de vidas há uma **Nota**: os projéteis
do aluno são Dúvidas que sobem a nota, e ser atingido a derruba. Passar de uma
prova é terminar o tempo dela com 60 ou mais.

O jogo é escrito em C++17 com SDL2 e vai rodar num arcade de duas posições no
departamento, o que explica as decisões adiante: a identificação por
matrícula, a planilha de notas pra fazer um ranking.

Autor: Nathan da Silva Lopes

Esta é uma reescrita do [projeto original](https://github.com/ufv-inf216/projeto-final-NathanLopes0),
feito na disciplina INF 216.

---

## Como jogar

| Tecla | O que faz |
|---|---|
| **W A S D** ou **setas** | mover o aluno |
| **Espaço** | lançar Dúvidas |
| **B** | gastar um Ponto Extra: limpa a tela e soma nota, mas sobrecarrega e impede de atirar por alguns segundos |
| **Enter** | confirmar (menus e seleção de matéria) |
| **T** | trocar de usuário, na seleção de matérias |
| **Esc** | voltar, nas telas de identificação, criação de personagem e opções |
| **0–9** e **Backspace** | digitar a matrícula |

Uma partida começa pela matrícula. **Novo Jogo** registra uma matrícula que
ainda não existe e leva à criação de personagem; **Carregar Perfil** abre uma que
já existe e traz de volta a aparência escolhida. Dá para jogar sem se
identificar, e nesse caso nada é gravado.

## A nota

Toda prova começa em **40**, inclusive para quem já foi bem naquela matéria — a
nota anterior continua guardada na ficha como informação, mas não é o ponto
de partida.

Subir até **60** é fácil e custa poucos acertos. Acima disso cada
ponto custa mais que o anterior: a nota cheia sai por **256 acertos**, e uma
batalha de 51 segundos entrega entre 268 e 382. Apesar de ser possível, ainda estou achando
difícil, porque é necessário seguir o Boss que em alguns movimentos são mais rápidos. Talvez eu diminua
esse número depois de receber feedback externo.

Há um segundo freio, independente da curva: **quem é atingido três vezes para em
99,99** naquela tentativa, por mais tempo que fique em tela. É isso que faz o 100
significar "subiu a curva inteira **e** quase não foi atingido" em vez de ser só
uma questão de paciência — e é por isso que as notas aparecem com duas casas
decimais em todo lugar.

## O curso

As matérias formam uma grade de cinco colunas, e cada uma abre conforme o que já
foi aprovado:

| Coluna | Matérias | Abre quando |
|---|---|---|
| 0 | INF 110 *(André)* | sempre |
| 1 | INF 213 *(Salles)* | INF 110 aprovada |
| 2 | INF 250 *(Ricardo)*, INF 220 *(Thiago)*, INF 330, INF 332 | INF 213 aprovada |
| 3 | INF 420 *(Júlio)*, BIOINF, INF 394, VISCPP | 2 aprovações na coluna 2 |
| 4 | TCC | 2 aprovações na coluna 3 |

Essas são as matérias que eram o plano original, mas elas serão modificadas para refletir o
atual corpo docente. Colocar todos os professores pode ficar fora do escopo, mas é um desejo meu.

Cinco matérias têm professor; as outras aparecem na grade e esperam um. Quem
decide tudo isso é `Assets/materias.json` — a ordem, as colunas, as regras de
desbloqueio e qual chefe atende cada matéria. Não há lista de matérias no C++.

## O que fica gravado

Ao lado de `Assets/` o jogo cria uma pasta `Saves/`:

- **`<matrícula>.json`** — uma ficha por aluno: aparência do personagem, recorde
  e última nota de cada matéria, com data. A nota vai para o disco **uma vez, no
  fim da batalha**, e não durante.
- **`notas.csv`** — a planilha do professor, com uma linha por aluno-matéria.
  É regravada a cada batalha, então está sempre em dia e ninguém precisa lembrar
  de exportar. Ela é **derivada** das fichas: apagá-la não perde nada.

As chaves dos arquivos são sempre códigos, nunca posições. Reordenar as matérias
no JSON não reatribui a nota de ninguém.

## Os professores

Cada chefe tem três fases mais uma fase final, e uma identidade de ataque
própria. A fase final acontece quando a nota termina entre 40 e 59 — é o exame.

- **André — INF 110.** A primeira matéria do curso: balões que sobem e descem
  pelas bordas da tela.
- **Salles — INF 213.** Estruturas de dados: listas encadeadas que avançam em
  fila, listas duplamente encadeadas que avançam e voltam, e uma capivara no
  exame.
- **Ricardo — INF 250.** Anéis de Arduinos que se abrem a partir dele.
- **Júlio — INF 420.** Inteligência artificial, e as fases contam isso: na
  primeira ele atira a esmo, na segunda mira onde você está, na terceira mira
  onde você **vai estar**.
- **Thiago — INF 220.** Banco de dados. O campo vira uma tabela e cada ataque é
  uma consulta que varre uma linha ou uma coluna inteira. Aqui ao invés de desviar de
  projétil, a estratégia é sair da faixa anunciada (representando uma "Busca" em um BD).

## Como compilar

Precisa de **CMake 3.26+**, um compilador **C++17** e as quatro bibliotecas da
SDL2:

```
SDL2   SDL2_image   SDL2_mixer   SDL2_ttf
```

No Windows, o [tutorial de instalação da SDL2](https://github.com/sagedemage/SDL2_windows_setup)
cobre o essencial. Em Linux, os pacotes `libsdl2-dev`, `libsdl2-image-dev`,
`libsdl2-mixer-dev` e `libsdl2-ttf-dev` bastam.

```bash
cmake -S . -B build
cmake --build build
./build/DPIBulletHellRe
```

O executável acha a pasta `Assets/` subindo os diretórios a partir de onde ele
está, então rodar de dentro de `build/` ou da raiz dá na mesma.

No gabinete do departamento ele abre com `--arcade`: tela cheia, sem cursor, sem
saída acidental e voltando ao menu sozinho quando fica parado. O que isso muda,
e **como sair da máquina**, está em `Documentacao/gabinete.md`.

## Os testes

```bash
./build/dpi_tests
```

São 385 casos. Eles cobrem a parte do jogo que não depende de SDL nem de disco —
a aritmética da nota, as regras de desbloqueio, a navegação da seleção de fases,
a montagem dos personagens, a planilha — e também **leem os arquivos de
`Assets/` de verdade**, quebrando ao primeiro erro de digitação num JSON. Um
chefe citado em `materias.json` sem fábrica registrada, um nome de fase que a
máquina de estados não conhece ou um projétil inexistente derrubam a suíte em vez
de virarem uma matéria que não abre.

Os testes foram na sua grande maioria gerados pelo Claude por enquanto, mas to confiando.

## Organização do código

```
Source/          o jogo
  *.h / *.cpp      camada pura: nota, matrícula, progresso, matérias,
                   personagens, navegação, tabela. Nada de SDL nem de disco.
  *Arquivo.cpp     as pontes: é só aqui que se lê arquivo e se fala com SDL.
  Actors/          jogador, professores, projéteis
  Attacks/         as estratégias de ataque e os comportamentos de projétil
  Scenes/          menu, identificação, criação de personagem, seleção, batalha
  Components/      desenho, colisão, física, máquina de estados
Assets/          sprites, fontes, sons e os JSON que descrevem o jogo
  Attacks/         fases.json, regras.json, projeteis.json
  materias.json    o curso inteiro
  personagens.json o catálogo de peças do personagem
Documentacao/    receitas para quem for mexer no código
Ferramentas/     geradores de arte provisória
tests/           a suíte
```

A separação entre o par `X.cpp` / `XArquivo.cpp` é o que torna a suíte possível:
a metade pura entra nos testes, a metade que toca disco e SDL fica de fora.

## Os dados

Quase tudo que define uma batalha está em JSON, não em C++: as fases de cada
chefe, os ataques de cada fase, o comportamento de cada projétil, o catálogo de
projéteis, a grade de matérias e as peças de personagem. Mudar a dificuldade de
uma fase, trocar a ordem das matérias ou dar um ataque novo a um professor é
editar arquivo, não recompilar.

O que continua em C++ é só o que **não é dado**: a mira que depende de onde o
jogador está no instante do disparo.

Como esses arquivos são lidos campo a campo, e como adicionar um professor novo,
está em `Documentacao/`.

Ainda estou pensando se continuo com essa arquitetura de dados ou se volto tudo para ser editado no C++ mesmo.
