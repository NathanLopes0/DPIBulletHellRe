# A fazer

O que ficou decidido mas não feito, com contexto suficiente para retomar sem
reconstruir a conversa. Não é lista de desejos: cada item aqui já foi discutido e
tem uma direção.

---

## Balanceamento

### O Salles deixa o jogador parado aprovar

`Ferramentas/parado-reprova.sh` roda os oito chefes com um piloto que não se
mexe e atira sem parar. **Sete reprovam; o Salles passa com 91,02** — 12
acertos sofridos e **309 acertos dados**, o maior de todos (o segundo é o Júlio
com 314, mas ele toma 36 e termina em zero).

A causa está identificada e é a mesma que a primeira versão do Vladimir tinha:
**três das quatro fases do Salles usam `GoToCenter`**. Parado no meio da tela,
ele fica permanentemente na coluna de quem atira para cima — o jogador acerta
de graça a luta inteira e a nota sobe sozinha.

A cura também é a mesma: **dar um gesto a cada fase**, como em
`MovementStrategies.h`. E aqui o gesto tem o que dizer, porque as fases dele são
estruturas de dados:

| Fase | Estrutura | Gesto candidato |
|---|---|---|
| 1 | lista encadeada | varredura — a travessia tem direção |
| 2 | lista duplamente encadeada | varredura que **inverte** — ida e volta |
| 3 | árvore binária | mergulho — a profundidade é o nível |
| final | lista circular | órbita — o laço que fecha |

**Atenção ao mexer:** a batalha do Salles é a referência da rede de regressão
(`provaigual.sh` contra `base_commit77.bruto`), que vem idêntica desde o commit
77 e já pegou coisa de verdade. Mudar o balanceamento dele **invalida a
referência de propósito** — então a referência precisa ser regerada no mesmo
commit, com o motivo escrito, e não "consertada" depois.

---

## Arquitetura

### Os comentários da fase 2 do Salles citam números que os dados não têm

Encontrado ao separar os arquivos por professor, e **não corrigido** porque não
dá para saber qual lado está certo sem jogar.

| | O comentário diz | O arquivo diz |
|---|---|---|
| `atraso` do caminho (`salles/regras.json`, `lista_dupla`) | 0,85 | **1,2** |
| `cooldown` da fase (`salles/fases.json`, `StateTwo`) | 3,6 | **1,2** |

Os dois números aparecem também em `Assets/Paths/formas.json`, onde uma conta de
geometria (`332px`) foi derivada do 0,85. A regra crítica continua valendo —
`1,2 > 0,70`, e há teste exigindo isso —, então o jogo está correto; o que
envelheceu foi a prosa.

O comentário do `cooldown` merece atenção: ele diz que abaixo de 3,6 "uma fila
nova sai enquanto a anterior ainda está voltando, e as duas se cruzam em sentidos
opostos". Com 1,2 isso acontece. Pode ter sido de propósito.

### JSON ou C++ para os ataques

Ficou em aberto se os ataques continuam vindo de arquivo ou voltam para o
código. Hoje estão em arquivo e funcionam; a dúvida é se o ganho compensa a
indireção. Não mexer sem decidir isso primeiro.

---

## Jogo

### Co-op para dois jogadores

É o motivo de o gabinete ter duas posições. Precisa de uma rodada de requisitos
como a que o sistema de perfis teve. O que já se sabe:

- As fichas são por matrícula, então **duas notas independentes já funcionam**
  sem mudar nada no armazenamento.
- A fase é por tempo, e não por vida do chefe, então dois jogadores não
  encurtam a batalha.
- O balanceamento real está nos ataques mirados: eles precisam de uma regra para
  escolher entre dois alvos.

**Em aberto:** o que acontece quando **um** dos dois zera a nota.

### Modo Normal e modo Difícil

No Normal a Dúvida persegue o professor; no Difícil ela sobe reta, como hoje. A
peça já existe: é um `TrackingBehavior` no `PlayerProjectile`, com força
ajustável — força 0 é o comportamento atual.

