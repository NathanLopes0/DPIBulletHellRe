# Requisitos — Modo dois jogadores

Levantamento feito antes de escrever código, para que as decisões fiquem
registradas junto com o que elas causaram. Escrito em 13/10/2026.

## 1. Contexto

O gabinete do departamento tem **duas posições de jogador**. É o motivo de ele
ser um gabinete e não um computador com um joystick, e o jogo ainda não usa a
segunda.

Hoje a batalha tem um `Player`, uma nota (`Battle::mGrade`), um conjunto de
teclas e um aluno na sessão (`Game::mMatricula`, `mProgresso`, `mAparencia`).
Nada disso é plural.

Três coisas já estão a favor, e vale dizê-las antes das decisões porque elas
encurtam o trabalho:

- **As fichas são por matrícula.** Duas notas independentes já funcionam sem
  mexer no armazenamento: são dois arquivos em `Saves/`, como sempre foram.
- **A fase termina por tempo, não por vida do chefe.** Dois jogadores não
  encurtam a batalha, e não existe "dano dobrado" a compensar.
- **Cada aluno sobe a própria nota com os próprios acertos.** `GradeUp` responde
  ao tiro daquele jogador. Dois atiradores não fazem a nota um do outro subir.

E uma está contra:

- **Os ataques mirados perguntam "onde está o jogador"**, no singular, em treze
  lugares. Essa é a superfície real da mudança — ver 5.1.

## 2. Decisões

As quatro primeiras foram escolhidas pelo autor; as demais seguem delas.

| # | Questão | Decisão |
|---|---|---|
| D1 | Como os dois entram | Opção **"Dois jogadores"** no menu principal, e dali as duas identificações |
| D2 | Quando um zera a nota | **A batalha acaba para os dois**, e cada um volta com a nota que tinha |
| D3 | Em quem um ataque mirado mira | **Alterna**: um disparo para cada, em revezamento |
| D4 | A nota de co-op no ranking | Conta, e **não pode ser mais fácil** que a de solo — o fator sai de medição |
| D5 | Teclas do jogador 2 | `W A S D` mais três teclas próprias; as setas deixam de ser alias |
| D6 | Personagem de cada um | Cada um com a sua, do próprio perfil |
| D7 | Visitante no co-op | Permitido, para qualquer um dos dois ou para os dois |
| D8 | Pontos extras | Cada jogador tem os seus; o item apanhado é de quem encostou |

### 2.1 — O fator da nota sai de medição, não de palpite (D4)

A pergunta foi feita com uma justificativa **errada**: "dois atiradores derrubam
o chefe mais rápido". O chefe não tem vida. O que o co-op muda de verdade são
duas coisas que puxam em sentidos opostos:

**A favor do co-op:** os ataques mirados se dividem entre os dois, então cada um
apanha menos deles. Só os mirados — leque, balão, laser e as varreduras do
Thiago cobrem o campo inteiro de qualquer jeito.

**Contra o co-op:** por D2, um parceiro fraco encerra a batalha do forte. Jogar
com alguém pior que você é um risco que o solo não tem.

Não dá para saber de que lado a balança pende lendo o código. O número sai do
mesmo lugar de onde saiu o expoente da curva da nota: **medição**, com o bot de
desvio, comparando a nota final de uma mesma dupla contra a de um solo nas mesmas
condições. Se a medição disser que o co-op já é mais difícil, o fator é 1 e não
se mexe em nada.

> **A decisão registrada é o objetivo, não o número.** D4 é "a nota de co-op não
> pode ser mais fácil que a de solo". Qual lever puxar — o ganho por acerto, a
> perda por dano, ou nenhum — é consequência da medição.

### 2.2 — Alternar é por disparo, e os dois estão sempre em campo (D3, D2)

D2 simplifica D3 mais do que parece: **nunca existe um jogador ausente no meio
da batalha**. Se um zera, a batalha acaba para os dois; não há o caso de mirar
num alvo que saiu de campo, nem de um jogador sozinho terminando a fase.

A alternância é **por disparo**, e não por quadro: um laser que trocasse de alvo
no meio do traçado seria ruído, não dificuldade.

### 2.3 — O menu ganha um caminho, não uma tela (D1)

"Dois jogadores" entra na tabela `kOpcoes` do `MainMenu`, que já deriva
navegação, desenho e espaçamento do tamanho da lista — acrescentar uma opção é
acrescentar uma linha. O que muda depois dela é o **número de identificações**:
duas em sequência, a segunda com o mesmo `Identificacao`, em outro modo.

## 3. Requisitos funcionais

### RF1 — Menu
Uma quinta opção, "Dois jogadores". As quatro existentes continuam entrando em
modo de um jogador.

