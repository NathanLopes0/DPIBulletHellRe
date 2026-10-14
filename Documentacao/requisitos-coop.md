# Requisitos — Modo dois jogadores

Levantamento feito antes de escrever código, para que as decisões fiquem
registradas junto com o que elas causaram. Escrito em 13/10/2026 e **revisto em
14/10/2026**, antes do incremento 2 — ver a seção 0.

## 0. O que mudou na revisão

A primeira versão deste documento deu duas respostas que o autor trocou um dia
depois. Elas ficam registradas aqui, e não apagadas, porque **o motivo da troca
é parte do projeto**:

| | Primeira versão | Agora |
|---|---|---|
| D2 | Cada aluno com a sua nota; a batalha acaba para os dois quando uma zera | **Uma nota só, da dupla** — e por isso a batalha acaba para os dois |
| D4 | A nota de co-op não pode ser **mais fácil** que a de solo | A de co-op é **levemente mais fácil**, com desafios a mais para compensar |

A troca de D4 é uma mudança de objetivo, não de método: o número continua
saindo de medição (2.1). A troca de D2 é maior, e **encolhe o trabalho** em vez
de aumentá-lo — ver 5.2.

Também entraram duas perguntas que não existiam: os **dois rankings** (D9) e o
que acontece com o mapa do curso (D10).

## 1. Contexto

O gabinete do departamento tem **duas posições de jogador**. É o motivo de ele
ser um gabinete e não um computador com um joystick.

Hoje a batalha tem um `Player`, uma nota (`Battle::mGrade`), um conjunto de
teclas e um aluno na sessão (`Game::mMatricula`, `mProgresso`, `mAparencia`).

Três coisas já estão a favor, e vale dizê-las antes das decisões porque elas
encurtam o trabalho:

- **As fichas são por matrícula.** Gravar o resultado de dois alunos já funciona
  sem mexer no armazenamento: são dois arquivos em `Saves/`, como sempre foram.
- **A fase termina por tempo, não por vida do chefe.** Dois jogadores não
  encurtam a batalha, e não existe "dano dobrado" a compensar.
- **O painel já tem as duas posições.** Feito no incremento 1: as setas são do
  jogador 1, o `W A S D` é do jogador 2, e cada um tem três botões próprios.

E uma está contra:

- **Os ataques mirados perguntam "onde está o jogador"**, no singular, em doze
  lugares. Essa é a superfície real da mudança — ver 5.1.

## 2. Decisões

A numeração é a da primeira versão de propósito: há comentários no código que
apontam para `RF1`, `RF2`, `RF3` e `D6`, e renumerar os deixaria mentindo em
silêncio. D2 e D4 mudaram de resposta, não de pergunta.

| # | Questão | Decisão |
|---|---|---|
| D1 | Como os dois entram | Opção **"Dois jogadores"** no menu principal, e dali as duas identificações |
| D2 | A nota | **Uma só, da dupla.** Zerou, a batalha acaba — para os dois, porque não há outra |
| D3 | Em quem um ataque mirado mira | **Alterna**: um disparo para cada, em revezamento |
| D4 | A dificuldade do co-op | **Levemente mais fácil** que o solo, com desafios a mais (2.1) |
| D5 | Teclas do jogador 2 | `W A S D` e os botões `F`, `G`, `H` — **feito**, incremento 1 |
| D6 | Personagem de cada um | Cada um com a sua, do próprio perfil |
| D7 | Visitante no co-op | Permitido, para qualquer um dos dois ou para os dois |
| D8 | Pontos extras | Cada jogador apanha os seus; gastar um paga na nota da dupla (2.2) |
| D9 | O ranking | **Dois**: o Solo, de alunos, e o Co-op, **de duplas** (2.3) |
| D10 | Passar jogando em co-op | **Destranca** a próxima matéria, igual ao solo |

### 2.1 — Uma regra só para a nota compartilhada (D2, D4)

A pergunta de D4 foi feita, na primeira versão, com uma justificativa **errada**:
"dois atiradores derrubam o chefe mais rápido". O chefe não tem vida. Com a nota
compartilhada, porém, dois atiradores **enchem a mesma barra**, e aí a vantagem
é real: a nota cheia custa 256 acertos no chefe, e dois jogadores chegam lá com
128 cada. Sem correção nenhuma, o co-op não seria levemente mais fácil — seria
muito mais fácil.

A correção é uma regra só:

> **O que um jogador sozinho causa é dividido pelo número de jogadores em campo.
> O que a dupla conquista junta é pago inteiro.**

Dividido (ações de um jogador):

- o **ganho por acerto no chefe** (`GRADE_CHANGE_UP`, 0,56 brutos) — **cada
  jogador precisa acertar o mesmo tanto que um solo precisaria**. A dupla soma o
  dobro de acertos no mesmo tempo, e cada um vale metade, então a barra sobe no
  ritmo de antes;
