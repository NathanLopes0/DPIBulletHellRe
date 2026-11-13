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
| **Botão 1** | **Espaço** | **F** | lançar Dúvidas (os balões com **?**); confirmar; digitar |
| **Botão 2** | **B** | **E** | voltar e apagar; na batalha, gastar um Ponto Extra: sozinho, uma onda dourada sai de você e desfaz os tiros do professor por onde passa, e você fica 1,5 s sem atirar — em troca, ganha a nota que ganharia acertando todos os tiros desse tempo; em dupla, dá ao parceiro um escudo que destrói os projéteis do chefe, e quem gastou segue atirando — e, se os dois gastarem juntos, a onda sai do meio deles, além dos escudos |
| **Botão 3** | **N** | **H** | na dificuldade, trocar a personagem (no boletim, é o manche para cima); na seleção, o boletim (e dele, o ranking da matéria); na batalha em dupla, dar um sinal ao parceiro; na batalha do TCC, soltar o poder do orientador quando o anel dele enche; no resultado da dupla aprovada, os dois juntos: o toca aqui |
| — | **Esc** | — | na batalha, **só no computador**: pausa — continuar, recomeçar a fase ou sair dela sem registrar a nota. A batalha também pausa sozinha quando a janela perde o foco |
| — | **Enter**, **Esc**, **0–9**, **Backspace** | — | atalhos de teclado para quem desenvolve |

> As setas **não são atalho do W A S D**: elas são o manche do jogador 1, e o
> W A S D é o do jogador 2.

O menu principal tem **Modo Solo**, **Modo Dupla** e **Como Jogar** — o manual,
quatro páginas (o manche vira a página): como a matéria funciona, os botões, a
personagem com o convite para a dupla, e as menções — cada selo, o nome e como se
ganha. É curto de propósito — a nota, o que se
destrói, o Ponto Extra e o que a dupla faz junta ficam para o aluno descobrir. O texto mora em
`Assets/como-jogar.json`: dá para revisar uma frase sem recompilar. Em dupla, cada aluno se
identifica, os dois jogam a mesma batalha e a **nota é uma só, da dupla**: o
que um jogador causa sozinho — o acerto no professor, o acerto sofrido, o Ponto
Extra — é dividido entre os dois, e o que a dupla conquista junta, como o bônus
de fase limpa, vale inteiro. Os ataques mirados vão em **quem mais acerta o
professor** — um **v** vermelho em cima do aluno avisa que a mira está nele —, e,
quando os dois acertam parecido, ele reveza um disparo para cada: quem ataca
chama a mira, e o outro fica livre para **cobri-lo**. Derrubar um projétil que ia
no parceiro é uma **cobertura**, e vale inteira, como a fase limpa: é da dupla.
Como o tiro só sobe, cobre quem está **embaixo** do projétil, entre ele e o
parceiro — e, para isso se ver, o projétil destrutível que vai num aluno
**brilha na cor dele**, por fora do contorno vermelho: o brilho laranja, para o
aluno azul, quer dizer "derrube este".
O **Ponto Extra em dupla é do outro**: em vez de
limpar a tela, um feixe sai de quem o gastou e dá ao parceiro um escudo que
destrói os projéteis do chefe por três segundos. **Gastos juntos** — com até
meio segundo de diferença —, os dois Pontos Extras ainda soltam a onda dourada
do solo, do meio dos dois: é a **sincronia**. Quem cobre o parceiro vê, na
hora, um **coração** na sua cor subir de quem foi coberto — o valeu —, e o
**Botão 3**, na batalha em dupla, dá um **sinal**: um anel na sua cor e um som
curto, para chamar o parceiro sem tirar o olho do campo (no TCC, o Botão 3
continua sendo o poder do orientador). As decisões do modo, e o
porquê de cada uma, estão em `Documentacao/requisitos-coop.md`.

O que cada botão faz **na tela em que você está** fica escrito no rodapé dela.

O tiro do aluno é uma **Dúvida**: um balão de fala com um **?**, azul no
jogador 1 e laranja no jogador 2 — em dupla, dá para ver de quem é cada
pergunta. Quando ela acerta o professor, o **?** vira um **!** na mesma cor, que
sobe e some: a dúvida foi respondida. Quando o professor está **blindado** (o
escudo do Hugo, a história que o cursor da Maria Lúcia arrasta), o balão
**murcha e fica cinza**: a pergunta não foi ouvida, e não vale nota.

Alguns projéteis o seu tiro **destrói** — os workers do Elixir, os agentes do
Júlio, os blocos da pilha do Salles, a bala de prata da Maria Lúcia, os alvos
do objetivo do TCC... Todos têm
o mesmo sinal: um **contorno vermelho** em volta do desenho. Sem contorno, o tiro
passa direto ou some nele.

Depois da identificação, **quem vai jogar escolhe a dificuldade**, numa tela
com dois cartões — o manche troca, o botão 1 confirma e leva à seleção de
fases, e o botão 2 volta para trocar de aluno (da seleção de fases, o botão 2
volta para esta tela):

- **Normal**: as fases como foram medidas;
- **Difícil**: um degrau acima para quem já tira 100 — mais coisa para desviar,
  e **quase sem respiro depois de um acerto**: no Normal o aluno pisca 1,2 s sem
  levar outro tiro; no Difícil, 0,4 s, e o tiro que vinha atrás do primeiro
  também acerta (o mesmo tiro, ou o corpo do professor, ainda espera 1,2 s para
  acertar de novo — ficar preso não é o que se cobra).

A nota vale o mesmo nos dois, e a batalha no Difícil mostra o selo **DIFÍCIL**
na ponta da faixa da nota. A escolha é **da partida**: não fica gravada, e toda
partida nova começa com o cursor no Normal — quem chega ao gabinete não herda o
Difícil de quem jogou antes.

**Ao continuar da pausa**, a luta não volta na hora: uma contagem **3, 2, 1**
aparece no meio do campo, com tudo ainda parado, para dar tempo de achar o
aluno no meio dos tiros. `Esc` durante a contagem reabre o menu.

**Quando a luta acaba**, a tela de resultado mostra a nota, se ela bateu o
recorde da matéria, as fases limpas e os acertos sofridos, e oferece **Tentar
de novo** (a mesma matéria, do zero) ou **Voltar às matérias**. Quem reprovou
vê também a **devolutiva**, como numa prova corrigida: a fase em que a nota
mais caiu, com quanto ela custou, e o desenho do que mais te acertou, com
quantas vezes. Na dupla, cada aluno, na cor dele, ganha um **título** pelo que
mais fez — **quem mais cobriu**, **quem mais perguntou** ou **quem mais
desviou** — e, embaixo, a parte dele: acertos no professor, coberturas e acertos
sofridos. Quem cobriu o parceiro vê que contou. Os títulos nunca se repetem e
nunca são uma posição: quem não está à frente em nada (por pelo menos 10%)
ganha o que fez, como "fez 12 perguntas". Quem acaba de passar no TCC vê **FORMADE!**, em dourado,
com a personagem de capelo dos dois lados do título. Na **dupla aprovada**, os
dois avatares ficam dos lados do título, e os dois apertando o **Botão 3 juntos**
fazem o **toca aqui**: os avatares pulam, o título estoura em dourado e se ouve
uma palma — o rodapé ensina. O menu só
aceita o botão depois de um instante, para o tiro que estava apertado não
escolher sozinho. No gabinete, se ninguém escolher em 20 segundos, ela volta
sozinha para as matérias.