### RF2 — Identificação em dupla
A tela de matrícula roda duas vezes, anunciando de quem é a vez. Cada uma aceita
visitante (D7). A mesma matrícula nas duas posições é **recusada**: duas sessões
do mesmo aluno gravariam uma por cima da outra.

### RF3 — Seleção de fases
Uma só, feita pelo jogador 1. O desbloqueio é o **do jogador 1**: a matéria que
ele abriu é a que a dupla joga.

### RF4 — Batalha
Dois jogadores em campo, cada um com a sua personagem, a sua nota, os seus
pontos extras e o seu conjunto de teclas. A HUD mostra as duas notas.

### RF5 — Fim da batalha
Acaba quando o tempo da última fase termina **ou** quando qualquer um dos dois
zera (D2). As duas notas são gravadas, cada uma na ficha do seu dono.

### RF6 — Ranking
As duas notas entram no ranking como qualquer outra, com o fator de D4 aplicado.

## 4. Requisitos não-funcionais

### RNF1 — Duas sessões no `Game`
`mMatricula`, `mProgresso` e `mAparencia` são singulares. Viram uma estrutura
por posição. **O armazenamento não muda**: continuam dois arquivos por
matrícula, gravados pelo mesmo caminho.

### RNF2 — O painel sabe de quem é a tecla
`Painel::Tecla(Botao)` vira `Painel::Tecla(Jogador, Botao)`, e os rodapés que
anunciam teclas passam a dizer de quem elas são quando houver dois. As setas
deixam de ser alias do `WASD` (D5) — hoje os dois movem o mesmo jogador.

### RNF3 — Um caminho só para "o alvo"
Os treze lugares que perguntam `GetPlayer()` querem todos a **mesma** coisa:
onde mirar. Eles passam a perguntar `Alvo()`, e a alternância vive num lugar só.
O décimo quarto uso, na HUD, quer outra coisa — "o jogador desta barra" — e
passa a dizer isso.

### RNF4 — Um jogador continua sendo o caso normal
Tudo acima tem de valer com a lista de jogadores de tamanho um, sem `if` de modo
espalhado. `Alvo()` com um jogador devolve sempre o mesmo; o fator de D4 com um
jogador é 1.

## 5. O que muda no que já existe

### 5.1 — Os treze `GetPlayer()`

| Onde | Para quê |
|---|---|
| `LaserAttack` | de onde até onde traçar o laser |
| `BaloonAttack` | centrar a fileira de balões no jogador |
| `BossProjectile` | perseguir, prever e mirar |
| `Boss` | três consultas de posição para os ataques |
| `MovementStrategies` | o `HoverAbovePlayer` |
| `Thiago` | qual linha ou coluna da tabela varrer |

Todos viram `Alvo()`. Nenhum precisa saber que há dois.

### 5.2 — `Battle`

`mGrade`, `mAcertosSofridos`, `mAcertosNaFase` e `mPlayer` passam a ser por
jogador. As colisões (`mPlayer->...` em seis lugares) passam a varrer a lista.
`FinishBattle` grava duas notas.

### 5.3 — `BattleHUD`

Uma barra de nota por jogador, e as orbes de ponto extra em volta do dono.

## 6. Riscos

**O maior: a batalha é a classe mais pesada do projeto.** Pluralizar o jogador
dentro dela toca colisão, pausa, fim de batalha e HUD ao mesmo tempo. O plano de
incrementos abaixo existe para que nenhum passo mexa em dois desses de uma vez.

**O segundo: regressão no modo de um jogador.** Tudo que já funciona passa pelo
mesmo código. A rede é a suíte, mais uma medição de nota solo antes e depois —
se a nota de um solo mudar, alguma coisa quebrou.

**O terceiro: a tela.** Dois jogadores, duas barras e duas personagens numa tela
de 1200×800 que já tem o chefe, os projéteis e a barra de tempo. Pode ficar
cheia demais, e isso só se vê jogando.

## 7. Plano de incrementos

Cada um compila, passa na suíte e pode ser jogado.

1. **O painel do jogador 2.** `Painel` ganha a posição; as setas param de ser
   alias. Um jogador só, ainda — o que muda é de onde vêm as teclas dele.
2. **A lista de jogadores na batalha.** `mPlayer` vira lista de um. `Alvo()`
   substitui os treze `GetPlayer()`. Nada muda em tela.
3. **A segunda sessão no `Game`.** Duas matrículas, dois progressos, duas
   aparências. O menu ganha a opção e a identificação roda duas vezes.
4. **O segundo jogador em campo.** Dois `Player`, duas notas, colisões para os
   dois, HUD com duas barras, fim por qualquer um zerar.
5. **A medição e o fator de D4.** Com o co-op jogável, medir e decidir.
