# Requisitos — Menu, perfil por matrícula e criação de personagem

Levantamento feito antes de escrever código, para que as decisões fiquem
registradas junto com o que elas causaram. Escrito em 03/10/2026.

## 1. Contexto

Hoje o jogo tem um menu de uma tecla só: a barra de espaço leva à tela de
identificação, que pede a matrícula e entra na seleção de fase. A matrícula
identifica o aluno e o progresso dele é gravado em `Saves/<matricula>.json`.

Não há distinção entre "sou novo aqui" e "já joguei antes": digitar uma
matrícula carrega o que houver e segue. Também não há personagem — todo aluno
joga com a mesma sprite, fixa no construtor do `Player`.

Este documento descreve o menu com três caminhos, a separação entre criar e
carregar um perfil, e a escolha de personagem que passa a fazer parte do perfil.

## 2. Decisões

As quatro primeiras foram escolhidas pelo autor; as demais seguem delas.

| # | Questão | Decisão |
|---|---|---|
| D1 | De onde vêm as sprites | Placeholders desenhados agora, trocáveis por arte definitiva depois sem mexer em código |
| D2 | Novo Jogo com matrícula que já tem perfil | **Recusa** e manda usar Carregar Perfil |
| D3 | Carregar Perfil sem save | **Erro** na tela, sem sair dela |
| D4 | Efeito da personagem | **Só aparência** |
| D5 | Quando o perfil é gravado | Ao confirmar a personagem, com progresso vazio — senão o perfil recém-criado não existiria para o Carregar |
| D6 | Modo visitante | Continua, pela tecla TAB na tela de matrícula do Novo Jogo, com a personagem padrão e sem gravar nada |
| D7 | "Trocar usuário" na seleção de fase | Passa a voltar ao menu, porque trocar de usuário agora é escolher entre criar e carregar |
| D8 | O que a ficha grava | **As escolhas de aparência**, nunca uma personagem pronta — ver seção 2.1 |
| D9 | Como a sprite é montada | As camadas são compostas **uma vez**, numa textura só, ao carregar o perfil |

### 2.1 — A aparência é um conjunto de escolhas (D8)

O destino deste sistema não é uma lista de personagens prontas: é um corpo
template em pixel art mais camadas escolhidas separadamente — tom de pele, tipo
e cor de cabelo, tipo e cor de camisa, tipo e cor de calça. A personagem de cada
matrícula é montada a partir dessas escolhas.

O **tom de pele** é uma escolha como as outras, e não uma cor fixa do template.
Isso obriga os **sapatos** a saírem numa peça separada, em cor fixa: tingidos
junto com a pele, eles clareariam com ela e deixariam de ser sapatos.

### 2.1.1 — O personagem é visto de costas (D10)

Ele olha para cima, para o professor, que é quem fica no topo da tela. É por
isso que o sprite original do jogo não tem rosto: o que aparece é a nuca.

Isso **apagou uma camada**. O template chegou a ser desenhado de frente, com
olhos numa peça própria — eles precisavam ficar fora da camada de pele para não
serem tingidos com ela, e ainda levavam esclera clara para não sumirem nos tons
mais escuros. De costas nada disso existe: não há olho nem boca, e o cabelo
cobre o crânio inteiro em vez de parar numa franja.

O cabelo comprido desce só até o ombro, e é mais estreito que o torso. Na
primeira tentativa ele ia até a cintura e cobria a camisa inteira — escolher
roupa deixava de ter efeito visível, o que anula metade da tela de criação.

**Por isso a ficha grava as escolhas desde a versão 4, e não um identificador de
personagem pronta.** Se gravasse o identificador agora, trocar para as camadas
depois custaria uma segunda migração, e os saves criados no meio teriam um id
que alguém precisaria traduzir em combinação.

As "personagens prontas" do primeiro momento são, então, apenas **combinações
predefinidas** dessas mesmas escolhas. A tela de criação começa deixando
escolher entre algumas delas; quando passar a deixar escolher camada por
camada, **o formato do save não muda** — muda só a tela.

Em consequência, a arte de placeholder não é "cinco personagens inteiras": é um
corpo template mais algumas peças de cabelo, camisa e calça, todas alinhadas ao
mesmo corpo e nos mesmos quatro quadros.

### 2.2 — Composição numa textura só (D9)

