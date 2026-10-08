# A fazer

O que ficou decidido mas não feito, com contexto suficiente para retomar sem
reconstruir a conversa. Não é lista de desejos: cada item aqui já foi discutido e
tem uma direção.

---

## Arquitetura

### Separar os arquivos de ataque por professor

Hoje `fases.json`, `regras.json` e `projeteis.json` são um arquivo cada, com
todos os professores dentro. Ficariam melhor como uma pasta por professor:

```
Assets/Attacks/thiago/fases.json
Assets/Attacks/thiago/regras.json
Assets/Attacks/thiago/projeteis.json
```

**O que isso mexe.** Os três leitores guardam os conjuntos num mapa global
carregado uma vez (`GarantirArquivoLido`), e as fábricas pedem pelo nome do
professor. Com pastas, quem lê passa a precisar do nome *antes* de ler, e a
leitura vira sob demanda em vez de tudo de uma vez. As funções puras
(`LerFases`, `LerRegras`, `LerProjeteis`) recebem texto e não mudam — só as
pontes `*Arquivo.cpp` e os testes que leem os Assets de verdade.

**Vale notar:** `regras.json` tem conjuntos nomeados por professor
(`thiago_varredura`), e com pastas esse prefixo deixa de ser necessário. Trocar
os nomes é mexer em `fases.json` junto.

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

**Falta:**

- **A tela de atração.** Hoje a máquina parada mostra o menu principal parado.
  Ela sairia do menu, e `Ranking::Geral` já dá o que mostrar.
- **Recuperação se o jogo cair.** Nada disso adianta se um `crash` deixa a área
  de trabalho à vista até alguém passar no corredor. É script de sistema, não
  código do jogo: um laço que reabre.

### As teclas que aparecem escritas na tela

A seleção de fases diz "T — trocar usuário", e o gabinete não tem a tecla T.
**Toda tecla que aparece escrita em tela precisa existir no painel de controle.**

Vale varrer as telas de uma vez: o rodapé da seleção, a ajuda da identificação,
o ESC que várias cenas usam como "voltar". O painel é manche + 2 ou 3 botões, e
hoje cada cena escolhe a tecla dela por conta própria — é a forma do erro que já
apareceu quatro vezes neste projeto: a mesma coisa descrita em dois lugares, e
uma das cópias envelhece.

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
