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
| `chefe` | o nome da fábrica, como registrado em `Game::InitializeBossFactory`. Pode faltar: a matéria aparece e espera uma batalha. |
| `orientacao` | `true` se a batalha pede um **orientador** antes de começar — o TCC. Entrar nela pela seleção de fases abre antes a tela de escolha: os três professores das maiores notas aprovadas (em dupla, a maior nota dos dois em cada matéria), e em dupla os dois votam. O escolhido entra na batalha voando ao lado do aluno, invulnerável, e os tiros dele pagam a nota como os do aluno. Cada um ataca com a matéria dele (`Golpe.h`), todos com o mesmo peso: o `for` do André, a pilha do Salles (LIFO), o barramento do Ricardo, a transação do Thiago (tudo sai no COMMIT), o ponteiro do Luizinho, a gramática `S → S S` do Vladimir, a descida do gradiente do Júlio, o Observer da Maria Lúcia (atira a cada quatro tiros do aluno), o `fetch` do Lucas e a janela do TCP do Vitor — e o tiro dele tem a cara da matéria. E cada um traz um **poder** (`Poder.h`), que carrega a cada acerto no chefe e sai no botão 3: o laço sem fim do André, a capivara do Salles, o `HLT` do Ricardo (que congela o chefe e os tiros dele), o `ROLLBACK` do Thiago, o `free()` do Luizinho, o autômato do Vladimir, os agentes do Júlio (os tiros caçam o alvo), a bala de prata da Maria Lúcia (que quebra o escudo do Hugo), o `Promise.all()` do Lucas (doze promessas que vão até o Hugo e voltam, desfazendo os tiros dele pelo caminho) e o `socket()` do Vitor (as paredes dos lados viram rede). Opcional, falso por padrão; um valor que não é booleano vira aviso. |
| `desbloqueio` | quando ela abre |

**Tipos de desbloqueio:**

- `"sempre"` — sempre aberta
- `"aprovadoEm"` com `"materias": [...]` — abre quando todas elas estiverem aprovadas
- `"aprovadasNaColuna"` com `"coluna"` e `"quantas"` — abre com N aprovações naquela coluna

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

> **Quem pode orientar** (`Source/Orientacao.h`): o professor de uma matéria em
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
| `movimento` | como o professor anda |
| `ataques` | lista; todos disparam na mesma fase, cada um no seu cooldown |

