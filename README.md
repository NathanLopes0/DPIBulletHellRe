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

O jogo é feito para o painel do gabinete — **duas posições**, cada uma com
manche e três botões — e **toda tela é inteiramente operável com eles**. As
teclas do computador são o mesmo painel, mais alguns atalhos que não aparecem
escritos na tela.

| Painel | Jogador 1 | Jogador 2 | O que faz |
|---|---|---|---|
| **Manche** | **setas** | **W A S D** | mover o aluno, escolher na tela |
| **Botão 1** | **Espaço** | **F** | lançar Dúvidas; confirmar; digitar |
| **Botão 2** | **B** | **G** | voltar e apagar; na batalha, gastar um Ponto Extra: limpa a tela e soma nota, mas sobrecarrega e impede de atirar por alguns segundos |
| **Botão 3** | **N** | **H** | o ranking da matéria, na seleção |
| — | **Esc** | — | na batalha, **só no computador**: pausa — continuar, recomeçar a fase ou sair dela sem registrar a nota |
| — | **Enter**, **Esc**, **0–9**, **Backspace** | — | atalhos de teclado para quem desenvolve |

> As setas **deixaram de ser atalho do W A S D**: elas são o manche do jogador
> 1, e o W A S D é o do jogador 2. Por enquanto só o jogador 1 joga — a segunda
> posição já tem teclas, mas ainda não tem jogador. Ver
> `Documentacao/requisitos-coop.md`.

O que cada botão faz **na tela em que você está** fica escrito no rodapé dela.

Na tela de **Opções**, do menu principal, fica a **dificuldade**: **Normal**, as
fases como foram medidas, ou **Difícil**, um degrau acima para quem já tira 100
— mais coisa para desviar. A nota vale o mesmo nos dois, e a batalha no Difícil
mostra o selo **DIFÍCIL** na ponta da faixa da nota. A opção é da máquina, e não
do aluno: vale para todos que jogarem ali até alguém trocar.

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
| 0 | INF 110 — Programação 1 *(André)* | sempre |
| 1 | INF 213 — Estruturas de Dados *(Salles)* | INF 110 aprovada |
| 2 | INF 250 — Organização de Computadores *(Ricardo)*, INF 220 — Banco de Dados *(Thiago)*, INF 340 — Linguagens de Programação *(Luizinho)*, INF 331 — Linguagens Formais e Autômatos *(Vladimir)* | INF 213 aprovada |
| 3 | INF 420 — Inteligência Artificial *(Júlio)*, INF 221 — Engenharia de Software *(Maria Lúcia)*, INF 321 — Programação Web *(Lucas)*, INF 310 — Programação Concorrente e Distribuída *(Vitor)* | 2 aprovações na coluna 2 |
| 4 | TCC — Trabalho de Conclusão de Curso *(Hugo)* | 2 aprovações na coluna 3 |

Toda matéria tem professor; **dez têm batalha** (André, Salles, Ricardo,
Thiago, Júlio, Vladimir, Vitor, Maria Lúcia, Lucas e Luizinho) e a outra, o TCC,
aparece na grade esperando uma. Ter professor e ter batalha são coisas separadas de
propósito, e cada professor aparece uma vez só na grade.

Quem decide tudo isso é `Assets/materias.json` — a ordem, as colunas, as regras
de desbloqueio, o professor e qual chefe atende cada matéria. Não há lista de
matérias no C++.

## O que fica gravado

Ao lado de `Assets/` o jogo cria uma pasta `Saves/`:

- **`<matrícula>.json`** — uma ficha por aluno: aparência do personagem, recorde
  e última nota de cada matéria, com data. A nota vai para o disco **uma vez, no
  fim da batalha**, e não durante.
- **`opcoes.cfg`** — as opções da máquina (hoje, a dificuldade), em
  `chave=valor`. A tela de Opções o escreve na hora em que algo muda.
- **`notas.csv`** — a planilha do professor, com uma linha por aluno-matéria.
  É regravada a cada batalha, então está sempre em dia e ninguém precisa lembrar
  de exportar. Ela é **derivada** das fichas: apagá-la não perde nada.

As chaves dos arquivos são sempre códigos, nunca posições. Reordenar as matérias
no JSON não reatribui a nota de ninguém.

## Os professores

Cada chefe tem três fases mais uma fase final, de 20 segundos cada, e uma
identidade de ataque própria. A fase final acontece quando a nota termina entre
40 e 59 — é o exame.

- **André — INF 110.** Programação 1, um conceito por fase, com o placar
  mostrando o código que ele está rodando — e os balões do BOCA. Na primeira,
  o fluxograma do `if`: balões numerados descem até o losango e seguem pelo
  ramo que a condição escolhe, e o confete cai na faixa daquele ramo; depois
  vêm o `else if`, o `switch` sem `break` caindo de case em case e a pegadinha
  `if (n = 0)`. Na segunda, o chão vira o vetor `v[0..11]` e de cada casa por
  onde o `i` do laço passa sobe um cacho de balões — o `i <= 12` estoura em
  *segmentation fault* e o `while (1)` só para no *Time limit exceeded*. Na
  terceira, funções: os argumentos voam até o cartão da função e o retorno
  volta, a passagem por valor ataca onde você estava e `segue(&aluno)` vai
  atrás de você. O exame é a prova no BOCA: um problema de cada fase, o balão
  do problema sobe no YES — e o último capota em *Runtime error*.