Isso ataca a causa medida de o 100 ser difícil: o tiro sobe reto, então fugir
desalinha e todo tiro erra. Ver `Nota::kBonusDeFaseLimpa`, que ataca o mesmo
problema pelo outro lado.

### A tabela do Thiago não é visível

A batalha do INF 220 trata o campo como uma tabela, mas não há tabela desenhada:
o jogador deduz as faixas pelos ataques. Uma grade de fundo que acenda a faixa
consultada fecharia a ideia. Seria cenário, não projétil.

### Os seis professores que faltam

INF 330, INF 332, BIOINF, INF 394, VISCCP e TCC estão na grade sem chefe. A
receita está em `adicionar-um-chefe.md`, e as estratégias existentes cobrem
muita coisa — só o Thiago precisou de uma nova.

### A fase 1 do Thiago

Duas coisas para decidir jogando:

- O `cooldown` é 0,9 e o `aviso` é 1,1, ou seja, uma consulta nova é anunciada
  antes de a anterior disparar. Funciona como fase difícil, mas ela era a que
  ensinava a tabela.
- Ela usa 3 linhas enquanto as fases 2 e 3 usam 4, então as fronteiras mudam de
  lugar no meio da batalha.

---

## Gabinete

### Modo arcade de verdade

A janela está pronta: `--arcade` dá tela cheia com escala lógica, esconde o
cursor, ignora o fechar da janela, volta ao menu sozinho quando fica parado — e
solta o perfil do aluno ao voltar. Sair é `Ctrl+Esc` segurado. Está documentado
em `gabinete.md`.

A tela de atração entrou depois: parado no menu, o gabinete passa a mostrar o
título com a chamada e os melhores do curso.

**Falta:**

- **Recuperação se o jogo cair.** Nada disso adianta se um `crash` deixa a área
  de trabalho à vista até alguém passar no corredor. É script de sistema, não
  código do jogo: um laço que reabre.

### Decidir quantos botões o painel terá de verdade

O jogo hoje supõe **três** (ver `Painel.h` e `gabinete.md`), e o terceiro é só
atalho: o ranking da matéria. Se o gabinete acabar com dois, nada fica
inalcançável — basta tirar o Botão 3 do rodapé da seleção e dar outro caminho ao
ranking. Decidir isso é compra de peça, não código.

### Os três `GetPlayerPosition` do `BossProjectile` ainda se chamam assim

`GetPlayerPosition`, `GetPlayerVelocity` e `GetPlayerDirection` são a camada
fina entre três chamadas a `Battle::Alvo()` e o `Behaviors.cpp`. Elas carregam
a mesma mentira que `GetPlayer()` carregava — "o jogador", no singular — e a
verdade é "o alvo". Ficaram de fora do incremento 2 de propósito, para o
incremento não crescer para dentro da lógica de ataque; o risco é baixo porque
elas passam por `Alvo()` e não têm como escolher outro jogador.

### A tela de Opções está vazia

Ela existe e não faz nada. No mínimo precisa de volume: o gabinete fica num
corredor.

---

## Ferramentas

### Portar o resto dos arnêses para `Ferramentas/`

`Ferramentas/medir-dificuldade.sh` é o primeiro: ele monta uma cópia da árvore
com semente, passo de tempo e piloto presos, e foi com ele que a seção 8 de
`requisitos-coop.md` foi medida — rodá-lo reproduz os números da tabela.

Faltam os outros dois que `build-e-testes.md` descreve e que vivem fora do
repositório: o que **fotografa** uma cena num quadro exato (foi assim que o
retângulo vazio no lugar do "+" apareceu) e o que **dirige o menu** por um
roteiro de botões.

### Arte definitiva

**Vladimir também**, com o mesmo tipo de cartão (`Ferramentas/gerar_vladimir.py`:
um autômato de dois estados). Os projéteis dele são os símbolos `a` e `b` do
alfabeto — esses **não** são provisórios no conceito, só no acabamento: num chefe
de linguagens formais a rajada é uma palavra e cada tiro é uma letra dela.

Júlio e Thiago usam cartão provisório em vez de sprite de professor, e as
camadas de personagem são placeholders. `Ferramentas/gerar_thiago.py` mostra o
formato esperado de PNG e JSON.
