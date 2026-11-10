# Os dados das tentativas

A ficha do aluno guarda o **recorde** de cada matéria, que é o que o jogo
precisa. Para estudar *como* os alunos jogam, o jogo também guarda a
**história**: uma linha a cada tentativa de batalha, com quando ela aconteceu,
quanto durou, como terminou e o que o aluno fez nela.

O código está em `Source/Pesquisa/Tentativas.h` (o formato, testado em
`tests/test_tentativas.cpp`), em `Source/Pesquisa/TentativasArquivo.cpp` (o disco) e em
`Source/Scenes/Battle/RegistroDaTentativa.cpp`, que ouve a batalha
(`OuvinteDaBatalha`) e anota tudo no instante em que acontece.

## Os quatro arquivos

Ficam em `Saves/`, ao lado das fichas. São CSV em UTF-8, separados por
**vírgula** e com **ponto** como separador decimal, sempre.

| Arquivo | Uma linha por | Como cresce |
|---|---|---|
| `tentativas.csv` | aluno, por tentativa | só acrescenta: nada já escrito é reescrito |
| `tentativas_fases.csv` | fase jogada, por tentativa | só acrescenta |
| `tentativas_resumo.csv` | aluno × matéria × dificuldade × modo | refeito inteiro a cada batalha, a partir de `tentativas.csv` |
| `tentativas_toques.csv` | toca aqui da dupla aprovada, na tela do resultado | só acrescenta |

Os dois primeiros são os **dados brutos**: guarde-os. O resumo é **derivado**:
apagá-lo não perde nada, porque ele volta na próxima batalha.

Os arquivos se juntam pela coluna `id`. Uma dupla gera **duas linhas** em
`tentativas.csv` com o mesmo `id`, uma por aluno, e as fases dela aparecem uma
vez só em `tentativas_fases.csv`.

## Quando uma tentativa é gravada

| Como a batalha acabou | `resultado` |
|---|---|
| A nota terminou em 60 ou mais | `aprovado` |
| A nota terminou abaixo de 60, ou chegou a zero | `reprovado` |
| Pausa → **Recomeçar** ou **Sair** | `abandonou` |
| A janela foi fechada no meio da luta | `abandonou` |

A tentativa começa a contar quando a tela de carregamento some e o professor
começa a atacar. Quem sai antes disso não jogou, e não gera linha. Uma queda de
energia ou um travamento também não gera linha: o jogo não teve como gravar.

**Recomeçar conta como abandono.** A tentativa que o aluno largou é uma, e a
que começa é outra, com outra linha.

**O jogo rodando sozinho não grava nada.** Com qualquer parâmetro do modo de
simulação (`--batalha`, `--piloto`, `--fotos`..., em `build-e-testes.md`), a
tentativa é anotada e não vai para os arquivos: medir ou fotografar no
gabinete não mistura batalhas de mentira com as dos alunos.

## `tentativas.csv`

| Coluna | O que é |
|---|---|
| `formato` | A versão do formato em que **esta linha** foi gravada (hoje `4`). Muda quando uma coluna muda; ver **Quando o formato muda**. |
| `id` | O identificador da tentativa, por exemplo `20261028-140322-0f3a9c21`. O começo é a data e a hora, então ordenar por `id` é ordenar no tempo. |
| `versao_do_jogo` | O commit de onde o executável foi compilado, ou `dev`. Serve para separar as tentativas de antes e de depois de uma mudança de balanceamento. É lido quando o CMake **configura** o projeto: depois de um `git pull`, rode o `cmake` de novo antes de compilar. |
| `matricula` | A matrícula do aluno, ou `visitante`. |
| `visitante` | `1` se o aluno jogou sem matrícula. |
| `posicao` | `1` ou `2`: o lado do painel em que o aluno jogou. |
| `parceiro` | Em dupla, a matrícula do outro aluno. Vazia no solo. |
| `modo` | `solo` ou `dupla`. |
| `vez` | Quantas vezes o aluno já tinha tentado esta matéria, contando esta: `1` na primeira. Conta todas as dificuldades e os dois modos, e conta os abandonos. É `0` para visitante. |
| `materia` | O código da matéria (`INF110`, `TCC`...). É a chave estável: o nome pode mudar, o código não. |
| `nome_da_materia` | O nome da disciplina, para quem lê. |
| `professor` | O professor da matéria. |
| `dificuldade` | `normal` ou `dificil`. |
| `resultado` | `aprovado`, `reprovado` ou `abandonou` (tabela acima). |
| `aprovado` | `1` se aprovado, `0` se não. Facilita somar e tirar médias. |
| `inicio`, `fim` | Data e hora locais, `AAAA-MM-DD HH:MM:SS`. |
| `data`, `hora` | As duas metades de `inicio`, separadas para filtrar por dia ou por horário. |
| `dia_da_semana` | `domingo` a `sabado`, a partir de `data`. |
| `duracao_s` | Segundos de **luta**: sem a tela de carregamento e sem o tempo em pausa. |
| `pausa_s` | Segundos com a pausa aberta, mais a contagem 3, 2, 1 da volta (a luta segue parada durante ela). |
| `nota_final` | A nota no fim. No abandono, a nota no momento em que o aluno saiu (ela **não** vai para a ficha). |
| `nota_maxima`, `nota_minima` | A maior e a menor nota durante a luta. |
| `nota_cheia` | `1` se a nota chegou a 100 em algum momento. |
| `fez_exame` | `1` se a batalha chegou à fase final (`StateFinal`). |
| `fases_jogadas` | Quantas fases a tentativa teve, inclusive a última, mesmo incompleta. |
| `fases_limpas` | Quantas fases pagaram o bônus de fase limpa (nenhum acerto sofrido). |
| `acertos_sofridos_dupla` | Acertos sofridos pela dupla inteira (no solo, igual à coluna seguinte). |
| `acertos_sofridos` | Acertos sofridos **por este aluno**. |
| `tiros` | Tiros que este aluno disparou. |
| `acertos_no_professor` | Tiros deste aluno que acertaram o professor, inclusive os que o escudo bloqueou. |
| `precisao` | `acertos_no_professor / tiros`, de 0 a 1. Vazia quando o aluno não atirou: ali não há precisão, e zero seria um dado falso. |
| `pontos_extras_pegos`, `pontos_extras_usados` | Pontos Extras que este aluno pegou e gastou. |
| `orientador` | No TCC, o professor escolhido como orientador. Vazio nas outras matérias. |
| `acertos_do_orientador` | Tiros do orientador que acertaram o Hugo. |
| `poderes_do_orientador` | Quantas vezes o poder do orientador foi usado. |
| `semente` | A variação sorteada da luta (`Variacao.h`). |

