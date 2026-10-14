# O gabinete

O jogo foi feito para rodar num **gabinete de arcade** no departamento, com duas
posições de jogador. Lá não há teclado, não há mouse, a máquina fica ligada o dia
todo e ninguém fecha o jogo entre um aluno e o outro.

Este documento é para quem monta e mantém essa máquina. Quem está desenvolvendo
não precisa de nada daqui: **o padrão é a janela**, do jeito que o CLion abre.

---

## Como abrir

```bash
./DPIBulletHellRe --arcade
```

É essa linha que vai no script de inicialização da máquina.

| | |
|---|---|
| `--arcade` | liga o modo gabinete |
| `--ociosidade <segundos>` | quanto tempo parado volta ao menu. `0` desliga |

Sem `--arcade`, o jogo abre em janela, fecha no X e não volta ao menu sozinho.

`--ociosidade` vale **com ou sem** `--arcade` — é assim que se testa a volta ao
menu numa janela, sem esperar um minuto e sem perder a tela de quem desenvolve.

Argumento que o jogo não conhece é ignorado, e os seguintes continuam valendo.
Um erro de digitação no script não pode impedir o jogo de abrir numa máquina
onde ninguém vê o terminal.

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
ela já termina por tempo.

**Do menu a ociosidade leva para a frente**, não de volta: passado o mesmo tempo
parado no menu principal, entra a **tela de atração** — o título com a chamada e
os melhores do curso, alternando a cada seis segundos. Qualquer tecla volta ao
menu; não há o que escolher ali, então não há botão certo.

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
| **Botão 2** | `B` | `G` | voltar, apagar, sobrecarga |
| **Botão 3** | `N` | `H` | o extra de cada tela (hoje, o ranking da matéria) |

São essas as teclas que o encoder do painel precisa mandar — **catorze, não
sete**, e nenhuma repetida entre as duas posições. Uma tecla repetida faria os
dois manches moverem o mesmo aluno, que era o estado do código antes de a
segunda posição existir: o `W A S D` era atalho das setas.

As teclas moram numa tabela só, em `Source/Painel.cpp`. É ela que muda no dia
em que o encoder chegar e mandar outra coisa; nenhuma cena e nenhum jogador
guarda tecla própria.

**O JOGO AINDA USA UMA POSIÇÃO.** A segunda já tem teclas e já é lida pelo
módulo do painel, mas nenhuma tela coloca um segundo jogador em campo. O
caminho até lá está em `requisitos-coop.md`.

**Toda tela é inteiramente operável com os três.** Nenhuma ação fica só no
teclado, e nenhum rodapé anuncia tecla que o painel não tem — `Painel::Rodape`
monta o texto a partir do próprio botão que a cena escuta, então as duas coisas
não têm como divergir. `ENTER` e `ESC` continuam valendo para quem desenvolve,
mas **não aparecem escritos em lugar nenhum**.

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

## O que ainda falta

Está em `a-fazer.md`, na seção *Gabinete*: a recuperação se o jogo cair. É
script de sistema, não código — um laço que reabre.

---

## Onde isso está no código

`Source/Gabinete.h` tem a camada pura: a leitura da linha de comando e a
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

`Source/Painel.h` tem os botões: a tecla de cada um, o nome com que cada um
aparece escrito, e o montador de rodapé. Nenhuma cena escreve `SDL_SCANCODE_`
para os botões do painel.

Os testes estão em `tests/test_gabinete.cpp` e `tests/test_painel.cpp`.
