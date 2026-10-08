# Os arquivos de dados

Quase tudo que define uma batalha está em JSON. Esta é a referência de cada
arquivo, campo a campo, com o que costuma dar errado em cada um.

Todos aceitam **comentários `//`**, que são removidos antes da leitura. Use-os: é
onde mora a razão de cada número.

Nenhum erro aqui é silencioso. A leitura junta os problemas numa lista, cada um
com o conjunto, a fase e o ataque em que aconteceu, e `tests/test_arquivos_de_dados.cpp`
lê os `Assets/` de verdade e quebra a suíte ao primeiro.

| Arquivo | O que descreve |
|---|---|
| `Assets/materias.json` | o curso: matérias, colunas, desbloqueio, professor |
| `Assets/Attacks/fases.json` | as fases de cada professor e os ataques de cada fase |
| `Assets/Attacks/regras.json` | o que cada projétil faz depois de criado |
| `Assets/Attacks/projeteis.json` | o catálogo de projéteis de cada professor |
| `Assets/personagens.json` | as peças de que o personagem do aluno é montado |

---

## `materias.json`

```json
{
  "materias": [
    { "codigo": "INF110", "nome": "INF 110",
      "nomeCompleto": "Programação 1", "professor": "André",
      "coluna": 0, "chefe": "andre",
      "desbloqueio": { "tipo": "sempre" } },

    { "codigo": "INF330", "nome": "INF 330",
      "nomeCompleto": "Teoria e Modelos de Grafos", "professor": "Salles",
      "coluna": 2,
      "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"] } },

    { "codigo": "TCC", "nome": "TCC",
      "nomeCompleto": "Trabalho de Conclusão de Curso", "professor": "Hugo",
      "coluna": 4,
      "desbloqueio": { "tipo": "aprovadasNaColuna", "coluna": 3, "quantas": 2 } }
  ]
}
```

| Campo | |
|---|---|
| `codigo` | a chave permanente. **É por ele que a nota do aluno é guardada.** |
| `nome` | o que aparece no botão |
| `nomeCompleto` | o nome da disciplina. Pode faltar — é só não mostrar. |
| `professor` | quem dá a matéria. **Não** escolhe chefe nenhum; ver abaixo. |
| `coluna` | em que coluna da grade o botão fica (0 é a da esquerda) |
| `chefe` | o nome da fábrica, como registrado em `Game::InitializeBossFactory`. Pode faltar: a matéria aparece e espera uma batalha. |
| `desbloqueio` | quando ela abre |

**Tipos de desbloqueio:**

- `"sempre"` — sempre aberta
- `"aprovadoEm"` com `"materias": [...]` — abre quando todas elas estiverem aprovadas
- `"aprovadasNaColuna"` com `"coluna"` e `"quantas"` — abre com N aprovações naquela coluna

> **A ordem da lista é a ordem dos botões**, e o índice dela é o que a navegação
> das setas usa. Reordenar muda para onde as setas levam — e há testes que dizem
> o que acontece. O que a reordenação **não** faz é mexer na nota de ninguém: as
> fichas são guardadas por `codigo`, nunca por posição.

> **`professor` e `chefe` são coisas diferentes.** A INF 330 é do Salles e não
> tem batalha — e o Salles já é o chefe da INF 213. Apontar o `chefe` dela para
> `"salles"` faria duas matérias idênticas de jogar. O professor é quem assina a
> disciplina; o chefe é a luta que existe no jogo. Há teste exigindo que as duas
> possam divergir.

> **A coluna 0 não precisa ter uma matéria só.** A grade se monta a partir desta
> lista; se uma coluna ficar vazia, as setas a pulam em vez de parar nela.

---

## `fases.json`

Um conjunto por professor, com quatro fases de nomes fixos: `StateOne`,
`StateTwo`, `StateThree`, `StateFinal`. Nome inventado é recusado na leitura.

```json
"thiago": {
  "StateOne": {
    "duracao": 17,
    "proximo": "StateTwo",
    "movimento": { "tipo": "RandomWander", "a": 2.5, "b": 320 },
    "ataques": [
      { "estrategia": "ConsultaAttack", "projetil": "Celula",
        "projeteis": 6, "velocidade": 620, "cooldown": 0.9,
        "consulta": { "eixo": "Linha", "linhas": 3, "aviso": 1.1 },
        "regras": "thiago_varredura" }
    ]
  }
}
```

