# A fazer

O que ficou decidido mas não feito, com contexto suficiente para retomar sem
reconstruir a conversa. Não é lista de desejos: cada item aqui já foi discutido e
tem uma direção.

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

### A faixa que o jogador não usa não acompanha a barra de nota

`Battle::GetPlayfieldBounds` reserva 25 px na base da tela, e a barra de nota
agora tem 28 e fica recuada 54 px do fundo. Os dois números eram o mesmo e
deixaram de ser: um é **regra de jogo** — até onde o aluno pode descer — e o
outro é desenho. Hoje o aluno ainda pode parar em cima da barra.

Alinhar os dois muda onde se joga, então ficou de fora da arrumação visual.
**É decisão do autor:** o aluno deve ser impedido de entrar na faixa da HUD?

### A tela de Opções está vazia

Ela existe e não faz nada. No mínimo precisa de volume: o gabinete fica num
corredor.

---

## Ferramentas

### Portar um arnês de medição para `Ferramentas/`

`build-e-testes.md` descreve a técnica de instrumentar o jogo para medir e
fotografar, mas os scripts que fazem isso não estão no repositório. Um script
portável que entre direto numa fase e fotografe por instante de jogo pouparia
reescrevê-lo toda vez.

### Arte definitiva

Júlio e Thiago usam cartão provisório em vez de sprite de professor, e as
camadas de personagem são placeholders. `Ferramentas/gerar_thiago.py` mostra o
formato esperado de PNG e JSON.
