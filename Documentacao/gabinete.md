# O gabinete

O jogo foi feito para rodar num **gabinete de arcade** no departamento, com duas
posições de jogador. Lá não há teclado, não há mouse, a máquina fica ligada o dia
todo e ninguém fecha o jogo entre um aluno e o outro.

Este documento é para quem monta e mantém essa máquina. Quem está desenvolvendo
não precisa de nada daqui: **o padrão é a janela**, do jeito que o CLion abre.

---

## O pacote do gabinete

O gabinete é um Ubuntu 20.04 **sem internet**. O pacote dele sai pronto de

```bash
bash Ferramentas/executavel-gabinete.sh   # dist/DPIBulletHellRe-gabinete.tar.gz
```

e é compilado com `DPI_GABINETE` (no `CMakeLists.txt`). Esse executável **já abre no
modo gabinete**, sem precisar do `--arcade`, e não tem nada do que é para quem
desenvolve: o modo de simulação (a batalha direta, os pilotos, as fotos) é
ignorado, e as teclas F9 e F10 da seleção de fases não existem. Ele leva o SDL2
junto (em `lib/`) e não pede nada além do que o Ubuntu já tem.

**Rode o script num Ubuntu 20.04** — uma máquina, um container ou um chroot
(`debootstrap focal`): o executável funciona na glibc de quem compilou ou numa
mais nova, e o feito num Ubuntu mais novo não abre no gabinete. O cabeçalho do
script lista o que instalar antes.

No gabinete:

```bash
tar -xzf DPIBulletHellRe-gabinete.tar.gz
cd DPIBulletHellRe-gabinete
./jogar.sh                        # abre o jogo
./instalar.sh --abrir-ao-ligar    # o ícone, e o jogo abre sozinho no login
```

O `jogar.sh` **reabre o jogo se ele cair** (e anota a queda em
`Saves/quedas.log`); só a saída do operador (abaixo) o fecha de vez. Para o jogo
abrir quando a máquina liga, a sessão também precisa entrar sozinha:
Configurações › Usuários › Login automático. O progresso, o ranking e os
registros das tentativas ficam em `Saves/`, ao lado do jogo.

## Como abrir

```bash
./DPIBulletHellRe --arcade
```

É essa linha que vai no script de inicialização de uma máquina com o executável
comum — o do pacote do gabinete já abre assim.

| | |
|---|---|
| `--arcade` | liga o modo gabinete |
| `--ociosidade <segundos>` | quanto tempo parado volta ao menu. `0` desliga |
| `--condicao <nomes>` | a condição de um experimento: as mecânicas que esta sessão desliga (`sem-brilho`, `sem-papeis`...). Sem ela, tudo ligado. Ver `dados-das-tentativas.md` |

Sem `--arcade`, o jogo abre em janela, fecha no X e não volta ao menu sozinho.

`--ociosidade` vale **com ou sem** `--arcade` — é assim que se testa a volta ao
menu numa janela, sem esperar um minuto e sem perder a tela de quem desenvolve.

Argumento que o jogo não conhece é ignorado, e os seguintes continuam valendo.
Um erro de digitação no script não pode impedir o jogo de abrir numa máquina
onde ninguém vê o terminal.

Os parâmetros do **modo de simulação** (`--batalha`, `--piloto`, `--passo`...)
**não vão neste script**: com eles o jogo pula o menu, joga sozinho e pode até
fechar no fim da luta. Servem para medir o jogo sem tela, e estão em
`build-e-testes.md`.

---

## O que `--arcade` muda

**Tela cheia.** A janela nasce do tamanho do monitor. O jogo inteiro é
posicionado em 1200×800 — inclusive as telas que usam frações da altura — e quem
reconcilia os dois é a escala lógica do renderizador, que amplia e centraliza
sem nenhuma coordenada do jogo mudar. Monitor de proporção diferente ganha tarja
preta em cima e embaixo, não corte.

**Sem cursor.** Não há mouse no painel; sem isso o ponteiro ficaria parado no
meio da tela o dia inteiro.

**Sem saída acidental.** Fechar a janela deixa de existir: um Alt+F4 de aluno
curioso mostraria a área de trabalho. Ver **Como sair**, abaixo.

**Volta ao menu sozinha.** Passado o tempo de `--ociosidade` sem ninguém mexer,
a tela volta ao menu principal **e o perfil do aluno é solto**. Sem isso, quem
chegasse depois encontraria a sessão de outra pessoa e jogaria no nome dela.

A **batalha não volta sozinha**: ficar parado é uma forma legítima de desviar, e
ela já termina por tempo. A exceção é a batalha **pausada**, que ninguém retomou
(ver a pausa, abaixo).