As colunas abaixo existem **desde o formato 2**. Elas contam o que o aluno
**fez**, mesmo quando a condição do experimento desliga a mecânica que paga por
isso (ver **Condições de experimento**): um raspão continua sendo um raspão sem
o bônus do raspão. O que só existe na dupla fica **vazio** no solo.

| Coluna | O que é |
|---|---|
| `condicao` | A condição do experimento em que o jogo rodava: `normal`, ou o que estava desligado (`sem-brilho+sem-papeis`). A mesma nas duas linhas da dupla. |
| `acertos_de_perto` | Acertos deste aluno no professor feitos **de perto** — os que viram "!!" (`Nota::kFatorDaRespostaDupla`). |
| `raspoes` | Projéteis que passaram rente a este aluno sem acertá-lo (`Raspao.h`), um por projétil. |
| `coberturas` | Na dupla, projéteis que o tiro deste aluno derrubou **indo no parceiro** (`Cobertura.h`). As coberturas recebidas são as do parceiro: a outra linha do mesmo `id`. |
| `oportunidades_de_cobertura` | Na dupla, projéteis destrutíveis que foram no **parceiro** — os que este aluno podia cobrir. Um por projétil, brilhe ele ou não. `coberturas / oportunidades_de_cobertura` é o quanto ele cobriu do que podia. |
| `tempo_na_mira_s` | Na dupla, segundos de luta com a mira do professor **presa** neste aluno (o "v" vermelho, `Papeis.h`). Vazio também quando a condição desliga os papéis: aí a mira nunca prende. |
| `distancia_media_px` | Na dupla, a distância média entre os dois alunos durante a luta, em pixels do campo (1200 × 800). A mesma nas duas linhas. |
| `sincronias` | Desde o formato 3. Na dupla, quantas vezes os dois gastaram o Ponto Extra **juntos** — com até meio segundo de diferença (`PontoExtra::kJanelaDaSincronia`). A mesma nas duas linhas. |
| `sinais` | Desde o formato 4. Na dupla, os sinais que **este** aluno deu ao parceiro com o Botão 3 (um por aperto, com um segundo de espera entre dois; fora do TCC). |

## `tentativas_fases.csv`

