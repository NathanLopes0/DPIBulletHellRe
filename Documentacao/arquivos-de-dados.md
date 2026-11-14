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
| `Assets/Attacks/<professor>/fases.json` | as fases daquele professor e os ataques de cada fase |
| `Assets/Attacks/<professor>/regras.json` | o que cada projétil daquele professor faz depois de criado |
| `Assets/Attacks/<professor>/projeteis.json` | o catálogo de projéteis daquele professor |
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

    { "codigo": "INF340", "nome": "INF 340",
      "nomeCompleto": "Linguagens de Programação", "professor": "Luizinho",
      "coluna": 2,
      "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"] } },

    { "codigo": "TCC", "nome": "TCC",
      "nomeCompleto": "Trabalho de Conclusão de Curso", "professor": "Hugo",
      "coluna": 4, "orientacao": true,
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
| `chefe` | o nome do chefe, como registrado em `Source/Actors/Teachers/BossFactory/Chefes.cpp` (é também a pasta dele em `Assets/Attacks/`). Pode faltar: a matéria aparece e espera uma batalha. |
| `orientacao` | `true` se a batalha pede um **orientador** antes de começar — o TCC. Entrar nela pela seleção de fases abre antes a tela de escolha: os três professores das maiores notas aprovadas (em dupla, a maior nota dos dois em cada matéria), e em dupla os dois votam. O escolhido entra na batalha voando ao lado do aluno, invulnerável, e os tiros dele pagam a nota como os do aluno. Cada um ataca com a matéria dele (`Golpe.h`), todos com o mesmo peso: o `for` do André, a pilha do Salles (LIFO), o barramento do Ricardo, a transação do Thiago (tudo sai no COMMIT), o ponteiro do Luizinho, a gramática `S → S S` do Vladimir, a descida do gradiente do Júlio, o Observer da Maria Lúcia (atira a cada quatro tiros do aluno), o `fetch` do Lucas e a janela do TCP do Vitor — e o tiro dele tem a cara da matéria. E cada um traz um **poder** (`Poder.h`), que carrega a cada acerto no chefe e sai no botão 3: o laço sem fim do André, a capivara do Salles, o `HLT` do Ricardo (que congela o chefe e os tiros dele), o `ROLLBACK` do Thiago, o `free()` do Luizinho, o autômato do Vladimir, os agentes do Júlio (os tiros caçam o alvo), a bala de prata da Maria Lúcia (que quebra o escudo do Hugo), o `Promise.all()` do Lucas (doze promessas que vão até o Hugo e voltam, desfazendo os tiros dele pelo caminho) e o `socket()` do Vitor (as paredes dos lados viram rede). Opcional, falso por padrão; um valor que não é booleano vira aviso. |
| `optativa` | `true` para uma **disciplina optativa**. Ela não entra na grade das obrigatórias: fica na **ala das optativas**, à direita do TCC na seleção de fases, e a câmera desliza até ela quando o cursor passa do TCC (`MapaDaGrade.h`). As optativas usam as colunas **depois** das obrigatórias (com o TCC na 4, a primeira optativa é a 5); cada página da ala tem quatro colunas, nos lugares das colunas 1 a 4 da primeira — o da coluna 0 fica livre para o cartão da matéria —, e a grade delas fica acima dos cartões. Uma optativa numa coluna das obrigatórias vira aviso. Ela não conta no "Aprovadas: N de M". Opcional, falso por padrão. |
| `desbloqueio` | quando ela abre |

**Tipos de desbloqueio:**

- `"sempre"` — sempre aberta
- `"aprovadoEm"` com `"materias": [...]` — abre quando todas elas estiverem aprovadas
- `"aprovadasNaColuna"` com `"coluna"` e `"quantas"` — abre com N aprovações naquela coluna

**O preço, só nas optativas.** Qualquer tipo aceita `"preco"`, em horas
complementares: cumprida a regra, a optativa ainda precisa ser **comprada**.
Na seleção de fases, o losango fechado diz quanto custa ("INF 480 · 20 h") e o
**painel de matrícula**, ao lado do cartão, mostra item por item o que ela pede (verde o
cumprido, rosa o que falta), as horas do aluno em blocos contra o preço, o prêmio
(o orientador novo) e o botão de matricular-se, que acende quando dá; o losango que já
pode ser comprado pisca "MATRÍCULAS ABERTAS" — o Botão 1 duas vezes compra, e
mudar o cursor desfaz a primeira. A compra vai para a ficha entre as atividades
(`compra@INF480`); as horas gastas são a soma dos preços das compradas, e o
saldo, as ganhas menos as gastas (`Horas::Saldo`). Na dupla, cada aluno que
ainda não a tem paga com o próprio saldo, e a compra é de todos ou de ninguém.
Numa obrigatória, o preço vira aviso e é ignorado: ninguém tranca o curso por
falta de horas.

```json
{ "codigo": "INF480", "nome": "INF 480", "coluna": 5, "optativa": true,
  "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"], "preco": 20 } }
```

**Os requisitos, além da regra.** O desbloqueio aceita também `"requisitos"`, uma
lista do que o aluno já conquistou no curso — todos valem junto com a regra, e
só então a optativa pode ser comprada:

| `tipo` | o que pede |
|---|---|
| `"mencao"` | `"quantas"` matérias (1, se faltar) com a menção `"mencao"`: `bronze`, `prata`, `ouro`, `notaCheia`, `dificil`, `dupla`, `orientou`, `pacifista`, `intocavel`, `mira` ou `dePerto` (`Mencoes::PelaChave`) |
| `"horas"` | `"quantas"` horas complementares **ganhas** — a carga, e não o saldo: gastar horas numa optativa não fecha outra |
| `"aprovadas"` | `"quantas"` obrigatórias aprovadas no curso inteiro (as optativas não contam) |

A frase do cartão junta tudo o que falta ("Precisa passar em INF 213, pelo menos
1 menção de ouro e 20 h"). As optativas do curso não pedem umas às outras, nem
matéria além da INF 213: o que as separa são as menções e o preço. O requisito que não existe, ou com uma
menção que não existe, vira aviso e fica de fora — a matéria não.

```json
"desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"], "preco": 20,
                 "requisitos": [ { "tipo": "mencao", "mencao": "ouro", "quantas": 1 } ] }
```

**A optativa dá orientador.** O professor de uma optativa aprovada é sempre um dos
três candidatos a orientador do TCC (`Orientacao::Candidatos`), mesmo com nota
menor que as outras: o orientador novo é o prêmio dela. Os outros lugares saem
das maiores notas, como sempre.

> **A ordem da lista é a ordem dos botões**, e o índice dela é o que a navegação
> das setas usa. Reordenar muda para onde as setas levam — e há testes que dizem
> o que acontece. O que a reordenação **não** faz é mexer na nota de ninguém: as
> fichas são guardadas por `codigo`, nunca por posição.

> **`professor` e `chefe` são coisas diferentes.** Uma matéria pode ter professor
> e ainda não ter batalha — o TCC foi assim até o Hugo ganhar a dele (no exemplo
> acima ele ainda está sem `chefe`). O professor é quem assina a disciplina; o
> chefe é a luta que existe no jogo. Há teste exigindo que as duas possam divergir.

> **A coluna 0 não precisa ter uma matéria só.** A grade se monta a partir desta
> lista; se uma coluna ficar vazia, as setas a pulam em vez de parar nela.

> **Quem pode orientar** (`Source/Curso/Orientacao.h`): o professor de uma matéria em
> que o aluno foi **aprovado** e que tem `chefe`, menos as que pedem orientação.
> Entram os três de nota mais alta, a maior primeiro; no empate, a que vem antes
> nesta lista; um professor aparece uma vez só. Em dupla, a nota de cada matéria
> é a **maior dos dois**, e os dois votam: discordando, vence o de nota mais
> alta. Um teste percorre toda combinação de aprovações que dá para alcançar
> jogando e confere que quem abre o TCC tem sempre os três para escolher.

---

## `<professor>/fases.json`

**Uma pasta por professor**: `Assets/Attacks/thiago/fases.json`. A raiz do
arquivo já é o mapa de fases, com quatro nomes fixos: `StateOne`, `StateTwo`,
`StateThree`, `StateFinal`. Nome inventado é recusado na leitura.

```json
{
  "StateOne": {
    "duracao": 20,
    "proximo": "StateTwo",
    "movimento": { "tipo": "RandomWander", "a": 2.5, "b": 320 },
    "ataques": [
      { "estrategia": "ConsultaAttack", "projetil": "Celula",
        "projeteis": 6, "velocidade": 620, "cooldown": 0.9,
        "consulta": { "eixo": "Linha", "linhas": 3, "aviso": 1.1 },
        "regras": "varredura" }
    ]
  }
}
```

> **Montar um professor abre só a pasta dele.** Antes era um `fases.json` com
> todos dentro: abrir a batalha do Thiago lia os cinco, e um erro no bloco do
> Júlio aparecia no log de uma batalha do Salles.

### A fase

| Campo | |
|---|---|
| `duracao` | segundos. **Toda fase dura 20** (`Fase::kDuracao`, em `Source/Attacks/Fase.h`), e um teste confere — há módulos que cortam a fase em pedaços contando com isso |
| `proximo` | a próxima fase. **Vazio significa "a batalha é decidida aqui"**, não "fim do arquivo". |
| `anuncio` | opcional. O texto do **placar**, o quadro no canto de cima, enquanto a fase dura. Sem ele o placar fica apagado |
| `intercambio` | opcional. `{ "aCada": 8, "aviso": 1.5 }`: os alunos **trocam de lugar** a cada `aCada` segundos da fase, com aviso `aviso` segundos antes |
| `etapas` | opcional. A fase em **trechos com nome**, em ordem: `[ { "nome": "Requisitos", "duracao": 4.0, "anuncio": "1 · Requisitos" }, ... ]`. Cada ataque que diz `"etapa"` só dispara na sua, e o placar mostra o `anuncio` da etapa corrente (sem ele, o `nome`). A leitura recusa etapas que passam da fase, e um teste cobra que elas a **cubram inteira** |
| `movimento` | como o professor anda. Um bloco `"dificil": { "a": 280 }` dentro dele troca o `a` e/ou o `b` no modo Difícil (o Salles vai aos baldes a 220 no Normal e a 280 no Difícil); chave que não é `a` nem `b` é avisada e ignorada |
| `ataques` | lista; todos disparam na mesma fase, cada um no seu cooldown |

> **O placar escreve na Carlito, e não na fonte do jogo.** A Saira não tem `ⁿ` nem `δ`,
> e o placar existe para o Vladimir escrever `(ab)*` e `aⁿbⁿ`. O texto é UTF-8:
> sobrescrito, `δ` e `→` saem como são. Um `anuncio` que não é texto é recusado
> na leitura em vez de virar um quadro apagado — e cuidado com a chave: escrita
> com acento, `anúncio`, ela é ignorada calada como qualquer campo desconhecido.

> **As etapas são um processo em ordem**, a cascata da Maria Lúcia: uma etapa só
> começa quando a anterior acaba. Um ataque fora da sua etapa não conta o
> cooldown nem dispara, e quando ela abre o primeiro disparo sai em 0,25 s —
> **menos na etapa que abre a fase**, que dispara a 1 s, como todo ataque no
> começo de uma fase (`PrimeiroDisparoNaJanela`, em `FasesDeAtaque.h`). A
> soma das durações tem de caber na fase, os nomes não se repetem, e um ataque
> que pede uma etapa que a fase não tem é descartado com aviso. O movimento
> `Escada` (`a` = segundos por degrau, `b` = degraus) é o diagrama da cascata:
> um degrau por etapa, da esquerda para a direita — e um teste confere que os
> dois relógios batem.

> **O Intercâmbio funciona sozinho e em dupla.** Em dupla, os dois trocam de
> lugar; sozinho, o aluno vai para a posição espelhada (`x` vira
> `largura − x`, na mesma altura) — o lugar onde o outro aluno estaria. Durante
> o aviso o placar diz "Programa de Intercâmbio" (ou o que o professor disser,
> em `Boss::AvisoDoIntercambio`) e um contorno marca onde cada um vai cair. No
> Vladimir o intercâmbio vale nas quatro fases, e no quadro da troca as letras
> em voo trocam também — o "a" passa a perseguir e o "b" segue reto
> (`Alfabeto.h`, `Vladimir::OnIntercambio`); por isso o placar dele avisa
> "Intercâmbio: a ↔ b". O destino é recalculado todo quadro: sozinho, o aluno escolhe
> onde cai andando; em dupla, cada um cai onde o parceiro está **agora**. As
> contas estão em `Intercambio.h`. `aCada` zero, `aviso` negativo ou do tamanho
> de `aCada`, e campo faltando são recusados na leitura.

**Movimentos:** `GoToCenter`, `RandomWander`, `HoverAbovePlayer`, `Orbitar`,
`Varredura`, `Mergulho`, `Escada`, `Deslocamento`, `OndaQuadrada`, `Conduzido`. `a` e `b` são os dois parâmetros do construtor, na
ordem, e cada movimento lhes dá um sentido próprio — a tabela está em
`FasesDeAtaqueArquivo.cpp`, onde eles são instanciados.

Os três últimos existem para **o movimento ser o ataque**: a órbita, o ir-e-vir
e o mergulho são gestos, e cada um deles é a ideia de uma fase do Vladimir em
vez de um jeito de o chefe não ficar parado. Ver `MovementStrategies.h`. O
`Deslocamento` (`a` = segundos em cada coluna, `b` = colunas) é o registrador de
deslocamento do Ricardo: um passo seco de uma coluna por vez, de ponta a ponta,
sempre parando no meio de uma das colunas da palavra de bits. A `OndaQuadrada`
(`a` = amplitude, `b` = velocidade de lado a lado) é o clock dele: em cima no
nível alto, embaixo no baixo, saltando nas bordas da frequência que o placar da
fase anuncia (`Relogio::LerFrequencia`; 1 Hz se ele não disser). O `Conduzido`
(`a` = velocidade) leva o professor aonde ele mesmo diz (`Boss::AlvoDoMovimento`)
e para lá: é o autômato do Vladimir, que voa de estado em estado conforme a
palavra que lê.

> **Um chefe que anda o tempo todo não leva tiro do piloto que persegue.** O
> tiro leva quase 1 s até o professor; andando uma coluna a cada 0,6 s, o
> Ricardo foi acertado 8 vezes na fase inteira. Parando 2 s em cada coluna, 94.

> **Chefe parado deixa a tela vazia, e isso não aparece numa foto só.** A
> primeira versão do Vladimir punha `GoToCenter` nas quatro fases: cada rajada
> caía na mesma coluna do meio e os lados do campo nunca viam um símbolo. Pior,
> com a origem fixa as frestas dos ataques também ficam fixas, e um alvo parado
> numa fresta fica nela a batalha inteira.

### A fase que se divide em dois corpos

Uma fase pode declarar um **clone**: enquanto ela dura, há *dois* chefes em
campo, cada um com os ataques (e o movimento) de uma fase diferente — e
**qualquer um dos dois toma dano**.

```json
"StateThree": {
  "duracao": 20, "proximo": "",
  "movimento": { "tipo": "Varredura", "a": 190, "b": 150 },
  "ataquesDe": "StateOne",
  "clone": { "ataquesDe": "StateTwo" }
}
```

| Campo | O que faz |
|---|---|
| `ataquesDe` | esta fase usa os ataques de **outra**, em vez de declarar os seus. Não pode vir junto com `ataques` |
| `clone.ataquesDe` | de qual fase o **segundo corpo** tira os ataques e o movimento |

**Por que `ataquesDe` e não a lista repetida.** Na fase que se divide, o corpo
original costuma repetir o ataque de uma fase anterior. Escrever a lista de novo
seria a mesma coisa dita duas vezes, e duas cópias divergem no dia em que
alguém ajusta uma.

**Sem corrente e sem apontar para si mesma.** `A → B → C` exigiria detectar
ciclo e não diz nada que apontar direto para a origem não diga melhor; e uma
fase que empresta de si mesma é uma fase sem ataques. Os dois casos são
recusados na leitura, com a frase dizendo qual fase errou.

> **O segundo corpo fica PRESO na fase dele.** Ele roda a mesma máquina de
> estados do original — é a mesma fábrica que o monta, e é isso que o deixa
> barato —, mas `BossAttackState` não o transiciona. Sem isso ele avançaria
> sozinho e, ao chegar na fase que se divide, criaria um clone do clone; pior,
> ao fim da terceira chamaria `EncerrarBatalha` por conta própria e encerraria a
> luta no tempo dele.

O segundo corpo nasce no **espelho** do original (`Battle::DividirChefe`). O
Vitor faz a passagem pela nuvem: as nuvens da fase 2 se juntam em cima dele, ele
entra, e saem dois — o outro anda da nuvem até o espelho. Isso é código dele
(`Vitor::ComecarOFork`, com os tempos em `Nuvem::Fork`), e não um campo: a
fase só declara o `clone`.

### O ataque

| Campo | Vai para |
|---|---|
| `etapa` | opcional. Em qual etapa da fase o ataque dispara. Sem ela, a fase inteira |
| `gatilho` | opcional. O ataque **não dispara pelo relógio**: dispara quando o professor puxa o gatilho (`Boss::Disparar`), e o `cooldown` é ignorado. Só existem os gatilhos que algum professor puxa — `notificacao` e `consulta` (o padrão Observer da Maria Lúcia), `agir` (os agentes do Júlio), `token` (cada token da fase 1 dele, que nasce na ponta da frase gerada), `nuvens` (as nuvens do Vitor), `portas` (a fileira de portas lógicas do Ricardo), `contador` (o leque do clock dele, a cada segundo), `estagios`, `buscar`, `lw`, `sw`, `add` e `addi` (o pipeline dele: as molduras, a busca e o ataque de cada instrução no seu estágio), `acender` (o LED das placas do Blink dele), `estados`, `diagrama`, `ler_a`, `ler_b`, `aceita` e `rejeita` (o autômato da fase 1 do Vladimir: o mapa, a letra lida e o veredito), `lexema`, `arvore`, `reduzir`, `emitir` e `store` (o compilador da fase 2 dele: o token que cai no léxico — o `a` e o `b` caem por `ler_a` e `ler_b` —, o nó que voa até a árvore sintática, o leque de cada operador reduzido, a instrução que cada nó vira na geração de código e o anel do STORE), `escrever`, `apagar`, `reescrever`, `reler` e `pare` (o castor mais ocupado da fase 3 dele, uma máquina de Turing de verdade: a marca escrita na casa, a marca apagada que cai reta, a apagada que no Difícil vira um "b" que persegue, a cópia da marca relida e o anel de quando a máquina para), `metade`, `triplo` e `parou` (o 3x + 1 da repescagem dele), `capivara` (a que atravessa o campo do Salles em qualquer fase), `push` e `estourar` (a pilha da fase 1 dele: o bloco empilhado, e o desempilhado que estoura em lascas), `baldes`, `chave`, `percorrer` e `nulo` (a tabela hash da fase 2: os baldes, a chave inserida, cada nó lido na busca e o NULL do fim da lista), `no`, `visitar`, `aresta`, `folha_nula` e `achado` (a árvore de busca da fase 3: a chave que desce e vira nó, cada nó por onde ela passa, os pontos da aresta, a busca que não acha e a que acha), `elemento`, `trocar` e `assentar` (o bubble sort da repescagem dele: o elemento do vetor, a capivara de cada troca e o anel de quem chega ao lugar), `cabecalho` e `registro` (a tabela escrita no campo do Thiago: o cabeçalho de cada coluna e o valor de cada casa), `marcar` (o marcador da casa que a consulta vai ler), `selecionar`, `projetar` e `celula` (as três formas do SELECT dele: a linha do WHERE, a coluna projetada e a casa do cruzamento), `caco` (os cacos de um valor que o DELETE dele derrubou e se espatifou no chão), `selo` (o selo de uma transação aberta na fase 3 dele, que o aluno derruba para provocar o ROLLBACK), `bala` e `estilhaco` (a repescagem dela, "não há bala de prata": a bala com o nome de uma promessa, e o leque de bugs em que ela se desfaz), `rotulo`, `balao` e `confete` (o fluxograma da fase 1 do André: o cabeçalho de cada ramo, o balão numerado que a condição manda para um ramo e o confete dele estourado), `indice`, `subir` e `core` (o vetor no chão da fase 2 dele: o índice de cada casa, o cacho de balões que sobe da casa por onde o `i` passa e a memória despejada pelo segfault), `cartao`, `valor` e `leque` (as funções da fase 3 dele: o cartão com a assinatura de cada função, o valor que vai como argumento e volta como retorno, e o leque de ponto-e-vírgulas), `trofeu` (o balão de cada problema da prova no BOCA, que sobe no YES), `resposta` (o valor de x que o bloco dono atira na fase 1 do Luizinho, as amarrações: o x é procurado de bloco em bloco, e quem o declara responde com um leque de tantos tiros quanto o valor, mirado em você), `operador`, `operando`, `anel`, `texto`, `erro` e `compilacao` (a coerção da fase 2 dele: o operador em cima do aluno, os dois operandos que voam até ele, o anel do resultado número, a palavra do resultado texto, os estilhaços do TypeError onde a conta roda e os do erro de compilação no professor), `volta` e `contradicao` (o programa Termina da fase 3 dele: cada volta do laço é um anel saindo do professor, e o laço que roda desenha uma espiral; a `contradicao` é o anel de cacos do Termina, que estoura quando o Paradoxo o contradiz), `excecao`, `destroco` e `finally` (a pilha de chamadas da final dele: a exceção que sobe, a cortina de cada faixa desempilhada e o anel do `finally`), `instrucao` (a instrução de uma thread na fase 1 do Vitor, os semáforos: desce até a seção crítica, e sem o semáforo para na borda e se empilha), `segmento`, `encaminhar` e `descarte` (a fase 2 dele, o TCP: o segmento que voa da nuvem ao roteador, o que o roteador encaminha mirado no aluno e o anel de cacos do que ele descarta), `envelope`, `processar` e `encontro` (a troca de mensagens da fase 3 dele: o envelope que corre pelo canal entre o cliente e o servidor, a mensagem processada que vai para o aluno e o anel do rendezvous), `garfo`, `garfada` e `espera` (o jantar dos filósofos da final dele: o garfo que anda de mão em mão, o leque de quem come e o cerco do deadlock), `cursor`, `clique`, `spinner`, `carregar`, `marquee`, `celula`, `navegar` e `digitar` (a página que o Lucas renderiza na fase 1: o cursor que voa até o `<button>` e o leque do clique, o spinner e o anel da `<img>` que carrega, a linha do `<marquee>`, cada célula da `<table>` que cai, o anel de quando o `<a href>` navega e cada letra digitada no `<input>`), `cartao`, `repintar` e `escorrer` (o DevTools da fase 2 dele: um cartão da grade, nascendo do professor, o tiro de cada cartão quando o navegador repinta e cada letra da regra que escorre embaixo dele enquanto ele a digita — a cascata do CSS), `worker`, `cast`, `handle_cast`, `crash` e `pipe` (a árvore de supervisão do Elixir da fase 3 dele: o worker que o supervisor inicia, a mensagem que corre pela linha até a caixa dele, o leque do worker que processa a caixa cheia, o anel de cada worker que a estratégia termina e cada etapa do `|>` que escorre embaixo do supervisor parado), `ficha`, `log`, `then`, `fetch` e `timeout` (o event loop da final dele: a ficha de cada linha do script, que anda entre a pilha, as Web APIs e as filas, e o ataque de cada coisa que roda na pilha — o `console.log`, o callback do `.then`, a resposta do `fetch` e o callback do `setTimeout`), `tarefa` (uma página de uma barra do cronograma da proposta do Hugo, que cai quando o prazo vence) e `questao` (cada tiro de um padrão de matéria do TCC: o tema do orientador, as perguntas da banca e as correções) `rascunho` (cada página que escorre embaixo do Hugo enquanto ele escreve um capítulo, e o rascunho de uma operação preparada por uma transação do Thiago, na cor dela, ou o risco da linha que o DELETE dele vai apagar) e `alvo` (o alvo do objetivo de cada etapa do TCC — a assinatura, a resposta A, B ou C, a correção —, que abre o escudo do Hugo, e também o que o aluno pega nos objetivos de andar: a referência, a versão final e a caixa da secretaria, que o Hugo marca como enfeite; o bug dos experimentos sai pelo `bug`), e os do Arroyo: `no`, `trabalho` e `folha` (a árvore de recursão da fase 1: o nó que se divide, o trabalho f(n) que ele paga e o tiro de cada folha), `moeda` (o troco guloso), `celula`, `chuva` e `casou` (a tabela da programação dinâmica: a casa calculada, a chuva dela e o leque da casa que casou letra com letra), `rainha`, `ataque` e `solucao` (o backtracking das seis rainhas: a rainha colocada, o tiro pela coluna dela e as oito direções da solução), `valor` (o valor que desce a rede de ordenação), `cidade`, `caixeiro`, `visita` e `rota` (o caixeiro viajante: a cidade, o caixeiro que corre a rota, o anel de cada cidade visitada e o de cada cidade quando a rota é a melhor até agora), `solucao`, `vale` e `congelou` (a têmpera simulada da final: cada solução do enxame, os ótimos locais e o anel de quem congelou), e os do Goulart: `processo`, `titulo` e `instrucao` (o escalonador da fase 1: o cartão do processo, o título de cada fila e a chuva de instruções na faixa de quem roda), `moldura`, `entrada`, `acesso`, `pagina` e `vitima` (a paginação: a moldura da memória física, a entrada da tabela de páginas no chão, o leque com que o professor lê a página do aluno, a página que chega do disco e os pedaços da vítima, que desabam na coluna dela), `requisicao` e `leitura` (o braço do disco: o pedido na trilha e o bloco lido, que cai na coluna do pedido), `vm`, `kernel`, `trap` e `emulacao` (a nuvem da final: o sistema convidado, os tiros do kernel presos na caixa dele, a instrução privilegiada que voa ao hipervisor e o leque de três que o hipervisor atira quando a emula), e os da Maria Lúcia refeita: `placa`, `queda`, `pedido`, `custo` e `estouro` (a cascata: as caixas, o que cai da caixa da etapa, a mudança de escopo, o custo que ela paga em cada caixa e o estouro na etapa da vez), `historia`, `cursor`, `titulo`, `trabalho`, `entrega` e `daily` (o Scrum: a história com os pontos, o cursor dela, o título de cada coluna, o tiro da história na FAZENDO, o incremento que estoura e as mensagens da daily), `inscrever`, `notificacao` e `consulta` (o Observer: o olho que se inscreve e orbita o aluno, e o aviso de todos quando ele muda de estado), `componente` e `dividida` (o Decorator: a bala, e a metade que a Dividida solta), `lobisomem` e `uivo` (o lobisomem de Brooks da final e o anel do uivo dele), além de `bala` e `estilhaco`; e os do Daniel, a optativa de Ciência de Dados: `fonte`, `dado` e `estouro` (a preparação da fase 1: o sensor, a planilha e a API, o dado que voa da fonte até o lugar dele no gráfico e o anel do outlier removido), `amostra`, `centroide` e `convergiu` (os grupos da fase 2: o ponto da nuvem da PCA e das k-médias, o centróide de cada grupo e o anel de quando ele chega à média nova), `rotulada` e `arvore` (a fronteira de decisão da fase 3: a rajada de uma classe, que passa pelo aluno da mesma classe, e cada árvore do comitê), `palpite`, `barra`, `previsao` e `caixa` (a explicabilidade da final: o palpite da caixa-preta, cada tiro de uma barra do SHAP, o anel da previsão onde a cascata termina e a caixa-preta por cima do professor), e os do sorteio de quem explica a aula passada, no começo das fases 2, 3 e da final: `resposta` (cada carta, derrubada pelo aluno para responder), `engano` (o anel da carta errada) e `garoa` (os dados que caem balançando enquanto a turma espera); e os do Michel, a optativa de visão computacional: `objeto`, `imagem` e `enfeite` (a câmera escura da fase 1: o objeto que ele arremessa na cena, a imagem dele do outro lado do furo, de cabeça para baixo — e a ponta do anel do desfoque —, e a câmera e o reflexo invertido do professor), `fluxo`, `ativacao`, `vetor` e `leque` (os descritores da fase 2: o tiro que anda pela seta do gradiente, o anel da célula que passou do limiar da convolução ou do pooling, o anel do vetor de features e o leque do professor), `neuronio`, `sinal` e `saida` (a rede da fase 3: o neurônio, que o aluno derruba para o dropout, o sinal que anda pelas ligações e pelos atalhos, e o leque da camada de saída), `pixel`, `previsao`, `colega`, `cerco` e `ruido` (o MaVIlab da final: o pixel da máscara da segmentação, a previsão da recorrente, os colegas do laboratório, o cerco e o rastreador do detector e a garoa do ruído); um nome desconhecido é recusado |
| `estrategia` | qual padrão: `CircleSpreadAttack`, `WaveAttack`, `ConsultaAttack`, `CaixaAttack`, `PontosAttack`, `TokenAttack`, `PalavraAttack` |
| `projetil` | o nome dentro do conjunto deste professor em `projeteis.json` |
| `projeteis` | quantos por disparo |
| `velocidade` | px/s |
| `angulo` | a **abertura** do leque, em graus |
| `anguloCentral` | o **meio** do leque. 0 = direita, 90 = baixo, 180 = esquerda |
| `intervalo` | atraso entre um projétil e o seguinte — só a `WaveAttack` e a `TokenAttack` leem |
| `cooldown` | segundos entre dois disparos |
| `dificil` | opcional. O **degrau do modo Difícil** deste ataque: `{ "cooldown": 1.25, "velocidade": 160, "projeteis": 18 }`, cada campo opcional, e o ausente fica como no Normal. Vale ao **montar** o ataque, e não a cada disparo: o `AttackParams` nasce com os números do modo, e o C++ que os lê vê o número certo. Serve aos ataques de relógio, e à quantidade de projéteis dos de gatilho (o Vladimir lê mais letras no Difícil); a velocidade de um de gatilho o professor muda na hora (`Boss::Disparar(gatilho, alvo, rapidez)`). Uma chave desconhecida é recusada |
| `variar` | opcional. O quanto **cada disparo muda** do anterior: `{ "giro": 12, "abertura": 0.2, "velocidade": 0.1, "projeteis": 1, "cadencia": 0.25 }`, cada campo opcional. Veja *A variação de cada disparo*, abaixo |
| `regras` | o nome de um conjunto de `regras.json`, ou a lista escrita ali mesmo |
| `som` | opcional, `true` por padrão. `false` cala o disparo: o fluxo contínuo (a chuva, os pedaços que caem um atrás do outro) e o enfeite não tocam o som do disparo a cada projétil — o professor marca o começo do fluxo com um som só (`Boss::SomDoDisparo`). Mesmo com som, o disparo passa pelo limite do grupo (`LimiteDeSom.h`): dois tocando juntos, no máximo, para a música ainda se ouvir |

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

**A variação de cada disparo.** Sem `variar`, todo disparo de um ataque é igual
ao arquivo: o mesmo leque, no mesmo ângulo, no mesmo ritmo — e uma fase que se
decora. Com ele, cada disparo sorteia um pouco de cada coisa:

```json
"variar": { "giro": 12, "abertura": 0.2, "velocidade": 0.1,
            "projeteis": 1, "cadencia": 0.25 }
```

| Campo | Cada disparo |
|---|---|
| `giro` | gira o leque (ou o anel) inteiro até tantos **graus** para cada lado. De 0 a 180 |
| `abertura` | abre ou fecha o leque nesta **fração** do `angulo`: 0.2 vai de 80% a 120%. De 0 a 0.9 |
| `velocidade` | a mesma fração, na velocidade. De 0 a 0.9 |
| `projeteis` | até tantos projéteis **a mais ou a menos**, nunca abaixo de um. Inteiro de 0 a 10 |
| `cadencia` | a fração do `cooldown` até o disparo seguinte: 0.25 espera de 75% a 125%. Só nos ataques do relógio — no de `gatilho`, quem decide quando sai é o professor. De 0 a 0.9 |

O sorteio é por igual para cada lado, e por isso **a média é o arquivo**: o
ataque não fica mais fácil nem mais difícil, só deixa de ser decorável. Ele sai
da **semente da batalha** (`BATALHA: semente N` no log, a coluna `semente` de
`tentativas.csv`): cada ataque de cada fase tem a sua sequência, e o clone a sua;
a mesma semente repete a luta inteira, que é o que a simulação e a linha de base
precisam. Ligar um campo não muda o que os outros sorteiam.

O giro vale **depois** do professor mirar (`CustomizeAttackParams`): o leque
mirado no aluno sai até `giro` graus torto, em volta dele. O anel da
`CircleSpreadAttack` não mira ninguém — sem `giro` ele sai sempre no mesmo
ângulo, e a fresta entre dois projéteis fica no mesmo lugar a luta inteira;
com `giro` de meio espaço (180 dividido pela quantidade: 15 num anel de 12),
cada anel sai num ângulo qualquer. Já o anel **mirado** (um caco no aluno,
`Coercao::CentroDoAnel`) perde a mira com qualquer giro: nele, varie a
`velocidade`. E o ritmo que é o tema da fase fica fixo — o clock do Ricardo, as
dailies da Maria Lúcia. Os valores fora
da faixa, de tipo errado ou de nome desconhecido não recusam a fase: saem no log
(`FASES em ...`) e o ataque continua, só sem aquela variação — e o teste
"o fases.json de cada chefe não tem problema nenhum" falha, para ninguém
publicar um `variar` que não vale.

**O que varia no código.** O `variar` é o disparo de um ataque. O que é do
professor — o ritmo de um roteiro, quem age primeiro, quanto cada coisa demora —
ele sorteia no próprio código, da mesma semente (`Boss::SorteioDaFase`). O Vitor,
por exemplo, é a matéria do não determinismo: na fase 1 cada thread manda as
instruções no seu ritmo, o escalonador decide quanto cada uma fica na seção
crítica e a corrida dá um saldo errado diferente a cada luta; na 2 o RTT varia e,
depois do slow start, o enlace às vezes perde um segmento no caminho (o ruído), que
estoura onde estava e que o TCP trata como congestionamento; na 3 cada envelope tem
a sua latência, e na assíncrona eles chegam fora de ordem; no jantar ninguém pensa
nem come o mesmo tempo duas vezes. As regras de cada fase continuam valendo em
qualquer sorteio — os testes de `Semaforo`, `Tcp`, `Mensagens` e `Filosofos`
conferem centenas deles.

### Blocos de estratégia

Algumas estratégias têm um bloco próprio, além dos campos comuns. Quando ele é
obrigatório e falta, a leitura recusa — um ataque que não dispara é mais difícil
de achar que um arquivo recusado.

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

**`caixa`**, opcional na `CaixaAttack`:

```json
"caixa": { "proporcao": 1.6, "meiaAlturaInicial": 72 }
```

| Campo | |
|---|---|
| `proporcao` | largura dividida pela altura. Padrão 1,6 |
| `meiaAlturaInicial` | a meia-altura com que a caixa nasce, em pixels. Padrão 72, um pouco maior que a metade do cartão do professor |

A `CaixaAttack` é a div do Lucas — hoje, o callback de um `setTimeout` no event loop da final dele: um retângulo de `projeteis` ladrilhos, igualmente
espaçados pelo **contorno** (e não pelo ângulo, o que deixaria buraco nos lados
curtos), que nasce em volta do professor e cresce sem deformar. `velocidade` é a
da meia-altura; os cantos andam mais rápido. Os ladrilhos se afastam enquanto a
caixa cresce, então ela nasce fechada e chega ao aluno com frestas. `angulo` e
`anguloCentral` são ignorados. Zero ou negativo nos dois campos é recusado.

**`tokens`**, opcional na `TokenAttack`:

```json
"tokens": { "temperatura": 0.8, "candidatos": 9, "abertura": 120,
            "frase": ["o", "mo", "de", "lo", "te", "pre", "diz"] }
```

| Campo | |
|---|---|
| `temperatura` | a do sorteio de cada token. Zero = sempre o mais provável, o tiro mirado. Padrão 0 |
| `candidatos` | quantos ângulos candidatos, um inteiro de 1 para cima. Padrão 9 |
| `abertura` | o leque dos candidatos, em graus, centrado em `anguloCentral`. Padrão 120 |
| `frase` | as animações dos tokens, na ordem em que saem. Sem ela, não mexe |

A `TokenAttack` é a geração de texto do Júlio: cada disparo é uma **frase** de
`projeteis` tokens que saem um depois do outro (`intervalo`, como na
`WaveAttack`), e cada token sai num ângulo **sorteado** entre os candidatos
com probabilidade softmax(logit / `temperatura`) — o logit é maior quanto mais
perto da mira (`Temperatura.h`). Fria, a frase chega inteira em fila; quente,
ela se espalha. `temperatura` e `abertura` negativas são recusadas, e um teste
confere que toda palavra da `frase` é uma animação do projétil.

**`palavra`**, opcional na `PalavraAttack`:

```json
"palavra": { "altura": 390, "zero": "Zero", "um": "Um", "atravessa": "zeros",
             "valor": "0000 1111 1111 0000" }
```

| Campo | |
|---|---|
| `altura` | o y da fileira. Ausente = a altura do professor no disparo |
| `zero` / `um` | as animações do bit 0 e do bit 1. Padrão `Zero` e `Um` |
| `atravessa` | `zeros` (padrão): o zero é enfeite e se atravessa, o um machuca. `todos`: a fileira inteira é enfeite — a fileira de portas |
| `valor` | os bits, escritos como se leem: 0 e 1, com espaços para agrupar, **um dígito por projétil**. Ausente = zero, ou o que o professor puser no disparo |

A `PalavraAttack` é a palavra de bits do Ricardo: uma fileira de `projeteis`
bits de uma borda da tela à outra, um por coluna (a tela dividida em colunas
iguais), descendo juntos a `velocidade`. A **coluna 0 é o bit mais alto**, à
esquerda — a palavra se lê na tela como se escreve. `angulo` e `anguloCentral`
são ignorados: ela sempre desce reta. Na fase 1 do Ricardo o valor vem dele, na
hora do disparo: uma palavra sorteada para a porta que ela vai atravessar
(`Logica.h`). Um `valor` com dígitos de mais ou de menos, ou com algo que não é
bit, é recusado.

**`pontos`**, obrigatório na `PontosAttack`:

```json
"pontos": { "posto": "quadro", "mirar": true, "prever": true }
```

| Campo | |
|---|---|
| `posto` | de onde atirar: `quadro` (reservado: hoje ninguém o ocupa), `observadores` (os olhos do Observer da Maria Lúcia), `agentes` (os robôs do Júlio), `nuvens` (as nuvens do Vitor), `palavra` ou `portas` (os bits do Ricardo que ainda não passaram pela porta, e as portas), `registradores` (os bits que andam no clock dele), `estagios` ou `pipeline` (as molduras e as instruções do pipeline dele), `placas` ou `acesas` (os Arduinos do Blink dele, apagados e acesos), `fita` (as casas da fita de Turing do Vladimir, que o intercâmbio não troca), `estados` (o diagrama do autômato dele), `pilha` (os blocos da pilha do Salles), `baldes` e `chaves` (a tabela hash dele), `arvore` (os nós da árvore de busca dele, e os da árvore sintática do compilador do Vladimir), `vetor` (os elementos do bubble sort da repescagem dele), `pagina` (o cursor e o spinner da página da fase 1 do Lucas), `grade` (os cartões do DevTools da fase 2 dele), `supervisao` (os workers e as mensagens da árvore de supervisão da fase 3 dele), `eventloop` (as fichas do event loop da final dele), `cronograma` (as páginas das barras do cronograma da proposta do Hugo), `objetivo` (os alvos que abrem o escudo dele, e o que se pega nos objetivos de andar), `tabela` (os cabeçalhos e os valores da tabela do Thiago), `aviso` (o marcador da casa que a consulta dele vai ler) — todos menos o primeiro, a regra `posto` põe lá ao nascer. Um posto desconhecido é recusado — ninguém o ocuparia, e o ataque nunca dispararia |
| `mirar` | cada ponto mira o aluno. Sem ele, todos saem em `anguloCentral` |
| `prever` | mirando, mira onde o aluno **vai estar** se continuar andando |

A `PontosAttack` não sai do professor: sai de **cada ponto** do posto, um leque
de `projeteis` com abertura `angulo` em cada um. Quem diz onde estão os pontos
é o professor, na hora do disparo (`Boss::PontosDoPosto`); um posto vazio não
dispara nada, e esse disparo vazio não passa a vez nem faz barulho.

---

## `<professor>/regras.json`

**Uma pasta por professor**: `Assets/Attacks/salles/regras.json`. Cada chave é um
conjunto, e é por esse nome que um ataque do `fases.json` ao lado o pede.

> **Os nomes perderam o prefixo do professor.** Eram `salles_lista_simples`
> porque um arquivo único com todos os conjuntos precisava evitar colisão; com
> uma pasta por professor o prefixo passou a repetir o nome da pasta.

Um conjunto é uma lista de regras aplicadas a **cada** projétil da rajada, na
ordem em que aparecem.

```json
"letra_b": [
  { "animacao": "B",
    "motion": { "tipo": "Tracking", "atraso": 0.3, "forca": 1.4, "duracao": 2.4 } }
],

"fase3": [
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
| `"quando": "alvoAcorrentado"` | **só enquanto um aluno estiver preso** por uma corrente |

> **`alvoAcorrentado` é a única condição que olha para o estado do jogo**, e não
> para a posição na rajada. Por isso ela entra em `RegraSeAplica` como um `bool`
> em vez de uma consulta: a camada pura não conhece `Player` nem `Scene` e não
> vai passar a conhecer — quem pergunta ao mundo é o configurator em
> `RegrasDeAtaqueArquivo.cpp`, que já é a ponte impura.
>
> Em dupla, **qualquer um dos dois preso já conta**. É a mesma escolha da nota
> compartilhada, e evita metade da rajada perseguir um aluno e metade o outro
> dentro do mesmo disparo.

### O que a regra faz

| | |
|---|---|
| `motion` | **uma** por projétil. Decide a direção e **preserva** o módulo. Uma segunda substitui a primeira. |
| `modifiers` | quantas quiser. Mudam o módulo ou agendam coisas, e acumulam. |
| `animacao` | nome de uma animação registrada no projétil |
| `escala` | multiplica o tamanho — e a hitbox junto, porque o colisor multiplica o raio pela escala |
| `rotacionar` | `true`/`false`: se o sprite gira para acompanhar a direção do voo. **Ausente = não mexe** e vale o que `projeteis.json` pediu |
| `posto` | o **posto** em que o projétil entra ao nascer: um lugar de onde uma `PontosAttack` atira, ou onde o professor o procura. Existem `quadro`, `observadores`, `agentes`, `nuvens`, `palavra`, `portas`, `registradores`, `estagios`, `pipeline`, `placas`, `acesas`, `fita`, `estados`, `derivacao`, `pilha`, `baldes`, `chaves`, `arvore`, `vetor`, `pagina` (o cursor e o spinner da página da fase 1 do Lucas), `grade` (os cartões do DevTools da fase 2 dele), `supervisao` (os workers e as mensagens da árvore de supervisão da fase 3 dele), `eventloop` (as fichas do event loop da final dele), `cronograma` (as páginas das barras do cronograma da proposta do Hugo), `objetivo` (os alvos que abrem o escudo dele), `tabela` e `aviso`; um nome desconhecido é recusado. Volta a vazio quando o pool recicla o projétil |

> **`rotacionar` existe porque girar é da rajada, e não do tipo.** Nasceu
> com o `Estruturas` do Salles antigo, que servia a duas fases com
> necessidades opostas: a lista da fase 1 era uma caixa com **seta**, que sem
> girar aponta sempre para a direita e não se lê como lista encadeada; o nó de
> árvore da fase 3 tinha dois filhos embaixo e era **simétrico**, então girado
> aparecia deitado. O tipo saiu com as listas, e o campo ficou. Como
> `escala` e `animacao`, é aparência: volta ao padrão de fábrica quando o
> projétil é reciclado pelo pool.

> **Numa `WaveAttack`, a `duracao` de um motion corre enquanto o projétil
> dorme.** A `WaveAttack` cria a rajada inteira adormecida e acorda um projétil
> a cada `intervalo`, mas o relógio do behavior começa no nascimento. Os dois
> casos se parecem e falham diferente:
>
> - o `Wobble` **morre**: ao ativar num projétil parado ele não acha direção
>   para oscilar em torno e se declara terminado. Precisa de um `atraso` maior
>   que o despertar do último;
> - o `Tracking` **sobrevive** (ele devolve sem fazer nada enquanto a velocidade
>   é zero), mas a `duracao` dele já está correndo — numa fila longa a janela
>   pode expirar antes de o último acordar. Precisa de `duracao` folgada.
>
> Em ambos, mexer em `projeteis` ou `intervalo` obriga a revisar os tempos.

> **`CircleSpreadAttack` IGNORA `angulo`.** O passo dela é sempre `360/n`
> (`CircleSpreadAttack.cpp`), e o campo é aceito pelo leitor sem ser usado pela
> estratégia — não há aviso. Quem quiser mudar a densidade do anel mexe em
> `projeteis`.
>
> E as frestas do anel ficam **sempre nas mesmas direções**, porque os ângulos
> saem de `i * 360/n` a partir de zero. Com o chefe parado, um alvo parado numa
> fresta fica nela para sempre: foi assim que a fase 3 do Vladimir passou a dar
> zero acerto em dezessete segundos, e subir `projeteis` de 10 para 18 não
> mudou nada. O que resolveu foi o chefe **andar**.

**Motions:** `Path`, `Tracking`, `Wobble`, `MiraPeriodica`, `Amarrar`, `Ancorar`
**Modifiers:** `Accelerate`, `SlowDown`, `Activate`, `Deactivate`, `PulsoDeVelocidade`, `Promessa`

Pôr um modifier em `motion` é recusado na leitura. No C++ quem garante isso é o
compilador; aqui é a leitura, porque arquivo de texto não passa por compilador.

### Campos de cada tipo

| Tipo | Campos |
|---|---|
| `Path` | `forma`, `formaPorIndice`, `mira`, `parametroDaMira`, `velocidade`, `atraso`, `pararNoPonto`, `pararPor` |
| `Tracking` | `forca`, `duracao`, `atraso` |
| `Wobble` | `amplitude` (graus), `frequencia`, `duracao`, `atraso` |
| `MiraPeriodica` | `ritmo` |
| `Amarrar` | `comprimento`, `duracao`, `sentido` (`horario` ou `antihorario`, padrão horário), `escreve` (opcional) |
| `Ancorar` | `dx`, `dy` (o lugar em relação ao professor), `duracao`, `velocidade` (a da queda) |
| `Accelerate` / `SlowDown` | `fator`, `atraso` |
| `Activate` / `Deactivate` | `atraso` |
| `PulsoDeVelocidade` | `ritmo`, `moduloInvestida`, `moduloPausa` |
| `Promessa` | `atraso` (quando assenta), `resolve` (a parte que resolve, de 0 a 1), `velocidade`, `escreve` (opcional) |

`mira` é `AlinharComVelocidade` (o padrão), `MirarNoJogador` ou `MirarPrevendo`.

**`Amarrar` é a corda dos guardrails do Júlio** — nasceu na primeira fase 1 do Luizinho (*ligāre*, "atar"), que continua sendo a das amarrações. A letra
nasce presa ao professor por uma corda que não se vê: a corda desenrola enquanto
ela se afasta dele, estica em `comprimento` pixels, e dali a letra **gira em
volta do professor e vai junto** para onde ele for — o nó é procurado a cada
quadro, e é isso que faz a amarração ser dinâmica. Passados `duracao` segundos
esticada, a corda é cortada e a letra sai pela **tangente**, como a pedra de uma
funda. `escreve` é a animação que ela ganha quando a corda estica. A corda nasce
no professor, então a letra tem de ser disparada de cima dele (uma
`CircleSpreadAttack`, por exemplo); `comprimento` e `duracao` zero e um
`sentido` que não seja um dos dois são recusados na leitura. A conta de cada
quadro é pura e mora em `Amarra.h`.

**`Ancorar` é a janela modal da fase final da Maria Lúcia.** O projétil vai
para o lugar `(dx, dy)` em relação ao corpo do professor mais perto e fica
**preso a ele** enquanto ele anda, como um diálogo centrado na janela que o
abriu, por `duracao` segundos; depois solta e cai reto a `velocidade`. Ancorada,
a velocidade dela é zero: é um anteparo, e não um tiro. Para cada janela da
leva ter o seu lugar, a regra usa `indices`. `duracao` e `velocidade` zero são
recusados. O passo da aproximação é puro e mora em `Ancora.h`.

**`Promessa` é a Promise do JavaScript.** Aos `atraso` segundos o projétil
assenta: uma parte `resolve` deles sai em direção ao alvo a `velocidade` (e troca
para a animação `escreve`, se houver), e o resto é rejeitado e some. Ela não
para sozinha — quem a deixa pendente é um `SlowDown` com `fator` zero antes
dela, na mesma regra. `velocidade` zero e `resolve` fora de 0 a 1 são recusados.

### Dois exemplos para copiar

Estes dois conjuntos viviam no `regras.json` como `exemplo_teleguiado` e
`exemplo_cacador`, sem nenhum professor usá-los. Com uma pasta por professor eles
deixaram de ter casa — e o lugar deles sempre foi aqui.

```json
"teleguiado": [
  { "chance": 0.2,
    "motion": { "tipo": "Path", "forma": "Reta", "velocidade": 280,
                "atraso": 1.2, "mira": "MirarNoJogador" },
    "animacao": "Homing" }
]
```

A **investida repetida**: o projétil quase para, re-aponta, e dá um mergulho
rápido. Precisa das **duas** peças derivadas do **mesmo** ritmo — a motion que
gira e o modifier que pulsa o módulo. Repare que o ritmo aparece duas vezes:
mudar um e esquecer o outro faz a mirada acontecer fora do mergulho.

```json
"cacador": [
  { "motion": { "tipo": "MiraPeriodica",
                "ritmo": { "investida": 0.2, "pausa": 1.0, "repeticoes": 6 } },
    "modifiers": [
      { "tipo": "PulsoDeVelocidade",
        "ritmo": { "investida": 0.2, "pausa": 1.0, "repeticoes": 6 },
        "investida": 640, "pausa": 15 }
    ] }
]
```

### O que costuma dar errado

> **`velocidade` num `Path` é a do caminho, não a do disparo.** A 340–400 px/s um
> laço vira um risco na tela, e por isso quase todo caminho usa menos que o
> ataque.

> **Motion captura a direção no instante em que ativa.** Se o projétil estiver
> parado nesse momento — por um `Deactivate` ou por ter nascido assim — não há
> direção para capturar e ele sai para a direita. Foi isso que obrigou o
> `"atraso"` da `lista_dupla` do Salles antigo a ser maior que o despertar do
> último nó da fila: numa `WaveAttack` com caminho, mexer em `projeteis` ou
> `intervalo` **obriga** a revisar esse atraso — e um teste confere a conta
> (hoje, no `diagrama` da Maria Lúcia).

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

## `<professor>/projeteis.json`

**Uma pasta por professor**: `Assets/Attacks/thiago/projeteis.json`. A pasta diz
de quem é o arquivo, então a raiz dele já é o mapa de projéteis — o nome de cada
entrada é como `fases.json` vai pedi-lo.

```json
{
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

> **Era um `projeteis.json` só, com um nível a mais dizendo o nome do professor.**
> Com uma pasta por professor esse nível virava uma segunda cópia do nome, livre
> para discordar da pasta. Quem monta o caminho é `PastaDoChefe::Arquivo`, num
> lugar só — e montar um professor deixou de abrir os arquivos dos outros quatro.

| Campo | |
|---|---|
| `sprite` / `dados` | caminhos **sem** `Assets/` na frente |
| `animacoes` | nome → lista de índices de quadro |
| `animacaoInicial` | com qual começar |
| `escala` | multiplica o desenho **e a hitbox** |
| `ordemDeDesenho` | menor desenha antes, ou seja, atrás |
| `posicionarNoDono` | se nasce em cima do professor |
| `margemEmSprites` / `margemDivisorDeTela` | quão longe da tela ele morre |
| `rotacionarComAVelocidade` | se o sprite gira para acompanhar o voo (uma regra pode desligar por rajada — ver `rotacionar`) |
| `derrubavel` | `true` se o tiro do aluno derruba este projétil. Os dois somem, o tiro não chega ao professor, e o professor é avisado (`Boss::OnProjetilDerrubado`). Todo projétil que o tiro derruba ganha um **contorno vermelho** na forma do sprite, o mesmo para todo professor (`Destrutivel.h`) — menos quando está blindado, é abrigo ou é enfeite, os casos em que o tiro não o destrói. Padrão `false`; usam o cursor e os workers do Lucas e os bugs, as histórias e as janelas da Maria Lúcia |
| `resistencia` | quantos tiros derrubam um projétil `derrubavel`; os anteriores **somem nele**, sem derrubá-lo e sem chegar ao professor. Padrão 1. Num projétil que não é `derrubavel`, a leitura avisa e ignora |
| `dificil` | opcional. `{ "resistencia": 3 }`: a resistência no modo Difícil, e só ela. Os blocos da pilha do Salles caem com 2 tiros no Normal e 3 no Difícil. As mesmas regras da `resistencia` |
| `acorrenta` | o que acontece com o aluno ao ser acertado: `segundos`, `fatorDeVelocidade`, `multiplicadorDaPerda` |
| `abrigo` | `true` se este projétil **abriga** o aluno: todo projétil solto do professor que encosta nele some (um que está num `posto` é parte de uma estrutura e fica), ele não machuca ninguém, e o tiro do aluno some nele também. Padrão `false`. É a capivara que atravessa o campo do Salles (`Capivara.h`) |

```jsonc
"Corrente": {
  "...": "...",
  "acorrenta": {
    "segundos": 3.5,              // quanto tempo o aluno fica preso
    "fatorDeVelocidade": 0.45,    // entre 0 (exclusivo) e 1
    "multiplicadorDaPerda": 0.15  // quanto da perda normal este acerto custa
  }
}
```

> **`acorrenta` é a primeira troca "quase nenhuma nota por bastante estorvo".**
> São as correntes do Vitor — a piada de *concorrentes*. Se ela custasse os 6
> pontos de um tiro qualquer, seria um tiro qualquer que por acaso também
> prende, e o jogador leria "tomei dano" em vez de "fiquei preso". **Mas ela
> conta como acerto sofrido**, para o direito à nota cheia e para a fase limpa:
> ser acorrentado não é de graça, é só barato em nota.
>
> Um `fatorDeVelocidade` fora de `(0, 1]` ou um `segundos` que não seja maior
> que zero **derrubam o efeito inteiro** e viram problema na leitura — aceitar
> calado daria um aluno que acelera ao ser acertado, ou um projétil que diz
> prender e não prende.

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
  "fixas": [],
  "formatura": { "materia": "TCC", "arte": "Player/Camadas/capelo", "ordem": 70 },
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
| `formatura` | opcional. A camada de quem passou na `materia` (o código dela em `materias.json`): `arte` e `ordem`, em cor final, como as fixas. Sem `materia` ou sem `arte`, é ignorada com aviso |
| `categorias` | o que o jogador escolhe: uma peça e uma cor por categoria |
| `ordem` | a ordem de empilhamento; maior fica por cima |
| `cores` | a paleta daquela categoria, com `rgb` em hexadecimal sem `#` |
| `pecas` | as formas daquela categoria, cada uma com a sua `arte` |
| `predefinidas` | combinações prontas, usadas como ponto de partida |

As ordens hoje são pele 10, sapato 30, calça 40, camisa 50, mochila 55, cabelo
60 e o capelo da formatura, 70, por cima de tudo — e os buracos entre elas são
de propósito, para caber camada nova no meio
sem renumerar as outras (a mochila entrou assim, entre a camisa e o cabelo: o
cabelo comprido cai por cima dela). A **ordem da lista** de categorias é outra
coisa: é a ordem das linhas na tela de criação.

O sapato já foi uma camada fixa, em cor final; virou categoria, e o primeiro
item de cada categoria é o padrão de quem não escolheu — o tênis grafite é o
sapato de antes, e a primeira mochila é "nenhuma". Quem tem uma ficha de antes de
uma categoria existir cai nesse padrão, e por isso a personagem não muda sozinha.
A arte das peças sai de `Ferramentas/gerar_camadas.py`.

> **A ficha guarda o `id` da peça, nunca a posição dela na lista.** Acrescentar
> uma peça no meio de uma categoria não troca o cabelo de ninguém.

> **A formatura não fica na ficha.** Quem se formou é quem tem a matéria da
> `formatura` aprovada — o recorde dela, em qualquer modo e nível, chegou a 60 —,
> e o recorde só sobe: por isso o capelo é para sempre sem campo novo na ficha.
> A mesma aparência com e sem capelo são duas texturas no cache
> (`ChaveDaAparencia`).

---

## Onde cada um é lido

Todo arquivo tem um par: uma função **pura** que recebe o texto e devolve dados
mais uma lista de problemas, e uma **ponte** que lê o disco e monta os objetos do
motor.

| Arquivo | Camada pura | Ponte |
|---|---|---|
| `materias.json` | `Materias::LerMaterias` | `MateriasArquivo.cpp` |
| `<professor>/fases.json` | `LerFases` | `FasesDeAtaqueArquivo.cpp` |
| `<professor>/regras.json` | `LerRegras` | `RegrasDeAtaqueArquivo.cpp` |
| `<professor>/projeteis.json` | `LerProjeteis` | `ProjeteisDeChefeArquivo.cpp` |
| `personagens.json` | `Personagens::LerCatalogo` | `PersonagensArquivo.cpp` |

É essa divisão que permite testar a leitura sem SDL e sem disco: os testes
entregam o texto do JSON direto para a função pura.