Na seleção de fases, o **Botão 3** abre o **boletim do semestre**: as matérias
período a período, com a melhor nota, a situação (aprovade, reprovade, a cursar
ou fechada), quantas provas você terminou em cada uma, o **CR** — a média das
melhores notas das matérias cursadas —, quantas foram aprovadas, as **menções**
e a sua posição no curso. Em dupla, o manche para os lados troca de aluno, e o
**manche para cima** troca a personagem de quem está em tela — a qualquer hora,
voltando para o boletim. Do boletim, o Botão 3 de novo leva ao ranking da
matéria em foco.

Passar abre a matéria seguinte; as **menções** são o que ainda há para buscar
em cada uma depois disso. São sete selos por matéria: **B** bronze (aprovação),
**P** prata (nota 80), **O** ouro (nota 90), **100** nota cheia, **D** aprovado
no Difícil, **2** aprovado em dupla e **T** — o professor daquela matéria
orientou um TCC seu aprovado (o TCC não tem este). Aparecem no boletim, no
cartão da matéria em foco na seleção de fases (com o total do curso no alto) e,
quando uma luta dá uma nova, numa linha dourada do resultado. Na tela que
escolhe o orientador, o canto de cada cartão diz quem **já orientou** você e
quem ainda é **novo**: defender o TCC com cada professor completa o currículo
de orientações.

Depois dos sete, separados por uma folga, vêm os quatro **desafios**: passar
na matéria jogando de outro jeito. **Q** quietinho da turma, sem fazer nenhuma
pergunta (a nota sobe só com as fases limpas, os raspões e o Ponto Extra — e, no
TCC, com os tiros do orientador); **I** intocável, sem ser acertado nenhuma vez;
**A** prestando atenção, com 70% das dúvidas chegando ao professor (de pelo menos
50); **F** sentar na frente, com metade dos acertos feitos de perto (de pelo menos 50). Na dupla, valem os
números da dupla inteira, e os dois ganham. São onze selos por matéria e **142**
no curso. Os desafios não saem dos recordes, como os outros: a batalha conta
como a prova foi jogada (`ContagemDaProva`), e a ficha guarda o cumprido na
lista de atividades (`desafio-intocavel@INF213`), sem mudar de versão — as
chaves guardam os nomes da primeira versão (`desafio-pacifista`, `desafio-mira`,
`desafio-de-perto`), para nenhum save perder o que já ganhou. A última página do
**Como Jogar** mostra cada selo, o nome e como se ganha.

Os selos são pixel art de 13 × 10, desenhada em 2x e pintada por código
(`Ferramentas/gerar_interface.py`): moeda para as notas, placa para o Difícil,
a dupla e o TCC, e escudo para os desafios. A luz vem de cima e da esquerda,
cada cor tem poucos tons, sem degradê, e um pontilhado faz a passagem de um tom
para outro. Apagado, o selo mostra o contorno e a letra em cinza.

**A prova da semana.** Toda semana, de segunda a domingo, uma matéria do curso
é a prova da semana (em rodízio; o TCC não entra). Quem a joga nessa semana
enfrenta **a mesma prova** — a mesma semente: a mesma ordem, os mesmos números,
os mesmos tempos — e a nota entra num ranking que só vale até domingo, um por
modo e por dificuldade. A seleção de fases marca a matéria com um selo dourado e,
com o cursor nela, abre no canto um cartão com quanto falta, o pódio e o seu
lugar; o resultado diz o lugar ao lado da nota. A
matéria continua valendo como sempre (recorde, menções), e continua fechada
para quem ainda não chegou nela. Visitante joga a mesma prova, mas não entra no
ranking.