**Do menu a ociosidade leva para a frente**, não de volta: passado o mesmo tempo
parado no menu principal, entra a **tela de atração** — o título com a chamada,
os melhores do curso (com a personagem de cada um) e o mural dos formados (quem passou no TCC, de capelo, dos
mais recentes), alternando a cada seis segundos. Um cartaz sem ninguém para
mostrar não entra. Qualquer tecla volta ao menu; não há o que escolher ali,
então não há botão certo.

A tela de atração também não volta sozinha: ela já é a tela de quem não está
jogando.

O cartaz dos melhores só aparece se houver ficha gravada. Numa máquina
recém-instalada a atração mostra só o título.

---

## O painel

**Duas posições**, cada uma com manche e **três botões**. É tudo o que existe
lá, e é por isso que nenhuma tela pode exigir outra tecla.

| | Jogador 1 | Jogador 2 | |
|---|---|---|---|
| **Manche** | setas | `W A S D` | andar, escolher |
| **Botão 1** | `Espaço` | `F` | confirmar, atirar, digitar |
| **Botão 2** | `B` | `E` | voltar, apagar, sobrecarga |
| **Botão 3** | `N` | `H` | o extra de cada tela (hoje, o ranking da matéria) |
| **Start** | `P` | `T` | pausa a batalha (abaixo) |
| **Select** | `O` | `R` | abre as opções, das telas de menu (abaixo) |
| **Foco** | `Shift` esquerdo | `Q` | segurado, o aluno anda devagar e vê a hitbox |

São essas as teclas que o encoder do painel manda quando ele se apresenta como
**teclado** — **vinte, e não dez**, e nenhuma repetida entre as duas posições.
O painel tem sete botões ao lado de cada manche; o jogo usa quatro — os três de
jogo e o **foco** —, e os outros três ficam livres. O foco é, de preferência, o
mais perto do manche: segurado, o aluno anda na metade da velocidade
(`Player::kFatorDoFoco`) e vê a própria hitbox. No encoder de joystick, ele é o
`"foco"` do `Assets/painel.json` (no padrão, o botão 3). Uma tecla repetida faria os
dois manches moverem o mesmo aluno, que era o estado do código antes de a
segunda posição existir: o `W A S D` era atalho das setas.

As teclas moram numa tabela só, em `Source/Entrada/Painel.cpp`. É ela que muda no dia
em que o encoder mandar outras teclas; nenhuma cena e nenhum jogador guarda tecla
própria.

### O encoder que é joystick

Muitos encoders de arcade (os USB "zero delay", por exemplo) não se apresentam
como teclado, e sim como **joystick**: o manche vira eixos ou um direcional, e
os botões viram botões de joystick. O jogo lê os dois. Cada joystick que o
sistema mostra é aberto, inclusive o que é ligado com o jogo aberto
(`Source/Entrada/Manches.h`), e a cada quadro o que ele aperta vira **as mesmas
teclas da tabela acima** (`Source/Entrada/MapaDoManche.h`). Por isso nenhuma tela
precisou mudar: elas continuam lendo só o teclado.

Qual joystick é de qual posição, e quais eixos e botões são o manche e os Botões
1, 2 e 3, está em **`Assets/painel.json`**. Ele pode ser editado no gabinete, sem
compilar. O padrão: o primeiro joystick é o jogador 1, o segundo é o jogador 2,
o manche é o eixo 0/1 ou o direcional 0, e os Botões 1, 2 e 3 são os botões 0, 1
e 2. O arquivo também cobre o encoder único para as duas posições (o mesmo
`dispositivo` nas duas, com eixos e botões diferentes) e o que manda o manche
como botões (`direcoes`).

Para acertar um encoder novo:
1. Abra o jogo pelo `jogar.sh`.
2. Mexa cada manche para os quatro lados e aperte cada botão.
3. Saia com Ctrl + Esc.
4. Leia `Saves/jogo.log`. As linhas `MANCHE:` dizem cada joystick ligado (nome,
   eixos, direcionais, botões) e cada coisa que se mexeu, por exemplo
   `MANCHE: joystick 0, botao 3`, `MANCHE: joystick 0, eixo 1 -` ou
   `MANCHE: joystick 0, direcional 0 = cima`. Esses números são os do
   `painel.json`.

Um erro no arquivo não impede o jogo de abrir: o que não se lê fica no padrão, e
o log diz o que foi.

**As duas posições são usadas** no Modo Dupla; no Modo Solo, só a do jogador 1.

**Toda tela é inteiramente operável com os três.** Nenhuma ação fica só no
teclado, e nenhum rodapé anuncia tecla que o painel não tem — `Painel::Rodape`
monta o texto a partir do próprio botão que a cena escuta, então as duas coisas
não têm como divergir. `ENTER` e `ESC` continuam valendo para quem desenvolve,
mas **não aparecem escritos em lugar nenhum**.