| Coluna | O que é |
|---|---|
| `formato`, `id` | Como em `tentativas.csv`. |
| `materia`, `dificuldade`, `modo` | Repetidos aqui, para analisar as fases sem precisar juntar os arquivos. |
| `matricula_1`, `matricula_2` | Os alunos da tentativa (`matricula_2` é vazia no solo). |
| `resultado` | Como a tentativa inteira terminou. |
| `ordem` | `1` para a primeira fase jogada, `2` para a segunda... |
| `fase` | O nome interno da fase: `StateOne`, `StateTwo`, `StateThree`, `StateFinal`. |
| `anuncio` | O nome que o placar mostra no começo da fase (`if / else`, `Defesa`...). |
| `inicio_s`, `fim_s`, `duracao_s` | Em segundos de luta, contados do começo da tentativa. |
| `nota_no_inicio`, `nota_no_fim`, `variacao` | A nota ao entrar, ao sair, e a diferença. A nota de saída já inclui o bônus de fase limpa. |
| `acertos_sofridos` | Acertos que a dupla sofreu nesta fase. |
| `limpa` | `1` se a fase pagou o bônus de fase limpa. |
| `completa` | `1` se a fase terminou pelo relógio. `0` se a luta acabou no meio dela: nota a zero ou abandono. |
| `acertos_de_perto`, `raspoes` | Desde o formato 2: os da dupla inteira, nesta fase. |
| `coberturas` | Desde o formato 2: as da dupla, nesta fase (vazia no solo). Com `acertos_sofridos`, mostra em que fase a dupla mais se ajuda. |

## `tentativas_toques.csv`

Na dupla aprovada, o resultado deixa os dois apertarem o Botão 3 juntos — o
**toca aqui** (`requisitos-coop.md`, 2.9). Quando isso acontece, a tentativa já
está gravada em `tentativas.csv`: por isso cada toque é uma linha neste arquivo,
ligada a ela pelo `id`. Uma tentativa sem linha aqui é uma dupla que não
comemorou (ou que não passou).

| Coluna | O que é |
|---|---|
| `formato`, `id`, `materia`, `dificuldade` | Como em `tentativas.csv`. |
| `matricula_1`, `matricula_2` | Os dois alunos. |
| `toque` | `1` no primeiro toca aqui daquela tela, `2` no segundo... |
| `segundos_no_resultado` | Quantos segundos depois de o resultado aparecer. |

## `tentativas_resumo.csv`

Uma linha por aluno, matéria, dificuldade e modo. Os **visitantes ficam de
fora**: várias pessoas diferentes jogam como visitante, e somá-las seria somar
alunos diferentes.

| Coluna | O que é |
|---|---|
| `tentativas` | Quantas vezes jogou. |
| `aprovacoes`, `reprovacoes`, `abandonos` | Quantas terminaram de cada jeito. |
| `tentativas_ate_aprovar` | Em qual tentativa passou pela primeira vez, contando as tentativas desta mesma dificuldade e modo. Vazia se ainda não passou. |
| `primeira_tentativa`, `primeira_aprovacao`, `ultima_tentativa` | Datas e horas. |
| `dias_distintos` | Em quantos dias diferentes jogou. |
| `melhor_nota`, `nota_media` | Sobre todas as tentativas, inclusive as abandonadas. |
| `tempo_total_s` | A soma de `duracao_s`. |

## Quando o formato muda

Coluna nova entra **sempre no fim**. Quando o jogo vai gravar num arquivo de um
formato anterior, ele troca só a **primeira linha** — o cabeçalho — pela do
formato atual, e as linhas antigas ficam como estavam: mais curtas, e com o
formato delas na coluna `formato`. Lido pelo nome das colunas (pandas, R,
Excel), o que falta numa linha antiga aparece **vazio** — não zero.

Se o arquivo tiver um cabeçalho que não é o começo do atual (outra coisa
qualquer com o mesmo nome), ele é guardado de lado como
`tentativas_de_outro_formato_<data>.csv`, e um arquivo novo começa. Nada se
apaga.

## Condições de experimento

Para comparar o jogo **com e sem** uma mecânica — o padrão de jogo como
variável do estudo —, o gabinete abre com `--condicao` (ver `gabinete.md`):

```bash
./DPIBulletHellRe --arcade --condicao sem-brilho
./DPIBulletHellRe --arcade --condicao sem-papeis,sem-cobertura
```

| Nome | O que deixa de existir |
|---|---|
| `sem-brilho` | O projétil que vai num aluno não brilha na cor dele. |
| `sem-cobertura` | Derrubar o que ia no parceiro não paga mais que uma derrubada comum, nem acende dourado. Apaga o brilho junto: ele prometeria o que não paga. |
| `sem-papeis` | A mira não prende em quem mais incomoda: os dois revezam, como antes dos papéis, e o "v" vermelho some. |
| `sem-raspao` | Passar rente não paga nem acende o risco azul. |
| `sem-perto` | A dúvida de perto vale o mesmo que a de longe, e o "!!" some. |
| `sem-devolutiva` | Quem reprova não vê onde a nota mais caiu nem o que mais o acertou. |
| `sem-sincronia` | Os dois Pontos Extras gastos juntos dão só os dois escudos: a onda não sai. As `sincronias` continuam contadas. |
| `sem-valeu` | Quem foi coberto não solta o coração na cor de quem cobriu. |
| `sem-sinal` | O Botão 3 não mostra o anel nem toca o som na batalha em dupla. Os `sinais` continuam contados. |
| `sem-toque` | No resultado da dupla aprovada, o Botão 3 dos dois não faz os avatares pularem, nem estoura, nem toca a palma, e o rodapé não ensina. Os toques continuam contados em `tentativas_toques.csv`. |
| `sem-titulos` | No resultado da dupla, a parte de cada aluno volta a ser uma linha só, com os números lado a lado, sem o título do que cada um mais fez ("quem mais cobriu"...). Os títulos não vão para o CSV: saem das colunas de acertos, coberturas e acertos sofridos de cada aluno (`Devolutiva::Titulos`). |