- a **perda por dano sofrido** (`GRADE_CHANGE_DOWN`, 6,0 brutos) — pelo mesmo
  motivo, e é isto que impede o co-op de virar mais difícil. São dois alvos em
  campo; sem dividir a perda, seis brutos por acerto drenariam a barra no dobro
  da velocidade e a dupla zeraria antes do fim da primeira fase;
- o **ganho de gastar um Ponto Extra** (10 brutos) — são dois estoques, e sem
  dividir seriam 20 na mesma barra.

Pago inteiro (resultado da dupla):

- o **bônus de fase limpa** (12 brutos) — é da fase, não de um jogador, e já
  ficou mais difícil: exige que **nenhum dos dois** tenha sido atingido.

**Com um jogador só, dividir por um não muda nada.** É o que mantém o solo
sendo o caso normal (RNF4), sem `if` de modo espalhado pela batalha.

#### O que sobra de diferença, e é de onde vem o "levemente"

Com a regra acima, uma dupla de dois jogadores igualmente bons sobe a barra no
ritmo de um solo e a perde no ritmo de um solo. O que **sobra** a favor do
co-op é uma coisa só: **os ataques mirados se dividem entre os dois**, então a
dupla, somada, apanha menos deles do que um solo apanharia. Só os mirados —
leque, balão, laser e as varreduras do Thiago cobrem o campo inteiro de
qualquer jeito.

Esse é o "levemente mais fácil", e ele **não precisa de fator nenhum**: já está
lá, de graça. Quanto ele vale é a pergunta da medição (incremento 6). Se a
medição disser que é muito, o fator aparece; se disser que está bom, não se
mexe em nada.

#### E os três desafios a mais, que também não precisam de regra nova

1. O **bônus de fase limpa** exige os dois limpos.
2. O teto de 99,99 cai com **3 acertos sofridos** (`kAcertosQuePerdemACheia`), e
   a contagem é da dupla: dois descuidados perdem a nota cheia no dobro da
   velocidade. A contagem **não** é dividida, de propósito — ela conta acertos,
   não quantidade de nota.
3. **Um parceiro fraco custa caro.** Ele contribui com metade do que subiria e
   drena o mesmo tanto ao apanhar — e é a **sua** barra que ele drena. Jogar com
   alguém pior que você é um risco que o solo não tem, e é o preço de a nota ser
   uma só.

### 2.2 — O que fica singular na batalha, e por quê (D2, D8)

Com uma nota só, três coisas que iam virar plurais **continuam como estão**:
`mGrade`, `mAcertosSofridos` e `mAcertosNaFase`. A HUD continua com **uma**
barra de nota. Ver 5.2 — é a metade do trabalho que esta decisão apagou.

O que continua sendo de cada um: a personagem (D6), as teclas (D5) e o estoque
de **Pontos Extras** (D8). O item apanhado é de quem encostou nele; gastar um
limpa o campo para os dois e paga na barra da dupla, dividido por dois (2.1).

### 2.3 — O ranking de co-op é de DUPLAS (D9)

Dois rankings de alunos, com os mesmos nomes e números um pouco diferentes,
seriam a mesma competição duas vezes. **Uma nota da dupla é da dupla**, e é
assim que ela entra: `89384 + 90112 — 87,30`.

Isso resolve de um jeito honesto o problema de carregar alguém: se um jogador
forte leva um fraco, o resultado é da *dupla*, e aparece como tal. Não há como
emprestar nota para ninguém.

**O par é identificado pelas duas matrículas em ordem**, para que a mesma dupla
seja sempre a mesma chave, jogue quem jogar na posição 1.

### 2.4 — O menu ganha um caminho, não uma tela (D1)

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
Dois jogadores em campo, cada um com a sua personagem, as suas teclas e o seu
estoque de Pontos Extras. **Uma nota, uma barra**, pelas regras de 2.1.

### RF5 — Fim da batalha
Acaba quando o tempo da última fase termina **ou** quando a nota zera. A nota da
dupla é gravada **na ficha dos dois**, como resultado de co-op, cada uma
nomeando o parceiro.

### RF6 — Os dois rankings
A tela de ranking mostra **Solo** ou **Co-op**, alternados por um botão. O Solo
é o de hoje, de alunos. O Co-op é de duplas, e sai das mesmas fichas.

**Uma dupla com visitante não entra no ranking**, porque visitante não tem
ficha. O aluno identificado guarda a nota (e com ela o desbloqueio de D10), com
o campo do parceiro vazio; o ranking de duplas ignora registro sem parceiro.

### RF7 — Progresso
Passar em co-op destranca a próxima matéria (D10). O recorde de co-op e o de
solo são **dois números distintos** na mesma matéria: ir bem acompanhado não
apaga o que você fez sozinho, nem o contrário.

## 4. Requisitos não-funcionais

### RNF1 — Duas sessões no `Game`
`mMatricula`, `mProgresso` e `mAparencia` são singulares. Viram uma estrutura
por posição. **O armazenamento não muda**: continuam dois arquivos por
matrícula, gravados pelo mesmo caminho.