> **O placar escreve na Carlito, e não na Zelda.** A Zelda não tem `*` nem `^`,
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
> ao fim da terceira chamaria `FinishBattle` por conta própria e encerraria a
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
| `gatilho` | opcional. O ataque **não dispara pelo relógio**: dispara quando o professor puxa o gatilho (`Boss::Disparar`), e o `cooldown` é ignorado. Só existem os gatilhos que algum professor puxa — `notificacao` e `consulta` (o padrão Observer da Maria Lúcia), `agir` (os agentes do Júlio), `token` (cada token da fase 1 dele, que nasce na ponta da frase gerada), `nuvens` (as nuvens do Vitor), `portas` (a fileira de portas lógicas do Ricardo), `contador` (o leque do clock dele, a cada segundo), `estagios`, `buscar`, `lw`, `sw`, `add` e `addi` (o pipeline dele: as molduras, a busca e o ataque de cada instrução no seu estágio), `acender` (o LED das placas do Blink dele), `estados`, `diagrama`, `ler_a`, `ler_b`, `aceita` e `rejeita` (o autômato da fase 1 do Vladimir: o mapa, a letra lida e o veredito), `lexema`, `arvore`, `reduzir`, `emitir` e `store` (o compilador da fase 2 dele: o token que cai no léxico — o `a` e o `b` caem por `ler_a` e `ler_b` —, o nó que voa até a árvore sintática, o leque de cada operador reduzido, a instrução que cada nó vira na geração de código e o anel do STORE), `escrever`, `apagar`, `reescrever`, `reler` e `pare` (o castor mais ocupado da fase 3 dele, uma máquina de Turing de verdade: a marca escrita na casa, a marca apagada que cai reta, a apagada que no Difícil vira um "b" que persegue, a cópia da marca relida e o anel de quando a máquina para), `metade`, `triplo` e `parou` (o 3x + 1 da repescagem dele), `capivara` (a que atravessa o campo do Salles em qualquer fase), `push` e `estourar` (a pilha da fase 1 dele: o bloco empilhado, e o desempilhado que estoura em lascas), `baldes`, `chave`, `percorrer` e `nulo` (a tabela hash da fase 2: os baldes, a chave inserida, cada nó lido na busca e o NULL do fim da lista), `no`, `visitar`, `aresta`, `folha_nula` e `achado` (a árvore de busca da fase 3: a chave que desce e vira nó, cada nó por onde ela passa, os pontos da aresta, a busca que não acha e a que acha), `elemento`, `trocar` e `assentar` (o bubble sort da repescagem dele: o elemento do vetor, a capivara de cada troca e o anel de quem chega ao lugar), `cabecalho` e `registro` (a tabela escrita no campo do Thiago: o cabeçalho de cada coluna e o valor de cada casa), `marcar` (o marcador da casa que a consulta vai ler), `selecionar`, `projetar` e `celula` (as três formas do SELECT dele: a linha do WHERE, a coluna projetada e a casa do cruzamento), `gravar` (a escrita de uma transação dele, que vai até a linha, fica e volta no ROLLBACK), `desfazer` (a varredura do ROLLBACK, do outro lado da linha), `placa`, `pedido`, `classe`, `mensagem`, `compilar`, `bug` e `debito` (a Cascata da Maria Lúcia: a caixa de cada etapa, o post-it do cliente, a classe que ele vira no Projeto, as mensagens dela pelas setas, o código, o bug de cada classe e a dívida de cada bug que sobrou), `bala` e `estilhaco` (a repescagem dela, "não há bala de prata": a bala com o nome de uma promessa, e o leque de bugs em que ela se desfaz), `rotulo`, `balao` e `confete` (o fluxograma da fase 1 do André: o cabeçalho de cada ramo, o balão numerado que a condição manda para um ramo e o confete dele estourado), `indice`, `subir` e `core` (o vetor no chão da fase 2 dele: o índice de cada casa, o cacho de balões que sobe da casa por onde o `i` passa e a memória despejada pelo segfault), `cartao`, `valor` e `leque` (as funções da fase 3 dele: o cartão com a assinatura de cada função, o valor que vai como argumento e volta como retorno, e o leque de ponto-e-vírgulas), `trofeu` (o balão de cada problema da prova no BOCA, que sobe no YES) `gota` (a chuva da variável na fase 1 do Luizinho, o escopo: nasce apagada na região em que o nome é visível, acende e cai), `operador`, `operando`, `anel`, `texto`, `erro` e `compilacao` (a coerção da fase 2 dele: o operador em cima do aluno, os dois operandos que voam até ele, o anel do resultado número, a palavra do resultado texto, os estilhaços do TypeError onde a conta roda e os do erro de compilação no professor), `celula`, `pendente` e `estouro` (o heap da fase 3 dele: cada int de um bloco alocado, os cacos do ponteiro pendente e os de cada célula vazada no `std::bad_alloc`), `excecao`, `destroco` e `finally` (a pilha de chamadas da final dele: a exceção que sobe, a cortina de cada faixa desempilhada e o anel do `finally`), `instrucao` (a instrução de uma thread na fase 1 do Vitor, os semáforos: desce até a seção crítica, e sem o semáforo para na borda e se empilha), `segmento`, `encaminhar` e `descarte` (a fase 2 dele, o TCP: o segmento que voa da nuvem ao roteador, o que o roteador encaminha mirado no aluno e o anel de cacos do que ele descarta), `envelope`, `processar` e `encontro` (a troca de mensagens da fase 3 dele: o envelope que corre pelo canal entre o cliente e o servidor, a mensagem processada que vai para o aluno e o anel do rendezvous), `garfo`, `garfada` e `espera` (o jantar dos filósofos da final dele: o garfo que anda de mão em mão, o leque de quem come e o cerco do deadlock), `cursor`, `clique`, `spinner`, `carregar`, `marquee`, `celula`, `navegar` e `digitar` (a página que o Lucas renderiza na fase 1: o cursor que voa até o `<button>` e o leque do clique, o spinner e o anel da `<img>` que carrega, a linha do `<marquee>`, cada célula da `<table>` que cai, o anel de quando o `<a href>` navega e cada letra digitada no `<input>`), `cartao`, `repintar` e `escorrer` (o DevTools da fase 2 dele: um cartão da grade, nascendo do professor, o tiro de cada cartão quando o navegador repinta e cada letra da regra que escorre embaixo dele enquanto ele a digita — a cascata do CSS), `worker`, `cast`, `handle_cast`, `crash` e `pipe` (a árvore de supervisão do Elixir da fase 3 dele: o worker que o supervisor inicia, a mensagem que corre pela linha até a caixa dele, o leque do worker que processa a caixa cheia, o anel de cada worker que a estratégia termina e cada etapa do `|>` que escorre embaixo do supervisor parado), `ficha`, `log`, `then`, `fetch` e `timeout` (o event loop da final dele: a ficha de cada linha do script, que anda entre a pilha, as Web APIs e as filas, e o ataque de cada coisa que roda na pilha — o `console.log`, o callback do `.then`, a resposta do `fetch` e o callback do `setTimeout`), `tarefa` (uma página de uma barra do cronograma da proposta do Hugo, que cai quando o prazo vence) e `questao` (cada tiro de um padrão de matéria do TCC: o tema do orientador, as perguntas da banca e as correções) `rascunho` (cada página que escorre embaixo do Hugo enquanto ele escreve um capítulo) e `alvo` (o alvo do objetivo de cada etapa do TCC — a assinatura, a resposta A, B ou C, a correção —, que abre o escudo do Hugo, e também o que o aluno pega nos objetivos de andar: a referência, a versão final e a caixa da secretaria, que o Hugo marca como enfeite; o bug dos experimentos sai pelo `bug`, o mesmo nome do da Maria Lúcia); um nome desconhecido é recusado |
| `estrategia` | qual padrão: `AngledAttack`, `CircleSpreadAttack`, `WaveAttack`, `BaloonAttack`, `LaserAttack`, `ConsultaAttack`, `CaixaAttack`, `SprintAttack`, `PontosAttack`, `TokenAttack`, `PalavraAttack` |
| `projetil` | o nome dentro do conjunto deste professor em `projeteis.json` |
| `projeteis` | quantos por disparo |
| `velocidade` | px/s |
| `angulo` | a **abertura** do leque, em graus |
| `anguloCentral` | o **meio** do leque. 0 = direita, 90 = baixo, 180 = esquerda |
| `intervalo` | atraso entre um projétil e o seguinte — só a `WaveAttack` e a `TokenAttack` leem |
| `cooldown` | segundos entre dois disparos |
| `dificil` | opcional. O **degrau do modo Difícil** deste ataque: `{ "cooldown": 1.25, "velocidade": 160, "projeteis": 18 }`, cada campo opcional, e o ausente fica como no Normal. Vale ao **montar** o ataque, e não a cada disparo: o `AttackParams` nasce com os números do modo, e o C++ que os lê vê o número certo. Serve aos ataques de relógio, e à quantidade de projéteis dos de gatilho (o Vladimir lê mais letras no Difícil); a velocidade de um de gatilho o professor muda na hora (`Boss::Disparar(gatilho, alvo, rapidez)`). Uma chave desconhecida é recusada |
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