**As horas complementares.** Do boletim, o **Botão 1** abre o quadro das
atividades complementares: feitos de uma prova aprovada, cada um valendo horas,
uma vez. Três contam **por matéria** e levam o nome dela no resultado ("Fase
limpa em INF 213"): uma fase sem levar acerto (2 h), aprovado com até 10
acertos sofridos (3 h) e três aprovações na matéria (3 h). As outras valem uma
vez no curso: a virada (aprovado depois de a nota cair abaixo de 25), 15
raspões numa prova, 2 pontos extras usados, 10 coberturas ao parceiro, 3
poderes do orientador num TCC, a prova da semana e o pódio dela. A meta é de
**100 horas**; o boletim mostra quantas você tem, e o resultado, as que a luta
deu. As horas também são **moeda**: as optativas se compram com elas. Uma
optativa com preço (`"preco"` no desbloqueio, em `materias.json`) abre com a
regra dela cumprida e comprada — o Botão 1 duas vezes, na seleção de fases. A
carga de 100 h conta tudo o que foi ganho e nunca desce; o que se gasta sai do
**saldo**, que o quadro das horas mostra ao lado ("para optativas: 14 h").

**O rival da turma.** O cartão da matéria, na seleção de fases, mostra quem
está logo acima de você no ranking dela — na lista do modo e da dificuldade da
vez — e quanto falta para passá-lo ("Rival: 90112 (2º) · faltam 2.41"). Quem
lidera vê quem vem atrás; quem nunca jogou vê o último da turma. Na tela do
resultado, uma linha diz quem a nota passou e o lugar novo ("Você passou 90112:
agora é o 2º da turma"), ou quanto faltou para o mais perto, se ficou a menos
de 5 pontos. O visitante, que não entra no ranking, lê no condicional ("você
passaria").

O **Ranking** — do menu, a média do curso; do boletim, a matéria em foco — tem
**quatro listas**: **Normal**, **Difícil**, **Normal em
dupla** e **Difícil em dupla**, trocadas com o manche para os lados. Cada nível
tem o seu topo: quem joga no Normal não disputa com quem escolheu o Difícil.
A tela abre na lista da partida, e a tela de resultado diz "NOVO RECORDE!"
pelo recorde **dessa** lista. Para o mapa do curso, passar é passar: a matéria
abre com 60 em qualquer uma das quatro. Cada linha traz a personagem do aluno
(as duas, na dupla), de capelo para quem já se formou.

**Na dupla, o mapa é o de quem está mais atrás.** Uma matéria só abre para a
dupla se estiver aberta para **os dois**: quem tem menos matérias liberadas não
joga o que ainda não alcançou. Quando só um dos dois falta, o cartão da matéria
diz quem ("J2 precisa passar em INF 110"). Passar jogando em dupla destranca a
próxima matéria para cada um, como no solo.

Parado no menu, o gabinete entra na tela de atração, que reveza três cartazes:
a chamada, os melhores do curso e o **mural dos formados** — quem passou no
TCC, de capelo, dos mais recentes, com a data em que passou.

Ao sair de uma batalha (ou do ranking), a seleção de fases volta com o cursor
na **última matéria escolhida**, e não no INF 110. Uma partida nova começa do
começo da grade. Trocar de matéria acende, num lampejo ciano, só a ligação de uma
à outra — a luz corre do ramo de quem saiu, pelo pedaço do tronco entre as duas,
até o de quem chegou, e some em meio segundo — e o losango em foco dá um pulo e
o reflexo de luz passa por ele. Os losangos são pixel art de 64 × 32,
desenhada em 2x (`Ferramentas/gerar_interface.py`): aresta em escada 2:1,
rampa de sete tons com a matiz andando (a sombra puxa para o violeta, a luz para
o amarelo), contorno seletivo, luz de borda e luz refletida, as facetas
lapidadas e o pontilhado só na passagem da mesa para elas.

Na tela de **Opções**, do menu principal, ficam:

- o volume da **música** e o dos **efeitos**, de mudo a 10;
- a **tela cheia** (no gabinete o jogo é sempre em tela cheia);
- a **hitbox**: mostra na batalha o círculo onde um tiro acerta o aluno de
  verdade, bem menor que o desenho dele.

O manche para cima e para baixo escolhe a opção, e para os lados (ou o botão 1)
troca. As opções são da máquina, e não do aluno: ficam em `Saves/opcoes.cfg` e
valem para todos que jogarem ali até alguém trocar.


Uma partida começa pela matrícula. Depois de escolher **Modo Solo** ou **Modo
Dupla**, cada aluno digita a sua no teclado da tela — o manche escolhe a tecla, o
botão 1 digita e o botão 2 apaga — e confirma em **ENTRAR**. Não é preciso dizer
se a conta é nova: se já existe um perfil com aquela matrícula, ele abre e traz
de volta a aparência escolhida; se não existe, o aluno passa pela criação de
personagem, e o perfil só é gravado quando ela é confirmada. A tecla
**VISITANTE** entra sem se identificar, e nesse caso nada é gravado.

A personagem é vista de costas, olhando para o professor, e é montada peça por
peça: **15 cabelos** em 16 cores, 9 tons de pele, 8 camisas, 7 calças, 4
sapatos e mochila ou bolsa, cada peça na cor que você quiser — o manche para os
lados percorre as opções, e a linha diz em qual delas você está ("3 de 15").
Quem já tem perfil troca a personagem quando quiser: na tela de dificuldade,
com o **Botão 3** do próprio painel, ou no boletim, com o **manche para cima**.

Quem passa no **TCC** se forma, e a personagem ganha o **chapéu de formatura**
para sempre — já na tela do resultado, e depois em toda prova e na criação de
personagem. Ele não é uma peça: não se escolhe nem se tira, e vai por cima de
qualquer cabelo. Nem fica gravado à parte: quem se formou é quem tem o TCC
aprovado, e o recorde só sobe. O visitante também se forma, mas só até o fim da
sessão.

Antes de cada batalha, uma **tela de carregamento** escura cobre o campo: o
código e o nome da matéria, o professor e uma barra enchendo. A batalha só
aparece pronta — o véu some, e só então o professor ataca e a música começa.

**A HUD da batalha conta o que acontece sem texto:**

- **as bordas do campo** são onde você para: dois trilhos dos lados e o painel
  da faixa da nota embaixo, exatamente onde o desenho do aluno encosta (em
  cima, uma linha tracejada discreta, porque ali é o lugar do professor). A
  parede **acende** quando você chega perto dela;
- **onde está o professor**, na horizontal: um brilho vermelho leve na borda de
  baixo, com um "prof" pequeno no painel, embaixo de cada corpo dele — dá para
  saber em que coluna ele está sem tirar o olho do próprio pé;
- **o campo pisca pelas bordas** a cada acontecimento: vermelho no acerto
  sofrido, ouro no Ponto Extra, lilás na troca de fase;
- **o tempo da fase** é a cápsula pequena colada no professor: anda do verde ao
  âmbar e ao vermelho, tem um dente a cada 5 segundos, pulsa nos últimos 3 (com
  o relógio piscando em vermelho) e se enche de volta na fase seguinte;
- **o quadro** no canto é de giz: o professor escreve nele o que está fazendo, e
  o apagador passa quando o texto troca;
- **cada tiro responde**: uma faísca miúda no acerto no professor, uma explosão
  laranja no projétil derrubado, um anel vermelho em volta de quem é acertado,
  um dourado em quem pega um Ponto Extra e três riscos azuis na sua borda
  quando um projétil passa raspando.

## A nota

Toda prova começa em **40**, inclusive para quem já foi bem naquela matéria — a
nota anterior continua guardada na ficha como informação, mas não é o ponto
de partida.

A **barra da nota**, na base da tela, tem duas marcas: a âmbar no **40**
(terminar entre ela e a verde leva ao exame) e a verde no **60** (a
aprovação). Ela corre atrás da nota em vez de saltar: um acerto sofrido deixa um
**rastro vermelho** do que foi perdido, que encolhe em seguida, e a barra
sacode; a nota que sobe dá um clarão. Abaixo do 40 a moldura pulsa em vermelho,
e no 100 ela brilha em ouro.

Subir até **60** é fácil e custa poucos acertos. Acima disso cada
ponto custa mais que o anterior: a nota cheia sai por **301 acertos**, e uma
batalha de 60 segundos tem tempo para até 500 tiros (a do TCC é mais longa,
mas o Hugo blindado só toma dano nas janelas). O ganho de cada acerto
acompanha a duração das fases: a nota sobe por segundo o mesmo que subia quando
as fases tinham 17 segundos. Apesar de ser possível, ainda estou achando
difícil, porque é necessário seguir o Boss que em alguns movimentos são mais rápidos. Talvez eu diminua
esse número depois de receber feedback externo.

**Arriscar compensa.** A Dúvida lançada perto do professor vale mais: o ganho
do acerto cresce de 1 vez, a 470 px dele ou mais, até **1,5 vez**, a 170 px ou
menos — e, a partir de 1,35, a resposta sobe com dois pontos de exclamação
(**!!**) em vez de um. Do fundo do campo, onde você nasce, vale o de sempre; e
encostar no professor continua derrubando a nota. O projétil que passa
**raspando** em você sem acertar — entre o seu colisor e a borda do desenho —
paga um terço de um acerto, com três riscos azuis na sua borda. Cada projétil
vale um raspão só; o que está parado não conta, nem o de quem ainda pisca
depois de ser atingido. Em dupla, os dois se repartem como os acertos.

Há um segundo freio, independente da curva: **quem é atingido três vezes para em
99,99** naquela tentativa, por mais tempo que fique em tela. É isso que faz o 100
significar "subiu a curva inteira **e** quase não foi atingido" em vez de ser só
uma questão de paciência — e é por isso que as notas aparecem com duas casas
decimais em todo lugar.

**O ajuste automático de dificuldade.** Quem vem reprovando numa matéria que
ainda não passou ganha, sem aviso, uma prova um pouco mais leve: a cada **duas
reprovações seguidas** ali, um degrau, até dois. Cada degrau deixa os tiros do
professor **6% mais lentos** e tira **15% a menos** da nota por acerto sofrido.
Aprovar zera a conta, e quem já passou na matéria joga a prova de sempre. Só no
**Normal**: no Difícil o aluno escolheu o desafio. Na dupla vale o menor degrau
dos dois — só alivia quando os dois vêm reprovando. É o canal de *flow*
(Csikszentmihalyi): desafio demais para a habilidade vira ansiedade, e a prova
volta devagar para dentro do canal. O degrau de cada tentativa vai para o
`tentativas.csv` (coluna `ajuste`), e a condição `sem-ajuste` desliga o ajuste
para o experimento (`Regras/Ajuste.h`).

## O curso

As matérias formam uma grade de cinco colunas, e cada uma abre conforme o que já
foi aprovado:

| Coluna | Matérias | Abre quando |
|---|---|---|
| 0 | INF 110 — Programação 1 *(André)* | sempre |
| 1 | INF 213 — Estruturas de Dados *(Salles)* | INF 110 aprovada |
| 2 | INF 250 — Organização de Computadores *(Ricardo)*, INF 220 — Banco de Dados *(Thiago)*, INF 340 — Linguagens de Programação *(Luizinho)*, INF 331 — Linguagens Formais e Autômatos *(Vladimir)*, INF 332 — Projeto e Análise de Algoritmos *(Arroyo)* | INF 213 aprovada |
| 3 | INF 420 — Inteligência Artificial *(Júlio)*, INF 221 — Engenharia de Software *(Maria Lúcia)*, INF 321 — Programação Web *(Lucas)*, INF 310 — Programação Concorrente e Distribuída *(Vitor)*, INF 451 — Sistemas Operacionais *(Goulart)* | 2 aprovações na coluna 2 |
| 4 | TCC — Trabalho de Conclusão de Curso *(Hugo)* | 2 aprovações na coluna 3 |

Toda matéria tem professor e batalha — as doze do curso e o TCC, do Hugo, que é
a última e a mais difícil. A INF 332 e a INF 451 entraram depois: no
`materias.json` elas ficam no fim da lista (o índice de cada matéria é o da
batalha na simulação e na linha de base, e nenhum podia mudar), e na grade, a
INF 332 embaixo da INF 331 e a INF 451 embaixo da INF 310, à direita dela. Ter professor e
ter batalha são coisas separadas de propósito (uma matéria pode existir na grade
antes de ter luta), e cada professor aparece uma vez só na grade.

Quem decide tudo isso é `Assets/materias.json` — a ordem, as colunas, as regras
de desbloqueio, o professor e qual chefe atende cada matéria. Não há lista de
matérias no C++.

**As optativas.** Uma matéria com `"optativa": true` fica numa ala à direita da
grade: com o cursor passando do TCC, a seleção de fases desliza até ela e o
título vira "OPTATIVAS" (a borda direita avisa "OPTATIVAS »" enquanto ela
existir, e a esquerda, "« OBRIGATÓRIAS", para voltar). Cada página da ala tem
até quatro colunas; com mais, ela continua para a direita ("MAIS »"). As
optativas não contam no "Aprovadas: N de M". Ainda não há nenhuma em
`materias.json`: a ala só aparece quando a primeira entrar.

## O que fica gravado

Ao lado de `Assets/` o jogo cria uma pasta `Saves/`:

- **`<matrícula>.json`** — uma ficha por aluno: aparência do personagem, os
  recordes de cada matéria (sozinho e em dupla, no Normal e no Difícil), a
  última nota, com data, e quantas provas de cada matéria você terminou e
  quantas reprovou (o boletim; desde a versão 7 da ficha — uma ficha anterior
  abre com a contagem em zero), e quando cada matéria foi aprovada pela
  primeira vez (desde a versão 8; é a data do mural dos formados, e quem passou
  antes dela aparece no mural sem data), e quantos TCCs aprovados seus o
  professor de cada matéria orientou (desde a versão 9: o currículo de
  orientações), e as atividades complementares cumpridas (desde a versão 10), e
  as reprovações seguidas em cada matéria, de onde sai o ajuste de dificuldade
  (desde a versão 11).
  A nota vai para o disco **uma vez, no fim da batalha**, e não durante.
- **`prova_da_semana.csv`** — as notas da prova da semana: a semana (a data da
  segunda-feira), a matéria, o modo, a dificuldade, a matrícula, o parceiro, a
  nota e quando. Só acrescenta; o ranking da semana sai dele.
- **`opcoes.cfg`** — as opções da máquina (os volumes, a tela
  cheia e a hitbox), em `chave=valor`. A tela de Opções o escreve na hora em
  que algo muda.
- **`notas.csv`** — a planilha do professor, com uma linha por aluno-matéria e
  os quatro recordes (as colunas do Difícil vêm por último).
  É regravada a cada batalha, então está sempre em dia e ninguém precisa lembrar
  de exportar. Ela é **derivada** das fichas: apagá-la não perde nada.
- **`tentativas.csv`**, **`tentativas_fases.csv`**, **`tentativas_resumo.csv`** e
  **`tentativas_toques.csv`** (os toca aqui da dupla aprovada) —
  os dados para pesquisa: uma linha a cada tentativa de batalha (quem jogou, em
  que dia, qual vez era, como terminou, tiros, acertos, Pontos Extras e, na
  dupla, coberturas, as chances de cobrir, o tempo com a mira presa e a
  distância entre os dois), uma a cada fase jogada, e o resumo de quantas vezes
  cada aluno jogou, passou e reprovou em cada matéria. Para comparar o jogo com
  e sem uma mecânica, o gabinete abre com `--condicao` (por exemplo,
  `--condicao sem-brilho`), e cada linha grava a condição em que foi jogada. As
  colunas, as condições, como analisar e o cuidado com a privacidade estão em
  `Documentacao/dados-das-tentativas.md`.

As chaves dos arquivos são sempre códigos, nunca posições. Reordenar as matérias
no JSON não reatribui a nota de ninguém.

## Os professores

Cada chefe tem três fases mais uma fase final, de 20 segundos cada (as etapas
do TCC duram 30), e uma identidade de ataque própria. A fase final acontece
quando a nota termina entre 40 e 59 — é o exame.

**Cada luta é outra.** Toda batalha sorteia uma semente, e cada professor tira
dela o que varia **dentro do conceito de cada fase**: a ordem das portas
lógicas do Ricardo, a temperatura de cada etapa do Júlio (escrita no placar),
o *planning poker* das sprints da Maria Lúcia, a página que o Lucas
renderiza, a fita da máquina de Turing do Vladimir. O conceito é o mesmo
em toda luta; o que muda é o exemplo — como uma lista de exercícios em que cada
aluno pega outros números. Decorar o padrão não serve, e o placar continua
sendo a regra: ele diz o que vai acontecer.

**No Difícil**, cada professor sobe um degrau dentro da mesma ideia — mais
tokens, leques mais largos, relógios mais curtos —, medido para cobrar de quem
**só persegue o professor sem desviar**: esse reprova em parte das lutas, e
quem desvia ainda passa. O Normal continua sendo as fases como foram pensadas.

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
  escrevendo cada operação: uma pilha pendurada ao lado dele (push, pop e stack
  overflow), uma tabela hash que encadeia, busca e faz rehash na sua frente, e
  uma árvore binária de busca em que a chave desce comparando — e o seu tiro
  também é uma operação: tira o topo, derruba o nó e a subárvore junto. O
  exame é um bubble sort: os pares trocam na sua frente, e o vetor ordenado
  cai em rampa. Em qualquer fase pode atravessar uma capivara: embaixo dela os
  tiros soltos não te alcançam, e o seu não alcança o professor.
- **Ricardo — INF 250.** Uma matéria de hardware por fase, e o placar é a
  regra: palavras de bits atravessando portas lógicas (Sistemas Lógicos), o
  clock e os registradores que andam nele (Organização), o pipeline com a
  instrução no estágio de execução (Arquitetura) e, no exame, quatro Arduinos
  rodando o Blink.
- **Júlio — INF 420.** Inteligência artificial, e a batalha é um modelo de
  linguagem. Na primeira fase ele gera a frase "o mo de lo te pre diz" token a
  token, cada um nascendo na ponta da frase com um leque que mostra as chances
  do próximo: fria, a frase é uma curva lisa que vem na sua direção; quente,
  ela ziguezagueia e alucina, passando do fim. Na segunda
  convoca agentes que trabalham no ciclo ReAct — pensam,
  agem mandando a ferramenta em você e observam. Na terceira ganha guardrails
  que comem o seu tiro, e o modelo te prediz de verdade: mira onde você **vai
  estar**. No exame, o modelo degenerou em repetição e só gera "e".

- **Thiago — INF 220.** Banco de dados. A tabela `alunos` está escrita no
  campo e cada ataque é uma consulta sobre ela, com o SQL no placar: o `WHERE`
  varre as linhas que satisfazem a condição, a projeção varre a coluna e o
  `ORDER BY` troca as linhas de lugar. Em vez de desviar de projétil, você lê
  a consulta e sai da faixa que ela vai buscar. Na segunda fase a tabela muda
  na sua frente, e cada comando tem o seu gesto: o `INSERT` arremessa o
  registro novo em arco até a linha vazia, o `DELETE` risca a linha, que
  despenca e se espatifa no chão, e o `UPDATE` chuta o valor velho em você
  enquanto o novo voa até a casa. Na terceira, as transações: cada uma abre
  com um selo (T1, T2…) e prepara comandos das duas fases anteriores, que
  ficam na tabela como rascunho na cor dela — e no `COMMIT` tudo acontece de
  uma vez. Derrube o selo antes disso e é `ROLLBACK`: nada do que ela
  preparou acontece. O exame é o produto cartesiano, casa por casa.
- **Maria Lúcia — INF 221.** Engenharia de Software, um processo ou padrão por
  fase. Na **cascata**, cinco caixas descem em escada, e da caixa da etapa da
  vez cai, o tempo todo, o que ela produz — post-its, classes, código, bugs,
  dívida —, cada vez mais rápido. De tempos em tempos o cliente pede outra
  coisa, e na cascata não se volta atrás: o pedido vai para a primeira caixa e
  desce a escada inteira, virando o artefato de cada caixa e pagando em cada
  uma o custo de mudar ali — 1, 2, 3, 5 e 8 tiros, a curva de Boehm nos números
  do planning poker —, até estourar na etapa da vez ("Mudança nos Testes: custo
  ×5"). Derrubar o pedido no caminho sai mais barato quanto mais cedo. No
  **Scrum**, o quadro A FAZER · FAZENDO · FEITO: cada sprint planeja mais
  histórias (cada uma com os pontos que o poker deu), e a cada **daily** o
  cursor dela arrasta o quadro uma coluna — na FAZENDO a história trabalha e
  atira, a mais pesada mais; o que estava FEITO cai como incremento no seu
  campo e estoura. O tiro tira a história da sprint, e o burndown no canto mostra
  os pontos que faltam. Nos **padrões de projeto**, o Observer — os olhos se
  inscrevem um a um e orbitam você, cada vez mais perto, e quando você muda de
  estado (para, anda, vira) é `notifyAll()`: todos atiram — e depois o
  Decorator: a bala nasce ao lado dela, sai para fora e ganha camadas no
  caminho (Rápida, Dividida, Teleguiada — que vira para você por um segundo e
  meio, e depois segue reta), com a composição no placar (`new Teleguiada(new
  Dividida(new Rápida(bala)))`); cada tiro seu tira o decorador de fora. No
  exame, a frase dela: **não há bala de prata**. O lobisomem de Brooks — o
  projeto de software — persegue você e uiva, blindado; ela atira as
  promessas (IA, UML, low-code...), e elas perseguem você, fazendo a curva. A
  que bate no lobisomem não o mata: se desfaz em bugs contra ele, e ele cresce.
  A que você derruba antes é o teste com usuário, da IHC que ela pesquisa.
  Nada disso depende do que está em campo: o Ponto Extra limpa a tela e o
  lobisomem volta — a complexidade essencial não some.

- **Vladimir — INF 331.** Teoria da computação: um autômato que lê a palavra
  e atira cada letra lida, um compilador (ele também dá Construção de
  Compiladores) que quebra uma linha de código em tokens, monta a árvore
  sintática no campo e a transforma em instruções mirando em você, uma máquina de
  Turing de verdade — o castor mais ocupado de 4 estados, que escreve, apaga e
  relê a fita por 107 passos até parar com 13 marcas — e, no exame, o problema da
  parada com o 3x + 1. O "a" vai reto e o "b" persegue, até o Programa de
  Intercâmbio trocar os dois — e ele vale a luta inteira.
- **Luizinho — INF 340.** Linguagens de Programação — e Teoria da Computação,
  que ele também dá —, um assunto da ementa por fase: cada uma abre com a
  palavra latina por trás dele, e depois o placar mostra o código que ele está
  rodando. Pouco se repete de uma luta para a outra: o programa da primeira
  fase, os laços da terceira, os horários das contas e as exceções são
  sorteados.

  Na primeira, as amarrações: o campo é um programa — o global, a função `f` com
  um bloco dentro e a função `g`, cada bloco com a sua declaração de `x` no
  canto. O professor roda `atira(x)` num bloco, e a busca do nome anda de canto
  em canto, do bloco de dentro para fora, até achar quem declara o `x` — e esse
  bloco responde com um leque de tantos tiros quanto o valor. No fim, `g()`
  chama `f()`: no escopo estático o `x` é o do global, onde `f` foi escrita; no
  dinâmico, o do LISP antigo, é o de `g`, quem chamou. Na segunda, a mesma conta
  em JavaScript, PHP, Python e Haskell: os operandos voam até em cima de você, o
  convertido troca de cor no meio do voo, e o resultado ataca do jeito do tipo
  dele — o número é um anel, o texto é a palavra, o `TypeError` estoura onde a
  conta roda e o erro de compilação, no professor. Na terceira, o programa
  Termina: o professor mostra um laço e a entrada dele, o Termina responde se
  ele para, e o laço roda — cada volta é um anel de tiros, e o laço que gira
  desenha uma espiral; o que não para só acaba no Ctrl+C. No fim vem o Paradoxo,
  que pergunta ao Termina sobre ele mesmo e faz o contrário da resposta: o
  Termina muda de ideia cada vez mais depressa, até se contradizer e estourar —
  não tem como fazer um programa que sabe se outro programa termina. O exame é
  uma exceção subindo pela pilha de chamadas: cada faixa que não a trata cai em
  cima de você — e a repescagem é um `catch`.
- **Arroyo — INF 332.** Projeto e Análise de Algoritmos, a ementa em quatro
  provas. Na **análise**, a árvore de recursão: o professor solta T(n), que se
  divide em `a` subproblemas de tamanho n/b, que se dividem de novo até as
  folhas, e cada nó, ao se dividir, paga o trabalho f(n) em tiros — o custo de
  cada nível fica na tela: no merge sort todo nível custa o mesmo, no Karatsuba
  as folhas dominam, na busca binária é um tiro por nível. O placar mostra a
  recorrência e, quando a árvore acaba, o Teorema Mestre (`T(n) = 2T(n/2) + n =
  Θ(n log n)`); são cinco recorrências sorteadas por luta, o merge sort sempre
  primeiro. O nó é derrubável, e o que cai leva a subárvore junto: dá para
  **podar a árvore**. Nos **paradigmas**, três problemas de sala: o troco
  **guloso** (as moedas saem da maior para a menor e perseguem você; nos
  sistemas ruins o guloso erra, e erra com moeda a mais — "Troco de 31:
  25+1+1+1+1+1+1 (ótimo: 4 moedas)"), a tabela da **programação dinâmica**
  (a maior subsequência comum de ALGORITMO e LOGARITMO, preenchida em ondas
  diagonais — cada casa calculada chove, a que casa letra com letra chove em
  leque) e o **backtracking** das seis rainhas (colocadas linha a linha e tiradas
  quando não há saída, cada uma atirando pela coluna e pelas diagonais de baixo,
  com o que ela ataca desenhado no tabuleiro; achada a solução, as seis atacam
  nas oito direções). Nos **limites**, a rede de ordenação — seis fios descem o
  campo, e em cada comparador os dois valores fora de ordem trocam de fio na
  diagonal; embaixo saem ordenados, e o placar lembra que ordenar por comparação
  pede ⌈log₂ 6!⌉ = 10 comparações —, e depois o **caixeiro viajante**, com
  5, 6 e 7 cidades: o número de rotas explode (12, 60, 360), o caixeiro tenta
  rota atrás de rota, cada cidade em que ele passa atira, e a melhor rota fica
  desenhada em ouro. A final é a **aproximação**: a têmpera simulada. Um enxame
  de soluções procura o mínimo de uma energia cujo vale mais fundo é você — e
  dois rasos, os ótimos locais. Quente, cada solução aceita até passo para
  pior (o critério de Metropolis, de verdade: `exp(-Δ/T)`), e o enxame se
  espalha; esfriando, desce para os vales, e quem cai num ótimo local fica
  preso lá e estoura. No meio da fase a temperatura volta a subir. A paisagem
  fica na tela: os anéis dos ótimos locais, rasos, e os seus, mais fundos; um
  termômetro na borda; e um risco vermelho a cada passo para pior que o
  Metropolis aceita. O placar diz o estágio ("Quente, T = 820: aceita passo
  pior (72%)", "Fria: só aceita o que melhora") e avisa quando o frio prende
  soluções num vale raso. Toda prova
  se refaz sozinha: o que o Ponto Extra limpa volta na rodada seguinte, porque
  o que anda é o algoritmo, e não os projéteis (`Recorrencia.h`,
  `Paradigmas.h`, `Intratavel.h`, `Tempera.h`).
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
  terceira ele se divide pela nuvem em cliente e servidor, que trocam mensagens por
  um canal em arco: a caixa postal que enche, o rendezvous, a RPC que devolve
  um leque e o impasse dos dois mandando ao mesmo tempo. O exame é o jantar
  dos filósofos — os garfos andam de mão em mão, e o deadlock vira um cerco
  até o `kill -9`.
- **Goulart — INF 451.** Sistemas Operacionais, com o sistema inteiro em
  campo. Nos **processos**, ele é a CPU, e os processos são cartões que andam
  entre a fila de PRONTOS, à esquerda, a CPU, embaixo dele, e a E/S, à direita:
  o escalonador é Round Robin de verdade (`Escalonador.h`) — a cada quantum o
  primeiro da fila sobe e roda, e rodar é atirar, cada tipo do seu jeito (o
  limitado por CPU solta leques; o limitado por E/S, uma rajada curta, e vai
  esperar o disco; o de três threads atira de três lugares ao mesmo tempo). A
  troca de contexto, a interrupção de quem volta da E/S e o `exit` de quem
  termina são anéis; ele dá `fork()` de tempos em tempos, o placar mostra as
  filas, e um gráfico de Gantt cresce no alto com cada fatia de CPU na cor do
  processo. O processo que espera na fila morre com tiros: `kill -9`. Na
  **memória**, a paginação (`Paginacao.h`): o placar faz a conta da MMU
  (`0x3A7F = página 3 + 0xA7F`), a tabela de páginas fica à esquerda, com o
  bit de válida e uma linha até a moldura, e a memória física é um relógio de
  molduras — o acerto acende o bit R e atira; a falta de página faz o
  ponteiro varrer, dando a segunda chance a quem tem R, e a página vítima é
  despejada em você. Primeiro a localidade (o conjunto de trabalho cabe),
  depois a mudança de fase do programa e, no fim, o **thrashing**: um laço
  que não cabe na memória, quase toda referência uma falta. Na **entrada e
  saída**, ele é o braço do disco (`Disco.h`): os pedidos caem numa trilha de
  200 cilindros, e ele os atende em FCFS, SSTF e SCAN, o elevador — o bloco
  lido cai em você, o pedido esquecido pelo SSTF passa fome e atira, e o
  placar compara o quanto o braço andou com o que o FCFS andaria
  (`SSTF: 150 cilindros em 7 pedidos · em FCFS seriam 314`). O exame é a
  **nuvem** (`Virtualizacao.h`): ele é o hipervisor, as VMs são caixas no campo
  com o sistema convidado dentro; o kernel de cada uma atira em anel, a
  instrução privilegiada vira um trap que voa até ele, e ele a emula atirando
  em você; na migração ao vivo uma VM escorrega de um host a outro, e na
  elasticidade a carga sobe e a nuvem sobe mais VMs — a que você derruba, ela
  repõe. Tudo anda pelo relógio do sistema: o Ponto Extra limpa a tela, e o
  escalonador, a memória, o disco e a nuvem continuam.
- **Lucas — INF 321.** Programação Web, na ordem da matéria. No HTML ele
  renderiza uma página gerada a cada luta, com o layout desenhado no campo, e
  cada elemento ataca do seu jeito: o cursor voa até o `<button>` e clica, a
  `<img>` gira o spinner e estoura, o `<marquee>` atravessa a tela, a
  `<table>` cai linha a linha, o `<a href>` o leva à âncora e o `<input>`
  digita letra por letra. No CSS ele abre o DevTools: uma grade de cartões que
  cada regra do placar transforma inteira — `gap`, `rotate`, `scale`,
  `display: none`, `animation`, `position: fixed`, `:hover` —, e enquanto ele
  digita a regra, as letras escorrem embaixo dele (a cascata). O Elixir é uma
  árvore de supervisão: casts correm pelas linhas até a caixa de cada worker,
  a caixa cheia vira um leque, e o worker derrubado é reiniciado pela
  estratégia da rodada (`:one_for_one`, `:one_for_all`, `:rest_for_one`);
  parado, o supervisor manda as dúvidas pelo `|>`. O exame é o event loop do
  JavaScript: cada script passa pela pilha, os timers correm nas Web APIs, e
  as promessas da fila de microtarefas rodam antes do `setTimeout(cb, 0)` —
  cada callback que sai de uma fila ataca.

- **Hugo — TCC.** A última matéria e a batalha mais difícil do jogo — e a
  maior: quatro etapas de 30 s. Antes dela você escolhe um **orientador**
  entre os professores em que tirou as maiores notas, e ele luta do seu lado:
  o tiro dele tem a cara da matéria (o `for` do André, a pilha do Salles, o
  barramento do Ricardo, a transação do Thiago, o ponteiro do Luizinho, a
  gramática do Vladimir, o gradiente do Júlio, o Observer da Maria Lúcia, o
  `fetch` do Lucas, a janela do TCP do Vitor, a busca binária do Arroyo, que
  fecha os dois tiros no Hugo pela metade a cada rodada, e o Round Robin do
  Goulart, uma rajada curta de cada lado por vez), e cada um traz um
  **poder**, que carrega a cada acerto no Hugo e sai no **botão 3**. O poder é um
  conceito da matéria dele, jogável:

  - **André — `for (i) for (j)`.** Laços aninhados: uma matriz de 5 linhas de
    8 tiros, centrada em você. Embaixo do Hugo, a matriz inteira acerta.
  - **Salles — `push()` e `pop()`.** A capivara anda embaixo de você e
    **empilha** os tiros que come; quando o poder acaba ela **desempilha**
    tudo no Hugo, o último primeiro. Cheia, é *stack overflow*: começa a
    devolver na hora.
  - **Ricardo — `HLT`.** O clock parou: o Hugo e os tiros dele congelam (a
    fase e o relógio param junto).
  - **Thiago — `BEGIN`.** Uma transação: a nota de agora fica salva. No fim, se
    ela caiu, é `ROLLBACK` — a nota volta, e os acertos sofridos deixam de
    contar; se subiu, `COMMIT`. Seis segundos para arriscar perto do Hugo.
  - **Luizinho — `try { } catch`.** Dentro do bloco, o acerto que você
    levaria é uma exceção capturada: não tira nota, e o `finally` responde
    com um leque no Hugo.
  - **Vladimir — o autômato rejeita.** Os tiros em volta de você viram seus.
  - **Júlio — agentes autônomos.** Os seus tiros e os do orientador caçam o
    alvo destrutível do objetivo (ou o Hugo).
  - **Maria Lúcia — a bala de prata.** Quebra o escudo do Hugo.
  - **Lucas — `Promise.all()`.** Doze promessas que vão até o Hugo e
    **voltam** douradas, desfazendo os tiros que tocam na ida e na volta.
  - **Vitor — `P(s)`.** O semáforo: uma porta de parede a parede, acima de
    você. Os tiros do Hugo fazem fila nela e passam um de cada vez; no `V(s)`
    quem ainda esperava morre de inanição.
  - **Arroyo — busca binária.** A cada passo some a metade dos tiros do Hugo —
    a mais perto de você —, de novo e de novo: n, n/2, n/4... A banca dele
    pergunta "Qual é a complexidade disso?".
  - **Goulart — Round Robin.** Os tiros do Hugo em quatro grupos, e só o da
    vez anda, um quantum de cada vez — os outros esperam parados: o campo
    inteiro anda a um quarto da rapidez, aos trancos. A banca dele pergunta
    "Quanto custa uma troca de contexto?".

  A tela em que se escolhe o orientador mostra o poder de cada um
  funcionando: um campo em miniatura, animado, com a legenda de cada passo.
  Na batalha, uma linha em cima do aluno diz o que o poder está fazendo
  (`push(tiro)  pilha: 7`, `ROLLBACK: a nota voltou a 62.40`). Escolher o
  orientador é escolher o jeito de jogar.

  **O Hugo é blindado** — o TCC ainda não está pronto — e cada etapa tem
  objetivos que abrem uma janela em que ele toma dano. São duas maneiras por
  etapa, alternando, e a primeira é sorteada a cada luta. Uma é derrubar a
  tiro: na proposta, as assinaturas, enquanto as barras do cronograma de Gantt
  caem no prazo; no desenvolvimento (o tema é a área do orientador, com os
  ataques da matéria dele), os bugs; na defesa, a banca são dois outros
  professores, e você acerta a resposta certa (A, B ou C) da pergunta deles —
  "E se der deadlock?" —; nas correções, as marcas em vermelho. A outra é
  **andar**, no meio dos tiros: na proposta, a **reunião de orientação** — o
  orientador voa até um lugar marcado no chão, e o anel enche enquanto você
  fica com ele; no desenvolvimento, as **referências**, livros espalhados para
  pegar; na defesa, a **arguição**, em que cada pergunta que você **raspa** sem
  ser acertado conta uma; nas correções, a **entrega** da versão final, que
  você pega e leva até a secretaria — e deixa cair onde for acertado.

  **Dá para ler o escudo sem o placar.** Blindado, o Hugo fica escuro, com um
  cadeado no peito e um escudo de **placas**, uma por coisa a fazer, que caem
  à medida que você avança; um fio tracejado sai do escudo até cada coisa que
  ainda o segura, e o tiro que bate nele solta uma faísca. O que se pega fica
  num anel verde. Aberto, as placas estouram, ele volta às cores dele, e uma
  mira dourada o cerca com o arco do tempo que falta, que pisca no fim; cada
  acerto dá um clarão. Aberto, ele também perde as páginas da monografia
  embaixo dele.

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

### Executáveis para distribuir

Para mandar o jogo para quem não vai compilar nada, dois scripts montam um
pacote pronto, com o executável, as bibliotecas e a pasta `Assets/`:

```bash
bash Ferramentas/executavel-windows.sh   # dist/DPIBulletHellRe-windows.zip
bash Ferramentas/executavel-linux.sh     # dist/DPIBulletHellRe-linux.tar.gz
```

Os dois rodam numa máquina Linux (o do Windows pela compilação cruzada com o
MinGW-w64). O GitHub também gera os dois sozinho: crie uma tag que comece com
`v` (`git tag v1.0 && git push origin v1.0`) e os pacotes aparecem numa Release
com o nome da tag; ou rode o workflow **Executaveis** pela aba Actions. Os
detalhes estão em `Documentacao/build-e-testes.md`.

No gabinete do departamento ele abre com `--arcade`: tela cheia, sem cursor, sem
saída acidental e voltando ao menu sozinho quando fica parado. O que isso muda,
e **como sair da máquina**, está em `Documentacao/gabinete.md`.

## Os testes

```bash
./build/dpi_tests
```

São 1070 casos. Eles cobrem a parte do jogo que não depende de SDL nem de disco —
a aritmética da nota, as regras de desbloqueio, a navegação da seleção de fases,
a montagem dos personagens, a planilha, o objetivo e o poder do orientador no
TCC — e também **leem os arquivos de
`Assets/` de verdade**, quebrando ao primeiro erro de digitação num JSON. Um
chefe citado em `materias.json` sem fábrica registrada, um nome de fase que a
máquina de estados não conhece ou um projétil inexistente derrubam a suíte em vez
de virarem uma matéria que não abre.

Os testes foram na sua grande maioria gerados pelo Claude por enquanto, mas to confiando.

## Organização do código

```
Source/          o jogo
  Motor/           o laço do jogo (Game), a matemática, o sorteio, a fonte,
                   o som, o JSON, os caminhos, as opções e o modo gabinete
  Entrada/         o painel do gabinete, o teclado da matrícula, a navegação
  Aluno/           matrícula, sessão, progresso, ficha, ranking e a planilha
  Pesquisa/        o registro das tentativas (os dados do TCC)
  Curso/           as matérias e quem pode orientar o TCC
  Personagem/      o catálogo de peças e a montagem da personagem
  Regras/          as regras puras da batalha: nota, Ponto Extra, acertos,
                   dificuldade, variação, intercâmbio, golpe e poder
  Interface/       as contas da HUD, da barra da nota, do carregamento, da
                   pausa e do resultado, e o manual da tela Como Jogar
  Actors/          jogador, professores, orientador, projéteis
  Attacks/         as estratégias de ataque e os comportamentos de projétil
  Scenes/          menu, identificação, criação de personagem, seleção, batalha
  Components/      desenho, colisão, física, máquina de estados
  Movements/       o movimento dos professores
Assets/          sprites, fontes, sons e os JSON que descrevem o jogo
  Attacks/         regras.json e uma pasta por professor com as fases e
                   os projeteis dele
  materias.json    o curso inteiro
  como-jogar.json  o texto da tela Como Jogar, uma página por vez
  personagens.json o catálogo de peças do personagem
Documentacao/    receitas para quem for mexer no código
Ferramentas/     geradores de arte e de som provisórios, e as medições: o jogo
                 rodando sozinho (medir-dificuldade.sh), o portão do aluno
                 parado (parado-reprova.sh) e as 64 batalhas que uma
                 refatoração não pode mudar (linha-de-base.sh)

tests/           a suíte
```

Dentro de cada pasta, o par `X.cpp` / `XArquivo.cpp` separa a camada pura (sem
SDL nem disco) da ponte que lê arquivo e fala com SDL. Essa separação é o que
torna a suíte possível:
a metade pura entra nos testes, a metade que toca disco e SDL fica de fora.

### Padrões de projeto

| Padrão | Onde | O que resolve |
|---|---|---|
| **Component** | `Actor` + `Components/` | um ator é a soma do que ele tem (desenho, colisor, corpo, máquina de estados), e não uma hierarquia de tipos |
| **State** | `FSMComponent` / `FSMState` | as fases de cada professor: cada uma é um estado com a sua duração e os seus ataques |
| **Strategy** | `IAttackStrategy`, `IMovementStrategy` | o ataque e o movimento do professor são trocáveis por fase, e vêm do JSON |
| **Strategy** | `PoderEmCampo` | o poder do orientador do TCC: um objeto por poder, com o seu estado, criado por uma fábrica |
| **Factory Method + Template Method** | `IBossFactory::CreateBoss`, `FabricaDeChefe<T>` | a montagem de todo professor tem os mesmos passos, escritos uma vez; o que é de cada chefe — a classe, a pasta, o retrato — é uma linha em `Chefes.cpp` |
| **Object Pool** | `ProjectilePool` | os projéteis são reaproveitados, e não alocados no meio da luta |
| **Observer** | `OuvinteDaBatalha` | a batalha anuncia o que acontece; o registro da tentativa, os clarões da moldura e o relatório da simulação ouvem |
| **Inversão de dependência** | `Colisoes::Regras` | as colisões acham quem encostou em quem e não conhecem a batalha; o que cada contato vale (a nota, a devolutiva, a dupla) é a batalha que decide, pela interface |

### A batalha

`Battle` é a cena da luta: orquestra o quadro e guarda o estado da prova. O
resto mora em colaboradores, um por assunto:

| Classe | O que faz |
|---|---|
| `Nota::Conta` | a nota, os acertos sofridos e as fases limpas — pura, e testada |
| `FaixaDaNota` | a barra da nota, o número e os avisos na base da tela |
| `Colisoes` | quem encosta em quem, na ordem em que vale |
| `EfeitosDaBatalha` | as faíscas, a onda do Ponto Extra e as moedas |
| `DesenhoSobreOsAlunos` | o feixe e o escudo da dupla, o sinal, o valeu, a mira presa |
| `PausaDaBatalha` | o menu da pausa e a contagem 3-2-1 |
| `TelaDoResultado` | o resumo, a devolutiva e o toca aqui |
| `TelaDeCarregamento` | o véu enquanto os projéteis são preparados |

Uma refatoração nela tem de passar pela linha de base
(`Ferramentas/linha-de-base.sh`): as 64 batalhas idênticas, byte a byte.

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
