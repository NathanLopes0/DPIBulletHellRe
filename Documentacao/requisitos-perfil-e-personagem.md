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
Mostra as personagens disponíveis, uma em destaque por vez, com o nome visível.
Navegação por esquerda e direita, confirmação por ENTER, ESC volta à matrícula.
Ao confirmar: grava o perfil com a personagem escolhida e progresso vazio (D5) e
entra na seleção de fase.

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

### RF7 — Catálogo de personagens
A lista de personagens é **dado, não código**: um arquivo em `Assets/`, com
identificador estável, nome exibido e caminhos da sprite e do atlas.
Acrescentar uma personagem é acrescentar os arquivos de arte e uma entrada —
sem recompilar.

## 4. Requisitos não-funcionais e restrições

### RNF1 — Quadros do mesmo tamanho
Todas as personagens usam quadros de 64×64, como a atual. O raio de colisão do
jogador é calculado a partir da largura da sprite
(`CircleColliderComponent(GetSpriteWidth() / 10.f)` em `Player.cpp`), então uma
personagem com quadro maior teria hitbox maior. Com D4 valendo, isso seria uma
diferença de jogo que ninguém decidiu ter. O tamanho é verificado em teste.

### RNF2 — Identificador estável, nunca a posição
O perfil grava o **identificador** da personagem, não o índice dela na lista.
É a mesma lição que o save das matérias já aprendeu: quando o save guardava a
posição, reordenar a lista trocava as notas de dono em silêncio.

### RNF3 — Ficha versão 4
O campo da personagem é **opcional**. Um save da versão 3 abre sem conversão e
cai na personagem padrão. Um identificador que não existe mais no catálogo
também cai na padrão, com o motivo relatado — mesma política da matéria que
saiu do curso.

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
| `Actors/Player/Player` | Lê o caminho da sprite do perfil, em vez de fixo no construtor |
| `Ficha` | Versão 4, campo da personagem, migração da 3 |
| `Game` | Guarda a personagem atual; `IdentificarAluno` se divide em criar e carregar |

Não muda: `Progresso`, `Materias`, `Exportacao`, `Relogio`, nem nada de
ataques. Um aluno que cria perfil e nunca joga não gera linha nenhuma no
`notas.csv`, porque a exportação é uma linha por aluno‑matéria **jogada**.

`FichaArquivo::Existe` já existe e é o que RF2 e RF4 precisam.

## 6. Riscos

- **Arte placeholder virando definitiva.** Mitigado por RF7: trocar a arte é
  trocar arquivo, não código.
- **Saves da versão 3 criados durante os testes.** Já existem na máquina de
  desenvolvimento. A migração da RNF3 é obrigatória, e tem teste.
- **A tela de matrícula acumulando responsabilidade.** Dois modos é o limite
  aceitável; um terceiro pede repensar em vez de mais um `if`.

## 7. Plano de incrementos

Um commit por item, na ordem. Cada um compila, passa nos testes e pode ser
jogado.

1. **Menu com três opções.** Opções já vazia e com volta. Visível e
   independente do resto.
2. **Catálogo de personagens em dados** + as artes de placeholder. Nenhuma tela
   muda ainda; entra com testes do catálogo.
3. **Ficha versão 4**: campo da personagem e migração da 3, com testes.
4. **O `Player` lê a personagem** do perfil em vez do caminho fixo.
5. **Tela de matrícula em dois modos**, com as recusas de D2 e D3.
6. **Tela de criação de personagem.**
7. **Ligação final**: Novo Jogo grava o perfil com a personagem escolhida,
   Carregar Perfil restaura, e a seleção de fase volta ao menu.