### A fase

| Campo | |
|---|---|
| `duracao` | segundos |
| `proximo` | a próxima fase. **Vazio significa "a batalha é decidida aqui"**, não "fim do arquivo". |
| `movimento` | como o professor anda |
| `ataques` | lista; todos disparam na mesma fase, cada um no seu cooldown |

**Movimentos:** `GoToCenter`, `RandomWander`, `HoverAbovePlayer`. `a` e `b` são
os dois parâmetros do construtor, na ordem, e cada movimento lhes dá um sentido
próprio — a tabela está em `FasesDeAtaqueArquivo.cpp`, onde eles são instanciados.

### O ataque

| Campo | Vai para |
|---|---|
| `estrategia` | qual padrão: `AngledAttack`, `CircleSpreadAttack`, `WaveAttack`, `BaloonAttack`, `LaserAttack`, `ConsultaAttack` |
| `projetil` | o nome dentro do conjunto deste professor em `projeteis.json` |
| `projeteis` | quantos por disparo |
| `velocidade` | px/s |
| `angulo` | a **abertura** do leque, em graus |
| `anguloCentral` | o **meio** do leque. 0 = direita, 90 = baixo, 180 = esquerda |
| `intervalo` | atraso entre um projétil e o seguinte — só a `WaveAttack` lê |
| `cooldown` | segundos entre dois disparos |
| `regras` | o nome de um conjunto de `regras.json`, ou a lista escrita ali mesmo |

> **Campo omitido = padrão do motor.** A ponte só escreve no `AttackParams` o que
> o arquivo disser; o resto fica com o padrão que a struct já tem. Não existe
> "zero por omissão" em lugar nenhum.

> **`anguloCentral` pode ser sobrescrito** pelo professor em tempo de execução,
> em `CustomizeAttackParams`. Se um valor escrito aqui parece não ter efeito,
> é lá que ele está sendo substituído.

**Cooldown derivado.** Em vez de um número, o cooldown pode ser calculado a
partir do ritmo do ataque:

```json
"cooldown": { "doRitmo": "total", "dividirPor": 7 }
```

`doRitmo` é `"total"` ou `"ciclo"`. Serve para que mexer no ritmo não exija
recalcular o cooldown à mão.

### Blocos de estratégia

Duas estratégias não se contentam com os campos comuns e exigem um bloco próprio.
Sem ele, a leitura recusa — um ataque que não dispara é mais difícil de achar que
um arquivo recusado.

**`balao`**, obrigatório na `BaloonAttack`:

```json
"balao": { "lado": "Up", "spawnAleatorio": true,
           "centradoNoJogador": true, "deslocamento": 600 }
```

`lado` é `Down`, `Left`, `Right` ou `Up` — e a convenção é a de onde o balão
**nasce**: `Down` nasce abaixo da tela e sobe. Com `spawnAleatorio` falso é
preciso dar `pontosDeSpawn`, uma lista de `[x, y]`.

**`consulta`**, obrigatório na `ConsultaAttack`:

```json
"consulta": { "eixo": "Linha", "linhas": 4, "indice": 0, "aviso": 0.9,
              "invertido": true }
```

| Campo | |
|---|---|
| `eixo` | `Linha` ou `Coluna`. Obrigatório. |
| `linhas` / `colunas` | a forma da tabela |
| `indice` | qual faixa varrer. **Omitir deixa o professor escolher** em `CustomizeAttackParams` — é assim que a consulta persegue o jogador. |
| `aviso` | segundos de telégrafo |
| `invertido` | entra pelo outro lado |

> **O `aviso` é o ataque inteiro.** É nele que os projéteis ficam parados e
> visíveis na borda, e é como o jogador descobre qual faixa foi escolhida. Com 4
> linhas numa tela de 775, uma faixa tem 194 px; o jogador anda 300 px/s, logo
> atravessar custa 0,65 s. **Aviso abaixo disso torna alguma posição
> inescapável.** Negativo é recusado na leitura.

> **`indice` fora da tabela é recusado.** Já houve uma fase final inteira cujos
> quatro ataques diziam varrer as faixas 0, 2, 1 e 3 de uma tabela de **uma**
> faixa: os quatro varriam a tela toda e nada avisava.