Sem `--condicao` (ou com `--condicao normal`), tudo funciona como sempre, e a
coluna `condicao` diz `normal`. Um nome que o jogo não conhece é ignorado — e
dito no log, junto da condição em uso, ao abrir o jogo.

**Desligar tira a mecânica do jogo, e não da medida.** Com `sem-raspao`, os
raspões continuam contados em `raspoes`; com `sem-brilho`, as
`oportunidades_de_cobertura` também. É isso que permite comparar: por exemplo,
se a proporção `coberturas / oportunidades_de_cobertura` sobe com o brilho.

**Uma condição por sessão.** A condição vale para a máquina enquanto o jogo
estiver aberto; trocar é fechar e abrir com outra. Combine com o protocolo do
estudo quem joga em qual, e confira a coluna `condicao` depois.

## Respondendo às perguntas de sempre

**Quantas vezes cada pessoa jogou cada matéria, passou e reprovou?** Está
pronto em `tentativas_resumo.csv`.

**Em que dia foi cada tentativa?** Em `tentativas.csv`, cada linha traz `vez`
(a primeira, a segunda...) e `data`. Filtrar por matrícula e ordenar por `vez`
dá a linha do tempo de um aluno.

**Em qual fase os alunos mais perdem nota?** Em `tentativas_fases.csv`, a média
de `variacao` e de `acertos_sofridos` agrupada por `materia` e `fase`.

### Com pandas

```python
import pandas as pd

t = pd.read_csv("Saves/tentativas.csv")
f = pd.read_csv("Saves/tentativas_fases.csv")

t = t[t.visitante == 0]                      # só quem se identificou
t.groupby(["materia", "dificuldade"]).aprovado.mean()          # taxa de aprovação
t.groupby("vez").nota_final.mean()            # a nota melhora com a prática?
f.groupby(["materia", "fase"])[["variacao", "acertos_sofridos"]].mean()

# Desde o formato 2: a dupla
d = t[(t.modo == "dupla") & (t.formato >= 2)]
d.groupby("condicao")[["coberturas", "oportunidades_de_cobertura"]].sum()  # o brilho faz cobrir mais?
d.groupby("condicao").distancia_media_px.mean()                            # jogam mais juntos?
d[d.formato >= 3].groupby("condicao").sincronias.mean()                    # combinam o Ponto Extra?
```

### Com R

```r
t <- read.csv("Saves/tentativas.csv", encoding = "UTF-8")
aggregate(aprovado ~ materia + dificuldade, data = subset(t, visitante == 0), FUN = mean)
```

### No Excel

Abrir com dois cliques pode embaralhar as colunas: o Excel em português espera
ponto e vírgula e vírgula decimal. Use **Dados → Obter Dados → De Texto/CSV**,
escolha a origem **65001: Unicode (UTF-8)** e o delimitador **Vírgula**.

## Antes de coletar

- **Comece com os arquivos vazios.** As tentativas de teste, de quem montou a
  máquina, também viram linhas. Antes da coleta, mova os quatro arquivos para
  outra pasta (não apague: são dados também).
- **Faça cópia.** `tentativas.csv` só cresce, mas é um arquivo só numa máquina
  só. Copie a pasta `Saves/` ao fim de cada dia de coleta.
- **Anote as mudanças.** Se o balanceamento mudar no meio da coleta, a coluna
  `versao_do_jogo` separa os dois períodos, desde que o projeto seja
  reconfigurado (`cmake`) antes de compilar.
- **Uma dupla é uma tentativa.** Para contar tentativas, conte `id` distintos;
  para contar alunos, conte linhas.

## Privacidade

A matrícula identifica o aluno: estes arquivos são **dados pessoais** (LGPD).
Para usá-los no TCC:

- Peça o **consentimento** dos participantes (um termo de consentimento livre e
  esclarecido, conforme o comitê de ética da instituição exigir), dizendo o que
  é gravado e para quê.
- **Pseudonimize antes de publicar ou compartilhar**: troque cada matrícula
  (e cada `parceiro`, `matricula_1` e `matricula_2`) por um código (`A01`,
  `A02`...), e guarde a tabela de correspondência separada, fora do trabalho.
- O jogo grava só o que está descrito aqui: nenhum nome, nenhuma imagem e
  nenhum dado da máquina.
