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
| D8 | Pontos extras | Cada jogador apanha os seus; gastar um paga na nota da dupla (2.2). O **efeito** muda em dupla — ver D11 |
| D9 | O ranking | **Dois**: o Solo, de alunos, e o Co-op, **de duplas** (2.3) |
| D10 | Passar jogando em co-op | **Destranca** a próxima matéria, igual ao solo |
| D11 | O Ponto Extra em dupla | **Não limpa a tela: deixa o _outro_ imune** pelos 3 s que o doador fica sem atirar (2.5) |

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
  da velocidade e a dupla zeraria antes do fim da primeira fase.

  **E só a perda leva um alívio a mais** (`Nota::kAlivioDaPerdaEmGrupo`, 0,8):
  em dupla um acerto custa **2,4** brutos, e não os 3,0 da divisão pura. O
  motivo não é o total — a medição da seção 8 mostrou que a dupla já pagava 21%
  menos no fim —, é a **frequência**: ela apanha 57% mais vezes, e a barra caindo
  quase no dobro das vezes é o que o jogador sente. O pior caso, os dois
  atingidos no mesmo quadro, custava exatamente um acerto de solo (3,0+3,0=6,0)
  e agora custa 4,8.

  O alívio **não vale para o ganho**, e isso é o que mantém a regra de pé: o
  ganho é dividido justamente para a barra subir no ritmo de um solo;
- o **ganho de gastar um Ponto Extra** (10 brutos) — são dois estoques, e sem
  dividir seriam 20 na mesma barra. **O efeito dele muda no co-op**: em vez de
  limpar a tela, ele deixa o *outro* aluno imune pelos mesmos três segundos que
  o doador passa sem atirar — ver 2.5.

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
lá, de graça. Quanto ele vale é a pergunta da medição (incremento 7, seção 8).

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

### 2.1b — A alternância precisa que o projétil seja dono do alvo (D3)

Descoberto ao fazer o incremento 4, e **não estava previsto**: três dos doze
`GetPlayer()` não perguntam "onde mirar agora" uma vez, no disparo. Eles são do
`BossProjectile` (`GetPlayerPosition`, `GetPlayerVelocity`,
`GetPlayerDirection`) e são lidos **a cada quadro**, enquanto o projétil voa —
é assim que a perseguição e a previsão funcionam.

Com um `Alvo()` que reveza, um projétil perseguidor trocaria de alvo no meio do
voo, toda vez que outro ataque fosse disparado. Seria exatamente o ruído que D3
existe para evitar, só que pior: não no traçado de um laser, mas na trajetória
de dezenas de projéteis ao mesmo tempo.

A alternância exige que **cada projétil guarde o alvo que tinha ao nascer**.
Isso é mudança no `BossProjectile` e no caminho que o cria, e não cabia junto
com pôr o segundo aluno em campo. Virou o incremento 5, **feito**.

Como ficou: cada projétil guarda a **posição** do alvo (0 ou 1), e não o
ponteiro — os projéteis vêm de um pool que vive no `Game` e sobrevive à troca
de batalha, e um ponteiro guardado apontaria para um aluno já destruído. Quem
põe é `Boss::ExecuteAttack`, na leva inteira de uma vez, porque **uma rajada é
um disparo**: os vinte projéteis de um leque miram o mesmo aluno.

E a vez passa **depois** do disparo, não antes. Antes, o primeiro ataque da
batalha saía mirado no jogador 2 — a vez começa em 0, e passá-la antes de
atirar a levava para 1 sem ninguém ter sido mirado.

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

"Modo Dupla" entra na tabela `kOpcoes` do `MainMenu`. O que muda depois dela é
o **número de identificações**: duas em sequência, a mesma tela `Identificacao`.

**O menu encolheu em vez de crescer.** A opção entrou como quinta, e as cinco
passaram a se sobrepor em tela: "Carregar Perfil" saía impresso por cima de
"Dois Jogadores". O menu derivava o *passo* da quantidade, mas o tamanho da
letra e a altura da moldura eram constantes — apertar o passo não apertava
nada, e nada avisou.