> **O número de projéteis segue a espessura da faixa.** A célula tem 32 px
> desenhados; uma linha de 4 mede 194 px, logo 7 projéteis fazem uma parede
> sólida. Mais que isso só empilha projétil no mesmo lugar.

---

## `regras.json`

Um conjunto é uma lista de regras aplicadas a **cada** projétil da rajada, na
ordem em que aparecem.

```json
"salles_lista_simples": [
  { "animacao": "ListaSimples" }
],

"andre_fase3": [
  { "quando": "pares",
    "motion": { "tipo": "Wobble", "amplitude": 40, "frequencia": 1.5, "duracao": 2.5 } },
  { "quando": "impares",
    "motion": { "tipo": "Wobble", "amplitude": -40, "frequencia": 1.5, "duracao": 2.5 } }
]
```

### A quem a regra se aplica

Escolha no máximo uma forma:

| | |
|---|---|
| *nada* | a todos os projéteis da rajada |
| `"quando": "sempre" \| "pares" \| "impares"` | por posição na rajada |
| `"indices": [0, 3, 7]` | posições específicas |
| `"chance": 0.2` | sorteio por projétil |

### O que a regra faz

| | |
|---|---|
| `motion` | **uma** por projétil. Decide a direção e **preserva** o módulo. Uma segunda substitui a primeira. |
| `modifiers` | quantas quiser. Mudam o módulo ou agendam coisas, e acumulam. |
| `animacao` | nome de uma animação registrada no projétil |
| `escala` | multiplica o tamanho — e a hitbox junto, porque o colisor multiplica o raio pela escala |

**Motions:** `Path`, `Tracking`, `Wobble`, `MiraPeriodica`
**Modifiers:** `Accelerate`, `SlowDown`, `Activate`, `Deactivate`, `PulsoDeVelocidade`

Pôr um modifier em `motion` é recusado na leitura. No C++ quem garante isso é o
compilador; aqui é a leitura, porque arquivo de texto não passa por compilador.

### Campos de cada tipo

| Tipo | Campos |
|---|---|
| `Path` | `forma`, `formaPorIndice`, `mira`, `parametroDaMira`, `velocidade`, `atraso`, `pararNoPonto`, `pararPor` |
| `Tracking` | `forca`, `duracao`, `atraso` |
| `Wobble` | `amplitude` (graus), `frequencia`, `duracao`, `atraso` |
| `MiraPeriodica` | `ritmo` |
| `Accelerate` / `SlowDown` | `fator`, `atraso` |
| `Activate` / `Deactivate` | `atraso` |
| `PulsoDeVelocidade` | `ritmo`, `moduloInvestida`, `moduloPausa` |

`mira` é `AlinharComVelocidade` (o padrão), `MirarNoJogador` ou `MirarPrevendo`.

### O que costuma dar errado

> **`velocidade` num `Path` é a do caminho, não a do disparo.** A 340–400 px/s um
> laço vira um risco na tela, e por isso quase todo caminho usa menos que o
> ataque.

> **Motion captura a direção no instante em que ativa.** Se o projétil estiver
> parado nesse momento — por um `Deactivate` ou por ter nascido assim — não há
> direção para capturar e ele sai para a direita. Foi isso que obrigou o
> `"atraso": 0.85` do `salles_duplamente` a ser maior que o despertar do último
> nó da fila: mexer em `projeteis` ou `intervalo` naquela fase **obriga** a
> revisar esse atraso.

> **Motions que escrevem velocidade não compõem com modifiers que a modificam.**
> `Wobble` e `Path` reescrevem a velocidade todo frame a partir do que
> capturaram, então `Accelerate`, `SlowDown` e `Tracking` no mesmo projétil são
> desfeitos no frame seguinte. `Activate` e `Deactivate` continuam seguros.

> **`Deactivate` apaga o sprite mas NÃO desliga o colisor.** Um projétil
> "desativado" é uma hitbox invisível. Para esperar antes de disparar sem criar
> dano invisível, deixe o projétil nascer visível com velocidade zero e use só o
> `Activate` — é o que a `ConsultaAttack` faz.