- **Salles — INF 213.** Estruturas de dados, uma por fase, com o placar
  escrevendo cada operação: uma pilha pendurada nele (push, pop e stack
  overflow), uma tabela hash que encadeia, busca e faz rehash na sua frente, e
  uma árvore binária de busca em que a chave desce comparando — e o seu tiro
  também é uma operação: tira o topo, derruba o nó e a subárvore junto. O
  exame é um bubble sort: os pares trocam na sua frente, e o vetor ordenado
  cai em rampa. Em qualquer fase pode atravessar uma capivara: embaixo dela os
  tiros soltos não te alcançam, e o seu não alcança o professor.
- **Ricardo — INF 250.** Anéis de Arduinos que se abrem a partir dele.
- **Júlio — INF 420.** Inteligência artificial, e as fases contam isso: na
  primeira ele atira a esmo, na segunda mira onde você está, na terceira mira
  onde você **vai estar**.
- **Thiago — INF 220.** Banco de dados. A tabela `alunos` está escrita no
  campo e cada ataque é uma consulta sobre ela, com o SQL no placar: o `WHERE`
  varre as linhas que satisfazem a condição, a projeção varre a coluna e o
  `ORDER BY` troca as linhas de lugar. Em vez de desviar de projétil, você lê
  a consulta e sai da faixa que ela vai buscar. Depois a tabela muda na sua
  frente (`INSERT`, `UPDATE`, `DELETE`), e na terceira fase várias transações
  disputam as linhas, com lock, `COMMIT`, `ROLLBACK` e deadlock. O exame é o
  produto cartesiano, casa por casa.
- **Maria Lúcia — INF 221.** Engenharia de Software, um processo ou padrão por
  fase. A cascata é uma linha de produção: os pedidos do cliente viram
  post-its no backlog, os post-its viram classes ligadas por setas, as classes
  compilam código, cada uma solta o seu bug, e o bug que sobra vira dívida
  parada no campo — e o post-it que você derruba é escopo cortado. Depois vêm
  cinco sprints de métodos ágeis, com o cursor arrastando as histórias, o
  padrão Observer (olhos que te assinam e avisam quando você vira) e, no exame,
  as janelas modais do sistema velho.
- **Vladimir — INF 331.** Teoria da computação: um autômato que lê a palavra
  e atira cada letra lida, a gramática S → aSb derivando em voo, uma máquina de
  Turing cuja cabeça escreve a fita em cima de você e, no exame, o problema da
  parada com o 3x + 1. O "a" vai reto e o "b" persegue, até o Programa de
  Intercâmbio trocar os dois — e ele vale a luta inteira.
- **Luizinho — INF 340.** Linguagens de Programação, um assunto da ementa por
  fase: cada uma abre com a palavra latina por trás dele, e depois o placar
  mostra o código. Na primeira o campo é o texto do programa, um painel por
  bloco com as declarações no canto, e a variável que a busca acha chove onde
  ela é visível — menos nos blocos que têm outra com o mesmo nome; no fim, a
  mesma chamada em escopo estático e em escopo dinâmico. Na segunda, a mesma
  conta em JavaScript, PHP, Python e Haskell: os operandos voam até em cima de
  você, o convertido troca de cor no meio do voo, e o resultado ataca do jeito
  do tipo dele — o número é um anel, o texto é a palavra, o `TypeError` estoura
  onde a conta roda e o erro de compilação, no professor. Na terceira o campo é
  o heap e o alocador te escolhe: no Java o coletor para o mundo e derruba o
  lixo, no C++ o ponteiro pendente estoura e o laço que só aloca vaza até o
  `bad_alloc`. O exame é uma exceção subindo pela pilha de chamadas: cada
  faixa que não a trata cai em cima de você — e a repescagem é um `catch`.
- **Vitor — INF 310.** Programação Concorrente e Distribuída — e Redes, que
  ele também dá. Na primeira, semáforos: cada thread é uma coluna de
  instruções descendo até a seção crítica, quem não tem o semáforo se empilha
  na porta (a fila do `P(s)`, que o placar mostra) e o `V(s)` solta a pilha
  seguinte em leque, com mutex, semáforo de 2, espera ocupada e uma condição
  de corrida em que o saldo dá errado. As correntes prendem: preso, você fica
  lento, e a coluna em cima de você fura a fila. Na segunda, as nuvens mandam
  janelas TCP ao roteador do meio do campo, que só dá conta de 10 por RTT e
  descarta o resto em cacos; a janela segue o dente de serra do controle de
  congestionamento, desenhado num gráfico, e o Ponto Extra vira um timeout. Na
  terceira ele se divide pela nuvem em pai e filho, que trocam mensagens por
  um canal em arco: a caixa postal que enche, o rendezvous, a RPC que devolve
  um leque e o impasse dos dois mandando ao mesmo tempo. O exame é o jantar
  dos filósofos — os garfos andam de mão em mão, e o deadlock vira um cerco
  até o `kill -9`.

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

São 927 casos. Eles cobrem a parte do jogo que não depende de SDL nem de disco —
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
  Attacks/         regras.json e uma pasta por professor com as fases e
                   os projeteis dele
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
