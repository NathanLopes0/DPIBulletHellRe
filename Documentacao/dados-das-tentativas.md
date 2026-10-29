# Os dados das tentativas

A ficha do aluno guarda o **recorde** de cada matéria, que é o que o jogo
precisa. Para estudar *como* os alunos jogam, o jogo também guarda a
**história**: uma linha a cada tentativa de batalha, com quando ela aconteceu,
quanto durou, como terminou e o que o aluno fez nela.

O código está em `Source/Data/Tentativas.h` (o formato, testado em
`tests/Data/test_tentativas.cpp`), em `Source/Data/TentativasArquivo.cpp` (o disco) e em
`Source/Scenes/Battle/RegistroDaTentativa.cpp`, que ouve a batalha
(`OuvinteDaBatalha`) e anota tudo no instante em que acontece.

## Os três arquivos

Ficam em `Saves/`, ao lado das fichas. São CSV em UTF-8, separados por
**vírgula** e com **ponto** como separador decimal, sempre.

| Arquivo | Uma linha por | Como cresce |
|---|---|---|
| `tentativas.csv` | aluno, por tentativa | só acrescenta: nada já escrito é reescrito |
| `tentativas_fases.csv` | fase jogada, por tentativa | só acrescenta |
| `tentativas_resumo.csv` | aluno × matéria × dificuldade × modo | refeito inteiro a cada batalha, a partir de `tentativas.csv` |

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

## `tentativas.csv`

| Coluna | O que é |
|---|---|
| `formato` | A versão deste formato de arquivo (hoje `1`). Muda quando uma coluna muda. |
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
| `pausa_s` | Segundos com a pausa aberta. |
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
  máquina, também viram linhas. Antes da coleta, mova os três arquivos para
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