> **`PulsoDeVelocidade` com pausa zero perde a direção.** A direção mora dentro
> do vetor velocidade; com módulo zero não há o que girar. Use 10 a 20 px/s.

---

## `projeteis.json`

Um conjunto por professor. O nome de cada entrada é como `fases.json` o pede.

```json
"thiago": {
  "Celula": {
    "sprite": "Teachers/Projectiles/DPIBHThiagoCelula.png",
    "dados":  "Teachers/Projectiles/DPIBHThiagoCelula.json",
    "animacoes": { "Linha": [0] },
    "animacaoInicial": "Linha",
    "escala": 2,
    "ordemDeDesenho": 90,
    "posicionarNoDono": false,
    "margemEmSprites": 2,
    "margemDivisorDeTela": 0
  }
}
```

| Campo | |
|---|---|
| `sprite` / `dados` | caminhos **sem** `Assets/` na frente |
| `animacoes` | nome → lista de índices de quadro |
| `animacaoInicial` | com qual começar |
| `escala` | multiplica o desenho **e a hitbox** |
| `ordemDeDesenho` | menor desenha antes, ou seja, atrás |
| `posicionarNoDono` | se nasce em cima do professor |
| `margemEmSprites` / `margemDivisorDeTela` | quão longe da tela ele morre |

A regra geral de morte é um sprite mais um doze avos da tela. Os balões do André
usam dois sprites e nenhuma parcela, e por isso somem mais cedo — era o único
motivo de existir uma subclasse de projétil só para eles.

> **Cada tipo registrado custa 300 instâncias no pré-aquecimento.** Variações de
> cor ou de forma do mesmo projétil devem ser **animações**, não tipos novos.

---

## `personagens.json`

O catálogo de peças com que a aparência do aluno é montada. Cada peça é uma
camada de arte, e a aparência final é a pilha dessas camadas tingidas.

```json
{
  "fixas": [
    { "arte": "Player/Camadas/sapato", "ordem": 30 }
  ],
  "categorias": [
    { "id": "pele", "nome": "Tom de pele", "ordem": 10,
      "cores": [ { "id": "tom1", "nome": "Tom 1", "rgb": "f6cfa6" } ],
      "pecas": [ { "id": "corpo", "nome": "Corpo", "arte": "Player/Camadas/corpo" } ] }
  ],
  "predefinidas": [
    { "id": "a", "nome": "Aluno 1",
      "escolhas": { "pele": { "peca": "corpo", "cor": "tom2" } } }
  ]
}
```

| | |
|---|---|
| `fixas` | camadas que todo personagem tem, sem escolha. Só `arte` e `ordem`. |
| `categorias` | o que o jogador escolhe: uma peça e uma cor por categoria |
| `ordem` | a ordem de empilhamento; maior fica por cima |
| `cores` | a paleta daquela categoria, com `rgb` em hexadecimal sem `#` |
| `pecas` | as formas daquela categoria, cada uma com a sua `arte` |
| `predefinidas` | combinações prontas, usadas como ponto de partida |

As ordens hoje são pele 10, sapato 30, calça 40, camisa 50 e cabelo 60 — e os
buracos entre elas são de propósito, para caber camada nova no meio sem
renumerar as outras.

> **A ficha guarda o `id` da peça, nunca a posição dela na lista.** Acrescentar
> uma peça no meio de uma categoria não troca o cabelo de ninguém.

---

## Onde cada um é lido

Todo arquivo tem um par: uma função **pura** que recebe o texto e devolve dados
mais uma lista de problemas, e uma **ponte** que lê o disco e monta os objetos do
motor.

| Arquivo | Camada pura | Ponte |
|---|---|---|
| `materias.json` | `Materias::LerMaterias` | `MateriasArquivo.cpp` |
| `fases.json` | `LerFases` | `FasesDeAtaqueArquivo.cpp` |
| `regras.json` | `LerRegras` | `RegrasDeAtaqueArquivo.cpp` |
| `projeteis.json` | `LerProjeteis` | `ProjeteisDeChefeArquivo.cpp` |
| `personagens.json` | `Personagens::LerCatalogo` | `PersonagensArquivo.cpp` |

É essa divisão que permite testar a leitura sem SDL e sem disco: os testes
entregam o texto do JSON direto para a função pura.