A correção foi dupla. O tamanho passou a sair do passo, e **"Novo Jogo" e
"Carregar Perfil" sumiram**, junto com o `Game::ModoDeEntrada` inteiro: a tela
de matrícula descobre sozinha quem já tem perfil — há arquivo, abre; não há,
cria. Perguntar antes obrigava o jogador a responder uma pergunta que a própria
matrícula responde, e numa dupla obrigava os dois ao mesmo caminho.

O menu ficou: **Modo Solo, Modo Dupla, Ranking, Opções**.

### 2.5 — O Ponto Extra em dupla é do *outro* (D11)

Sozinho, gastar o Ponto Extra limpa a tela. **Em dupla, ele deixa o parceiro
imune** pelos mesmos três segundos que o doador passa sem atirar — e não limpa
nada.

**O preço é o mesmo tempo, dos dois lados**, e sai de um número só
(`Player::OVERLOAD_DURATION`): quem gasta fica desarmado no meio do que já
estava voando, e o parceiro atravessa. Dois números separados fariam mexer na
sobrecarga sem mexer no presente.

**E não as duas coisas.** Limpar a tela *e* proteger daria, por metade do
estoque da dupla, o dobro do efeito de um Ponto Extra de solo — e tiraria o que
torna esta versão interessante: **ela não salva quem aperta.** O item deixa de
ser um botão de pânico e vira uma decisão sobre o outro, que é o que um modo
cooperativo deveria cobrar.

A imunidade entrou dentro de `Player::GetIsInvincible`, junto com os quadros de
graça de quem apanhou: as duas respondem a mesma pergunta — "um tiro encosta
neste aluno agora?" — e as regras 1 e 3 da batalha já perguntavam isso, então
a colisão não ganhou linha nenhuma. **O contador, porém, é separado**: os
quadros de graça duram 1,2 s e piscam, e misturar os dois faria "fui atingido" e
"estou protegido" terem a mesma cara justamente quando o aluno precisa saber
qual dos dois é. A imunidade pulsa em **azul**, no mesmo ritmo do vermelho da
sobrecarga.

## 3. Requisitos funcionais

### RF1 — Menu
Quatro opções: **Modo Solo, Modo Dupla, Ranking, Opções**. O modo é a única
coisa que o menu decide sobre uma partida — quem é novo e quem é veterano a
tela de matrícula descobre sozinha (2.4).

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
2. ~~**A lista de jogadores na batalha.**~~ **Feito.** `mPlayer` virou lista de
   um e `Alvo()` substituiu os doze `GetPlayer()`. Nada mudou em tela.
3. ~~**A segunda sessão no `Game`.**~~ **Feito.** Duas matrículas, dois
   progressos, duas aparências; o menu ganhou a opção e a identificação roda
   duas vezes, cada uma no painel do seu jogador.
4. ~~**O segundo jogador em campo.**~~ **Feito.** Dois `Player`, cada um no seu
   painel e com a sua personagem, colisões para os dois, a divisão de 2.1, uma
   barra de nota, e a mesma nota gravada na ficha dos dois.
5. ~~**A alternância do alvo (D3).**~~ **Feito.** Cada projétil guarda a
   posição do alvo que tinha ao nascer, e o revezamento acontece uma vez por
   disparo, em `Boss::ExecuteAttack` — ver 2.1b.
6. ~~**Os dois rankings.**~~ **Feito**, em três partes: a ficha na versão 5 com
   o recorde de co-op e o parceiro ao lado; `Ranking::DuplasDaMateria` e
   `DuplasGeral`, que juntam as duas fichas de uma dupla numa linha; e o
   BOTÃO 3 que alterna as duas listas na tela.

   Falta uma coisa pequena: **"você está em 23º" só aparece na lista de solo.**
   Na de duplas a pergunta muda de sujeito — a posição é da *dupla* — e mostrar
   ali a posição de solo seria um número que não pertence a nada na tela.
7. ~~**A medição.**~~ **Feita** — seção 8. Com a mira equalizada, a dupla
   terminava cerca de **4 pontos acima** do solo numa nota de ~79, e o que
   decide de verdade é a posição: parados onde nascem, os dois da dupla não
   acertam o chefe e a nota zera nas seis sementes.

   **Remedido depois do alívio da perda (2,4): +7,5.** Ver 8.2b — o número
   dobrou, e isso é maior que o "levemente" que esta medição tinha concluído.