**`sprint`**, opcional na `SprintAttack`:

```json
"sprint": { "revisao": 2.5, "historia": "Historia", "cursor": "Cursor", "clique": "Clique" }
```

| Campo | |
|---|---|
| `revisao` | quantos segundos depois do disparo vem a revisão. Padrão 2,5; zero ou menos é recusado |
| `historia` | a animação das histórias |
| `cursor` / `clique` | as animações do cursor apontando e clicando |

A `SprintAttack` é a sprint dos métodos ágeis da Maria Lúcia: um **cursor** de
mouse pega a pilha de `projeteis` histórias no professor e a arrasta até o
quadro (a lei de Fitts diz quanto ele leva; `Sprint.h`), onde elas se espalham
pelas vagas e esperam. O tipo do projétil deve ser `derrubavel`: derrubar uma
história é **entregá-la**. Na mão do cursor a pilha é blindada (o tiro some
nela), parada na vaga a história entra no **posto** `quadro`, e na revisão o
que sobrou cai em cima do aluno a `velocidade` px/s, mirando onde ele vai estar.
O cursor sai da mesma fábrica, para usar a mesma folha, mas é um **enfeite**:
não acerta o aluno e o tiro passa por ele. `angulo` e `anguloCentral` são
ignorados, e uma sprint não leva regra com `motion`: as regras rodam depois da estratégia, e a `motion` dela substituiria a da história (um teste em `test_arquivos_de_dados.cpp` recusa).