Empilhar uma componente de desenho por camada parece mais simples, mas o
`Player` chama `GetComponent<DrawAnimatedComponent>()` em vários lugares — o
piscar da invencibilidade, a troca entre `Idle` e `Moving`, a largura usada no
raio de colisão. Com várias componentes, cada uma dessas chamadas pegaria só a
primeira camada e as outras ficariam para trás, visíveis e erradas.

Compondo as camadas numa textura única ao carregar, tudo depois disso continua
vendo **uma** sprite animada, com o mesmo atlas de quatro quadros, e nenhum
desses pontos muda. O custo acontece uma vez por perfil carregado.

A ordem é **corpo, sapato, calça, camisa, cabelo**: a camisa cobre a cintura da
calça e o cabelo cai por cima do ombro da camisa. Dessas, só o sapato não é
tingido.

A **regra** de composição — quais camadas, em que ordem, com que cores — é
pura e testável. Só o desenho dos pixels fica na ponte.

**Sobre D2 e D3.** As duas são a mesma regra vista dos dois lados: cada caminho
só aceita a matrícula que faz sentido para ele. O motivo é a máquina
compartilhada do departamento — um aluno que digita a matrícula de outro por
engano não pode apagar o progresso dele, e quem erra um dígito só redigita.

**Sobre D4.** Manter a escolha puramente cosmética evita rebalancear as quatro
fases e evita que a decisão vire vantagem competitiva num trabalho cuja nota é
a métrica do jogo. Se um dia a personagem mudar atributos, isso vira um módulo
próprio, com camada pura e testes.

## 3. Requisitos funcionais

### RF1 — Menu principal
Três opções: **Novo Jogo**, **Carregar Perfil**, **Opções**. Navegação por cima
e baixo, confirmação por ENTER. A opção em foco é visivelmente distinta das
outras. Substitui o "aperte espaço" atual.

### RF2 — Novo Jogo: matrícula
Tela de matrícula em modo *novo*. Valida pelas regras que já existem
(`Matricula::Validar`). Se já houver perfil para a forma canônica da matrícula,
mostra que esse perfil já existe e permanece na tela (D2). Se não houver, segue
para a criação de personagem. ESC volta ao menu. TAB entra como visitante (D6).

### RF3 — Criação de personagem
Mostra a personagem montada, com as opções disponíveis, e deixa trocá-la.
Confirmação por ENTER, ESC volta à matrícula. Ao confirmar: grava o perfil com
a aparência escolhida e progresso vazio (D5) e entra na seleção de fase.

A tela evolui em dois passos, **sem mudar o formato do save** (D8):

- **Agora:** escolhe entre algumas combinações predefinidas, navegando por
  esquerda e direita.
- **Depois:** escolhe camada por camada — tipo e cor de cabelo, de camisa e de
  calça — navegando entre as categorias por cima e baixo e entre as opções de
  cada uma por esquerda e direita.

### RF4 — Carregar Perfil
Tela de matrícula em modo *carregar*. Se não houver perfil para aquela
matrícula, mostra isso e permanece na tela (D3). Havendo, carrega progresso e
personagem e entra na seleção de fase. ESC volta ao menu.

### RF5 — Opções
Tela vazia por enquanto, com título e um caminho de volta ao menu por ESC.
Existe para que o menu já tenha o seu lugar definitivo.

### RF6 — A personagem no jogo
A sprite usada pelo `Player` na batalha é a do perfil carregado. O visitante e
qualquer perfil sem personagem válida usam a personagem padrão.

### RF7 — Catálogo de peças
O catálogo é **dado, não código**: um arquivo em `Assets/` que descreve, por
categoria (pele, cabelo, camisa, calça), as peças disponíveis — cada uma com
identificador estável, nome exibido, caminho da arte e se aceita cor. Descreve
também as cores oferecidas e as combinações predefinidas da primeira versão da
tela (D8).

Acrescentar um tipo de cabelo é acrescentar o PNG e uma entrada — sem
recompilar.

## 4. Requisitos não-funcionais e restrições

### RNF1 — Quadros do mesmo tamanho
Todas as personagens usam quadros de 64×64, como a atual. O raio de colisão do
jogador é calculado a partir da largura da sprite
(`CircleColliderComponent(GetSpriteWidth() / 10.f)` em `Player.cpp`), então uma
personagem com quadro maior teria hitbox maior. Com D4 valendo, isso seria uma
diferença de jogo que ninguém decidiu ter. O tamanho é verificado em teste.

### RNF2 — Identificador estável, nunca a posição
O perfil grava o **identificador** de cada peça escolhida, não o índice dela na
lista. É a mesma lição que o save das matérias já aprendeu: quando o save
guardava a posição, reordenar a lista trocava as notas de dono em silêncio.
Aqui o efeito seria mais discreto e pior de perceber — todo mundo acordaria com
outro cabelo depois de uma reordenação.