**Cada posição tem um Start e um Select** (`Painel::Sistema`), fora dos botões
de jogo. Pelo encoder de teclado são as teclas da tabela; pelo de joystick, os
botões `"start"` e `"select"` do `Assets/painel.json` (no padrão, o 9 e o 8, a
numeração comum dos encoders "zero delay"; o nome antigo `"pausa"` ainda vale
como o Start).

**O Start pausa a batalha**, de qualquer jogador: abre o mesmo menu do
computador — continuar, recomeçar a fase, sair dela —, o rodapé diz
"START continuar", e apertá-lo de novo também continua, com a contagem 3, 2, 1.
No computador o `ESC` faz o mesmo, sem aparecer escrito.

**O Select abre as opções**, de qualquer jogador, nas telas de menu — o menu
principal, a seleção de fases, a dificuldade, o boletim, as horas e o Como
Jogar —, e voltar delas (o Botão 2, ou o Select de novo) leva à mesma tela de
onde se veio (`Game::AbrirOpcoes`). Na batalha ele não faz nada, e também não
no meio de uma escolha — a matrícula sendo digitada, a personagem sendo
montada, o orientador sendo votado —, que se perderia. No gabinete as opções
não têm a linha da tela cheia: o jogo está sempre em tela cheia.

Uma batalha congelada no corredor seria uma tela morta para quem chega depois,
então **a pausa esquecida conta como ociosidade**: passado o tempo de
`--ociosidade` com o menu da pausa aberto e ninguém mexendo, o jogo volta ao
menu principal, solta o perfil e registra a tentativa como abandonada
(`Scene::Pausada`, `ParaOndeAOciosidadeLeva`). A luta em andamento continua
sem ser interrompida. Perder o foco da janela não pausa no gabinete: lá ninguém
troca de janela. O `Ctrl + Esc` de quem mantém a máquina continua sendo o de
sempre.

**A tela de resultado volta sozinha.** A ociosidade do gabinete não vale dentro
da batalha — uma luta em andamento não pode ser interrompida —, e a tela de
resultado é parte da batalha. Para a máquina não ficar parada no resumo de quem
foi embora sem escolher, ela volta para as matérias depois de 20 segundos
(`Resultado::kEsperaNoGabinete`), e dali a ociosidade de sempre assume.

Cada tela dá o significado dos botões no rodapé. O Botão 2 é quase sempre
"voltar" — na seleção de fases, voltar é trocar de aluno, e o rodapé diz isso
com todas as letras, porque a sessão do aluno acaba ali.

**Fora do gabinete, o rodapé mostra também a tecla**: "BOTÃO 1 (ESPAÇO)
jogar". Quem está testando no computador não tem botão na mão e não tem como
saber qual é o Botão 2. Com `--arcade` a tecla some, porque lá não há teclado e
ela seria ruído. Quem decide isso é o `Game`, numa linha só
(`JeitoDoRodape`) — não cada tela.

---

## Como sair

```
Ctrl + Esc, segurado por 2 segundos
```

Nenhuma das duas teclas existe no painel: para sair é preciso ligar um teclado
de propósito. E são dois segundos inteiros, não um toque — um encostão não
derruba o jogo no meio do corredor.

**Se esta combinação mudar, mude aqui também.** É a única saída da máquina.

---

## Onde isso está no código

`Source/Motor/Gabinete.h` tem a camada pura: a leitura da linha de comando e a
contagem de tempo. As duas esperas do gabinete — "ninguém mexeu" e "o operador
está segurando" — são a **mesma** classe com a condição trocada, e um limite
zero desliga a contagem. É por isso que não há `if (arcade)` espalhado:
fora do gabinete as duas nascem desligadas.

`Game::AtualizarGabinete` é quem roda isso a cada quadro, e
`Game::Initialize` é quem abre a janela em tela cheia. Para onde a ociosidade
leva em cada tela está numa tabela só, em `ParaOndeAOciosidadeLeva`.

`Source/Scenes/Atracao.h` é a tela de atração. Os cartazes se alternam com o
**mesmo** `Gabinete::Contagem` das duas esperas — terceiro uso da classe, com a
condição sempre verdadeira.

`Source/Entrada/Painel.h` tem os botões: a tecla de cada um, o nome com que cada um
aparece escrito, e o montador de rodapé. Nenhuma cena escreve `SDL_SCANCODE_`
para os botões do painel.

Os testes estão em `tests/test_gabinete.cpp`, `tests/test_painel.cpp` e
`tests/test_mapa_do_manche.cpp`.