> **Uma sprint por etapa.** Um teste confere que cada `SprintAttack` dispara
> uma vez só na etapa dela (`cooldown` que cobre a etapa), que a revisão cai
> antes de a etapa acabar e pelo menos 1 s antes do fim da fase, e que o número
> no anúncio da etapa ("Sprint 4 · 5 pontos") é o número de histórias.

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
| `posto` | de onde atirar: `quadro` (as histórias paradas no quadro da sprint), `observadores` (os olhos do Observer da Maria Lúcia), `agentes` (os robôs do Júlio), `nuvens` (as nuvens do Vitor), `palavra` ou `portas` (os bits do Ricardo que ainda não passaram pela porta, e as portas), `registradores` (os bits que andam no clock dele), `estagios` ou `pipeline` (as molduras e as instruções do pipeline dele), `placas` ou `acesas` (os Arduinos do Blink dele, apagados e acesos), `fita` (as casas da fita de Turing do Vladimir, que o intercâmbio não troca), `estados` (o diagrama do autômato dele), `pilha` (os blocos da pilha do Salles), `baldes` e `chaves` (a tabela hash dele), `arvore` (os nós da árvore de busca dele, e os da árvore sintática do compilador do Vladimir), `vetor` (os elementos do bubble sort da repescagem dele), `pagina` (o cursor e o spinner da página da fase 1 do Lucas), `grade` (os cartões do DevTools da fase 2 dele), `supervisao` (os workers e as mensagens da árvore de supervisão da fase 3 dele), `eventloop` (as fichas do event loop da final dele), `cronograma` (as páginas das barras do cronograma da proposta do Hugo), `objetivo` (os alvos que abrem o escudo dele, e o que se pega nos objetivos de andar), `tabela` (os cabeçalhos e os valores da tabela do Thiago), `aviso` (o marcador da casa que a consulta dele vai ler) — todos menos o primeiro, a regra `posto` põe lá ao nascer. Um posto desconhecido é recusado — ninguém o ocuparia, e o ataque nunca dispararia |
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

**`Amarrar` é a corda dos guardrails do Júlio** — nasceu na primeira fase 1 do Luizinho (*ligāre*, "atar"), que hoje é o escopo. A letra
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
| `<professor>/fases.json` | `LerFases` | `FasesDeAtaqueArquivo.cpp` |
| `<professor>/regras.json` | `LerRegras` | `RegrasDeAtaqueArquivo.cpp` |
| `<professor>/projeteis.json` | `LerProjeteis` | `ProjeteisDeChefeArquivo.cpp` |
| `personagens.json` | `Personagens::LerCatalogo` | `PersonagensArquivo.cpp` |

É essa divisão que permite testar a leitura sem SDL e sem disco: os testes
entregam o texto do JSON direto para a função pura.