### RNF2 — O painel sabe de quem é a tecla
**Feito no incremento 1.** `Painel::Tecla` recebe a posição, as setas deixaram
de ser alias do `W A S D`, e o manche entrou na mesma tabela dos botões. Falta
só o rodapé dizer de quem é o botão quando houver dois na tela.

### RNF3 — Um caminho só para "o alvo"
Os doze lugares que perguntam `GetPlayer()` querem todos a **mesma** coisa:
onde mirar. Eles passam a perguntar `Alvo()`, e a alternância vive num lugar só.
O décimo terceiro uso, na `BattleHUD`, quer outra coisa — "o jogador desta
barra" — e passa a dizer isso.

### RNF4 — Um jogador continua sendo o caso normal
Tudo acima tem de valer com a lista de jogadores de tamanho um, sem `if` de modo
espalhado. `Alvo()` com um jogador devolve sempre o mesmo; dividir por um não
muda número nenhum (2.1).

### RNF5 — Save antigo abre
A ficha ganha os números de co-op numa **versão 5** do formato, com os campos
novos opcionais. Um save da versão 4 abre sem registro de co-op nenhum, que é a
verdade: aquele aluno nunca jogou acompanhado.

## 5. O que muda no que já existe

### 5.1 — Os doze `GetPlayer()`

| Onde | Para quê | Quantos |
|---|---|---|
| `LaserAttack` | de onde até onde traçar o laser | 2 |
| `BaloonAttack` | centrar a fileira de balões no jogador | 1 |
| `BossProjectile` | perseguir, prever e mirar | 3 |
| `Boss` | três consultas de posição para os ataques | 3 |
| `MovementStrategies` | o `HoverAbovePlayer` | 1 |
| `Thiago` | qual linha ou coluna da tabela varrer | 2 |

Todos viram `Alvo()`. Nenhum precisa saber que há dois.

**A primeira versão deste documento dizia treze, e eram doze.** O `grep` que eu
usei contou uma linha de comentário do `LaserAttack` como se fosse chamada. Os
números aqui conferem com:

```bash
grep -rn "GetPlayer()" --include=*.cpp Source | grep -v Scenes/Battle/
```

### 5.2 — `Battle`

`mPlayer` vira lista. Os onze usos de `mPlayer->` passam a varrer a lista:
pausa, retomada, as duas regras de colisão, apanhar Ponto Extra e a
invencibilidade da tela de vitória. `GradeUp` e `GradeDown` dividem pelo
tamanho dela (2.1). `FinishBattle` grava a MESMA nota em duas fichas, marcada
como co-op.

**`mGrade`, `mAcertosSofridos` e `mAcertosNaFase` ficam como estão.** Na
primeira versão deste documento os três viravam plurais; a nota compartilhada
apagou esse trabalho inteiro.

### 5.3 — `BattleHUD`

Uma barra de nota, como hoje. O que muda são as orbes de Ponto Extra, que
passam a ficar em volta do dono.

### 5.4 — `Progresso`, `Ficha` e `Ranking`

`Progresso` guarda, por matéria, o recorde de solo e o de co-op — este último
com **a matrícula do parceiro** e a data. `Ficha` sobe para a versão 5.
`Ranking` ganha um ranking de duplas, que lê as mesmas fichas, junta os dois
lados de cada par pela chave ordenada e descarta repetição.

## 6. Riscos

**O maior: a batalha é a classe mais pesada do projeto.** O plano de incrementos
abaixo existe para que nenhum passo mexa em colisão, pausa, fim de batalha e HUD
ao mesmo tempo.

**O segundo: regressão no modo de um jogador.** Tudo que já funciona passa pelo
mesmo código. A rede é a suíte, mais uma medição de nota solo antes e depois —
se a nota de um solo mudar, alguma coisa quebrou. **Dividir por um é a linha
mais perigosa do projeto neste momento**, e é a que a medição vigia.

**O terceiro: a tela.** Dois jogadores e duas personagens numa tela de 1200×800
que já tem o chefe, os projéteis e a barra de tempo. Pode ficar cheia demais, e
isso só se vê jogando.

## 7. Plano de incrementos

Cada um compila, passa na suíte e pode ser jogado.

1. ~~**O painel do jogador 2.**~~ **Feito.** `Painel` ganhou a posição e o
   manche; as setas deixaram de ser alias.
2. **A lista de jogadores na batalha.** `mPlayer` vira lista de um. `Alvo()`
   substitui os treze `GetPlayer()`. Nada muda em tela.
3. **A segunda sessão no `Game`.** Duas matrículas, dois progressos, duas
   aparências. O menu ganha a opção e a identificação roda duas vezes.
4. **O segundo jogador em campo.** Dois `Player`, colisões para os dois, a
   divisão de 2.1, fim de batalha quando a nota zera.
5. **Os dois rankings.** Ficha versão 5, o recorde de co-op com parceiro, o
   ranking de duplas e o botão que alterna na tela.
6. **A medição.** Com o co-op jogável, medir a nota de uma dupla contra a de um
   solo nas mesmas condições e ver se "levemente" é o que está lá.