### RNF3 — Ficha versão 4
O campo da aparência é **opcional**, e guarda as escolhas (D8):

```json
"aparencia": {
  "corPele": "tom3",
  "cabelo": "curto",   "corCabelo": "castanho",
  "camisa": "regata",  "corCamisa": "verde",
  "calca":  "bermuda", "corCalca":  "jeans"
}
```

Um save da versão 3 abre sem conversão e cai na aparência padrão. Uma peça ou
cor que não existe mais no catálogo cai na padrão **daquela camada**, com o
motivo relatado, e as outras continuam valendo — mesma política da matéria que
saiu do curso. Perder o cabelo não pode custar a camisa.

### RNF4 — Camada pura
A leitura e a validação do catálogo ficam num módulo puro (`Personagens`), com
a ponte de disco separada (`PersonagensArquivo`), como `Materias` e
`MateriasArquivo`. Só o puro entra no `dpi_tests`.

### RNF5 — Uma tela de matrícula, dois modos
O Novo Jogo e o Carregar Perfil usam a **mesma** cena de matrícula,
parametrizada pelo modo. Duas cópias da tela divergiriam na primeira correção
feita em só uma delas.

## 5. O que muda no que já existe

| Arquivo | Mudança |
|---|---|
| `Scenes/MainMenu` | De uma tecla para três opções navegáveis |
| `Scenes/Identificacao` | Ganha modo (novo/carregar), mensagens próprias de cada um e volta ao menu |
| `Scenes/StageSelect` | "T — trocar usuário" passa a voltar ao menu (D7) |
| `Actors/Player/Player` | Usa a sprite composta do perfil, em vez do caminho fixo no construtor |
| `Ficha` | Versão 4, campo da aparência, migração da 3 |
| `Game` | Guarda a aparência atual; `IdentificarAluno` se divide em criar e carregar |

Não muda: `Progresso`, `Materias`, `Exportacao`, `Relogio`, nem nada de
ataques. Um aluno que cria perfil e nunca joga não gera linha nenhuma no
`notas.csv`, porque a exportação é uma linha por aluno‑matéria **jogada**.

`FichaArquivo::Existe` já existe e é o que RF2 e RF4 precisam.

## 6. Riscos

- **Arte placeholder virando definitiva.** Mitigado por RF7: trocar a arte é
  trocar arquivo, não código.
- **As camadas desalinhando.** Um cabelo desenhado para um corpo e usado em
  outro flutua. Mitigado por RNF1 e por um teste que confere que toda peça tem
  o mesmo tamanho de quadro do corpo.
- **Combinação ilegível.** Cabelo e camisa da mesma cor somem um no outro. É
  escolha do aluno, e não há o que impedir; o catálogo é que deve oferecer
  cores que se distinguem.
- **Saves da versão 3 criados durante os testes.** Já existem na máquina de
  desenvolvimento. A migração da RNF3 é obrigatória, e tem teste.
- **A tela de matrícula acumulando responsabilidade.** Dois modos é o limite
  aceitável; um terceiro pede repensar em vez de mais um `if`.

## 7. Plano de incrementos

Um commit por item, na ordem. Cada um compila, passa nos testes e pode ser
jogado.

1. **Menu com três opções.** Opções já vazia e com volta. Visível e
   independente do resto.
2. **Arte em camadas**: corpo template, cabelos, camisas e calças de
   placeholder, alinhados e nos mesmos quatro quadros.
3. **Catálogo de peças em dados** e o módulo puro da aparência — o que é uma
   escolha válida, qual a padrão, o que fazer com uma peça que sumiu. Com
   testes; nenhuma tela muda ainda.
4. **Composição da sprite**: montar as camadas tingidas numa textura só (D9).
5. **Ficha versão 4**: a aparência gravada e a migração da 3, com testes.
6. **O `Player` usa a aparência** do perfil em vez do caminho fixo.
7. **Tela de matrícula em dois modos**, com as recusas de D2 e D3.
8. **Tela de criação de personagem**, na versão de combinações predefinidas.
9. **Ligação final**: Novo Jogo grava o perfil com a aparência escolhida,
   Carregar Perfil restaura, e a seleção de fase volta ao menu.

A escolha camada por camada (RF3, segundo passo) vem depois disso, e por
construção não mexe em save nem em composição — só na tela.