## 8. A medição (incremento 7)

A pergunta de D4 era **quanto** o co-op é mais fácil, e a resposta tinha de ser
medida, não estimada: a seção 2.1 diz que a regra da divisão deixa o ritmo
igual ao do solo, e que o "levemente" vem de uma coisa só — os ataques mirados
se dividindo entre os dois. Se isso valesse muito, entraria um fator.

**Como foi medido.** `Ferramentas/medir-dificuldade.sh` — rodá-lo reproduz os
números abaixo. Ele monta uma cópia da árvore com tudo preso: a mesma batalha
(fase 0), semente fixa, passo de tempo fixo de 0,016 s, tiro automático e um
piloto no lugar do jogador. Seis sementes,
cada uma rodada com um e com dois jogadores. Seis, e não uma: com dois jogadores
há mais colisões, e cada colisão consome números do mesmo sorteador global, então
a sequência de ataques diverge da do solo depois do primeiro acerto. Uma corrida
de cada mediria o sorteio daquela corrida.

### 8.1 — O primeiro resultado estava errado, e o erro é o achado

Com o piloto **parado onde o jogador nasce**, a dupla zerou a nota nas seis
sementes, enquanto o solo passou em duas. Parecia que o co-op não era levemente
mais fácil, e sim brutalmente mais difícil. O traço quadro a quadro desmentiu:

```
            solo                  dupla
q00060   nota=41,12            nota=40,00
q00180   nota=42,80            nota=40,00
q00300   nota=42,80            nota=40,00
q00420   nota=48,96            nota=40,00     <- sete segundos sem ganhar nada
```

A dupla ficava em 40,00 cravados. Não era dificuldade: **eles não estavam
acertando o chefe uma vez sequer.** O tiro do jogador sobe reto
(`Player::Shoot`: velocidade `(0, -800)`), o chefe nasce no meio da tela, e o
jogador solo nasce no meio também — parado, ele acerta de graça. Os dois da
dupla nascem a 1/3 e 2/3 da largura (`Battle::LoadPlayer`), e parados atiram no
vazio. O arnês estava medindo a **posição de nascimento**.

Isso é uma informação sobre o jogo, e não só sobre a medição: **numa dupla, só
um dos dois pode ficar embaixo do chefe.** O dano da dupla não é o dobro do
solo de graça — ele depende de os dois se arranjarem, e esse arranjo é a
habilidade que o modo cobra.

### 8.2 — Com a mira equalizada

Refeita com um piloto que faz cada jogador perseguir o X do chefe, nos dois
modos:

> As tabelas desta seção são da batalha 0, a do André, **antes de ele ser
> refeito** (commits 154 a 156). Com as fases novas os números do solo são
> outros; elas ficam como registro do experimento, não como referência.

| semente | solo | dupla | dupla − solo |
|---|---|---|---|
| 20261014 | 79,12 | 80,28 | +1,15 |
| 7 | 78,50 | 83,46 | +4,97 |
| 1234 | 78,79 | 86,40 | +7,62 |
| 555 | 83,25 | 87,22 | +3,97 |
| 90210 | 74,19 | 80,87 | +6,68 |
| 31337 | 78,71 | 78,29 | **−0,41** |
| | | **média** | **+3,99** |

**A dupla termina cerca de 4 pontos acima do solo, em uma nota de ~79 — uns 5%.**
Uma das seis saiu negativa, o que é a melhor evidência de que o efeito é pequeno:
ele não domina o resultado, o jogo domina.

### 8.2b — Remedido depois do alívio da perda (2,4)

A tabela acima é de quando um acerto em dupla custava 3,0. Com o alívio de
`Nota::kAlivioDaPerdaEmGrupo` ele passou a custar **2,4** (2.1), e a medição foi
refeita com o mesmo arnês e as mesmas sementes:

| semente | solo | dupla com 3,0 | dupla com 2,4 |
|---|---|---|---|
| 20261014 | 79,12 | 80,28 *(+1,15)* | 84,21 *(+5,08)* |
| 7 | 78,50 | 83,46 *(+4,97)* | 86,72 *(+8,22)* |
| 1234 | 78,79 | 86,40 *(+7,62)* | 89,40 *(+10,61)* |
| 555 | 83,25 | 87,22 *(+3,97)* | 89,75 *(+6,50)* |
| 90210 | 74,19 | 80,87 *(+6,68)* | 84,67 *(+10,47)* |
| 31337 | 78,71 | 78,29 *(−0,41)* | 82,65 *(+3,95)* |
| | | **média +3,99** | **média +7,47** |

**O solo saiu idêntico nas seis sementes**, até a última casa — o alívio não
encosta nele (RNF4), e isso serve de controle: se algum número de solo tivesse
mudado, a mudança teria vazado para onde não devia.

**A vantagem da dupla praticamente dobrou**, de +4,0 para +7,5 pontos — uns 9%
da nota. E a semente 31337, a única negativa e a melhor evidência de que o
efeito era pequeno, virou +3,95: **agora a dupla ganha em todas.**

**Isto precisa ser dito com todas as letras: +7,5 é maior do que o "levemente"
que a seção 8.4 tinha concluído.** D4 pede "levemente mais fácil, mas não
muito", e dobrar o número pode ter atravessado a linha que a própria decisão
desenhou. Fica registrado em vez de maquiado.

Duas coisas em defesa de manter assim:

1. **A medição não capta o Ponto Extra novo** (2.5), que é um *nerf*. O arnês
   nunca aperta o BOTÃO 2, então nenhuma destas doze batalhas gastou Ponto
   Extra. Na prática a dupla perdeu a limpeza de tela, que valia muito num
   aperto — o +7,5 é o número **sem** essa perda descontada.
2. **Os pilotos empilham os dois embaixo do chefe** (8.4), o arranjo mais
   favorável possível. Dois humanos ficam abaixo disso.

Mexer nisso é uma linha: `kAlivioDaPerdaEmGrupo`. 0,9 (perda 2,7) devolveria o
número para perto de +5,5, pelo que a medição sugere.

### 8.3 — De onde vêm esses quatro pontos

O contador de acertos separa as duas metades:

| | solo | dupla | razão |
|---|---|---|---|
| acertos **dados** no chefe | 349 | 670 | 1,92× |
| acertos **sofridos** | 14,2 | 22,3 | 1,57× |
| sofridos **em moeda de solo** (cada um custa metade) | 14,2 | 11,2 | 0,79× |

**No ataque a dupla fica abaixo do empate.** A regra divide o ganho por dois,
então ela precisa de 2,00× para empatar com o solo, e consegue 1,92× — os dois
disputam o mesmo ponto embaixo do chefe e um deles sempre está chegando. Isso
tira da dupla, não dá.

**Na defesa ela ganha, e é daí que vem o "levemente".** Ela apanha muito mais
(1,57×) porque são dois alvos em campo — mas, com o revezamento da mira e a
perda dividida, o estrago em moeda de solo é **21% menor**. É exatamente o que a
seção 2.1 previa que sobraria, agora com número.

### 8.4 — A conclusão, e o que ela não cobre

**D4 estava cumprida sem fator nenhum.** Quatro pontos em oitenta é
"levemente", e a regra da divisão (2.1) entregava isso sozinha. **Essa conclusão
valeu até o alívio da perda entrar** — com 2,4 por acerto o número foi para
+7,5, e a seção 8.2b trata disso. O resto desta seção continua valendo: é sobre
o que a *posição* decide, e o alívio não mudou nada disso.

Mas a medição tem um limite que convém dizer: **os dois pilotos empilham os
jogadores no mesmo X.** É o arranjo mais favorável possível para a dupla, e dois
humanos não conseguem ocupar o mesmo ponto confortavelmente. O arranjo oposto —
parados onde nascem — zera a dupla nas seis sementes. **O jogo real fica entre
os dois**, e onde exatamente depende de quão bem a dupla se coordena.

Para um modo cooperativo, essa é a forma certa da curva: mais fácil para quem se
coordena, mais difícil para quem não se coordena. O que a medição descarta é a
hipótese de que o co-op seria fácil demais — ela não se sustenta em arranjo
nenhum.
