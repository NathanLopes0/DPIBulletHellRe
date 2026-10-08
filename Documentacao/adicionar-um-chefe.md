# Adicionar um professor

A receita completa, na ordem em que funciona. O exemplo é o Thiago, de INF 220,
que foi feito exatamente assim.

A maior parte do trabalho é em JSON. O C++ entra em três lugares só: a classe do
professor, a fábrica dele e uma linha de registro.

---

## 1. A arte

Duas imagens com um JSON de quadro ao lado de cada uma:

```
Assets/Teachers/DPIBH<Nome>.png              128x128, o professor
Assets/Teachers/DPIBH<Nome>.json             descreve os quadros do PNG
Assets/Teachers/Projectiles/DPIBH<algo>.png  o projétil
Assets/Teachers/Projectiles/DPIBH<algo>.json
```

O JSON é o formato do Aseprite, mas um arquivo escrito à mão serve. Com um
quadro só ele fica assim:

```json
{
  "frames": [{
    "filename": "DPIBHThiago.png 0.aseprite",
    "frame": { "x": 0, "y": 0, "w": 128, "h": 128 },
    "rotated": false, "trimmed": false,
    "spriteSourceSize": { "x": 0, "y": 0, "w": 128, "h": 128 },
    "sourceSize": { "w": 128, "h": 128 },
    "duration": 120
  }],
  "meta": { "image": "DPIBHThiago.png", "format": "RGBA8888",
            "size": { "w": 128, "h": 128 }, "scale": "1" }
}
```

Para arte provisória há `Ferramentas/gerar_thiago.py`, que desenha um cartão e
grava o PNG e o JSON juntos. Copiar e adaptar é mais rápido que escrever o JSON
na mão.

> **O quadro é o quadro, não a textura.** `GetSpriteWidth()` devolve a largura de
> **um quadro** (64 numa folha de 128 com dois quadros), e é esse número que o
> raio do colisor usa. Já custou uma hitbox do dobro do tamanho.

## 2. O projétil, em `Assets/Attacks/projeteis.json`

Um conjunto com o nome do professor. O nome de cada projétil dentro dele é como
`fases.json` vai pedi-lo:

```json
"thiago": {
  "Celula": {
    "sprite": "Teachers/Projectiles/DPIBHThiagoCelula.png",
    "dados":  "Teachers/Projectiles/DPIBHThiagoCelula.json",
    "animacoes": { "Linha": [0] },
    "animacaoInicial": "Linha",
    "escala": 2,
    "ordemDeDesenho": 90,
    "posicionarNoDono": false
  }
}
```

`posicionarNoDono` decide se o projétil nasce em cima do professor. Falso para
ataques que nascem em outro lugar — a `ConsultaAttack` nasce na borda da tabela,
e deixar verdadeiro faria tudo aparecer amontoado no meio da tela.

> **Cada tipo registrado custa 300 instâncias no pré-aquecimento.** Dois tipos
> parecidos que só mudam de cor devem ser duas animações do mesmo tipo, não dois
> tipos.

## 3. As regras dos projéteis, em `Assets/Attacks/regras.json`

Um conjunto por fase, ou um só reaproveitado. Cada regra se aplica a cada
projétil da rajada:

```json
"thiago_varredura": [
  { "animacao": "Linha", "escala": 2 }
]
```

Os detalhes de cada campo estão em `arquivos-de-dados.md`. O que importa aqui é
a divisão: **uma** `motion` por projétil (ela decide a direção) e quantos
`modifiers` quiser (eles mudam o módulo ou agendam coisas).

> **Motion captura a direção no instante em que ativa.** Se o projétil estiver
> parado nesse momento, não há direção para capturar e ele sai para a direita.
> Por isso os ataques que esperam antes de disparar não levam `motion` nenhuma.

## 4. As fases, em `Assets/Attacks/fases.json`

Um conjunto com o nome do professor e **quatro** fases, com estes nomes exatos:

```json
"thiago": {
  "StateOne":   { "duracao": 17, "proximo": "StateTwo",   "movimento": {...}, "ataques": [...] },
  "StateTwo":   { "duracao": 17, "proximo": "StateThree", "movimento": {...}, "ataques": [...] },
  "StateThree": { "duracao": 17, "proximo": "",           "movimento": {...}, "ataques": [...] },
  "StateFinal": { "duracao": 17, "proximo": "StateOne",   "movimento": {...}, "ataques": [...] }
}
```

`"proximo": ""` na terceira significa "a batalha é decidida aqui", e não "fim do
arquivo".

> **`StateFinal` é obrigatório**, mesmo num chefe que você pensa como de três
> fases. Duas notas chegam nela: entre 40 e 59 é o exame, e 100 é o teste final.
> Sem o estado, a máquina registra um erro no log e **o professor congela** — sem
> travar o jogo e sem avisar em tela.

## 5. A classe do professor

`Source/Actors/Teachers/Bosses/<Nome>.h` e `.cpp`. O mínimo é herdar de `Boss`.
O que justifica existir a classe é `CustomizeAttackParams`: é ali que mora o que
**não é dado**, isto é, o que depende de onde o jogador está no instante do
disparo.

```cpp
void Thiago::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    auto* consulta = dynamic_cast<ConsultaAttackParams*>(&params);
    if (!consulta) return;          // fases que não usam esta estratégia passam reto

    if (stateName == "StateTwo") {
        consulta->indice = FaixaDoJogador(*consulta);
    }
}
```

Se a mira do professor não depende do jogador, a classe pode ficar vazia — mas
ela tem de existir, porque a fábrica instancia um tipo concreto.

> **Estado que conta disparos mora aqui, não na estratégia.** As estratégias são
> sem memória de propósito: duas fases podem usar a mesma ao mesmo tempo, e um
> contador dentro dela seria compartilhado entre as duas.

## 6. A fábrica

`Source/Actors/Teachers/BossFactory/<Nome>Factory.h` e `.cpp`. São três métodos,
e dá para copiar a `ThiagoFactory` inteira trocando os nomes:

```cpp
std::unique_ptr<Boss> ThiagoFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Thiago>(scene);
}

void ThiagoFactory::ConfigureComponents(Boss* boss) {
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset("Teachers/DPIBHThiago.png"),
        Caminhos::Asset("Teachers/DPIBHThiago.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    const float raio = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    boss->AddComponent<CircleColliderComponent>(raio)->SetTag(ColliderTag::Boss);

    RegistrarProjeteisDeArquivo(boss, "thiago");     // o conjunto do passo 2
}

void ThiagoFactory::ConfigureAttacksAndFSM(Boss* boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    ConfigurarFasesDeArquivo(boss, fsm, "thiago");   // o conjunto do passo 4
    boss->SetInitialState("StateOne");
}
```

Os caminhos passam sempre por `Caminhos::Asset`, que resolve a pasta `Assets/`
subindo os diretórios. Caminho relativo escrito à mão quebra conforme de onde o
jogo é executado.

## 7. Registrar a fábrica

Duas linhas. Em `Source/Actors/Teachers/BossFactory/AllFactories.h`:

```cpp
#include "ThiagoFactory.h"
```

E em `Game::InitializeBossFactory`, em `Source/Game.cpp`:

```cpp
mBossFactory["thiago"] = std::make_unique<ThiagoFactory>(this);
```

A chave é o nome que `materias.json` vai citar. **Ela é procurada por texto**, e
um nome escrito errado não dá erro de compilação: a matéria abre, não acha chefe
e volta sozinha para a seleção.

## 8. Ligar à matéria, em `Assets/materias.json`

```json
{ "codigo": "INF220", "nome": "INF 220", "coluna": 2, "chefe": "thiago",
  "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"] } }
```

Uma matéria sem `chefe` é válida: aparece na grade e espera um.

## 9. Avisar os testes

Dois guardas em `tests/test_arquivos_de_dados.cpp` espelham o C++ e **vão
falhar** até serem atualizados — de propósito, para que ninguém acrescente um
professor pela metade:

```cpp
CHECK(comChefe == 5);   // quantas matérias têm professor
const std::set<std::string> kFabricasRegistradas = {"salles", "ricardo", "andre", "julio", "thiago"};
```

Se você esqueceu o passo 7, o segundo guarda falha dizendo o nome que ficou sem
fábrica. É esse o trabalho dele.

## 10. Conferir

```bash
cmake --build build && ./build/dpi_tests
```

A suíte lê os `Assets/` de verdade: um nome de fase inválido, um projétil que não
existe no conjunto, um índice de consulta fora da tabela ou uma transição
quebrada derrubam a suíte com a frase e o lugar. Se ela passa, o professor está
montado.

Para ver em tela sem jogar até a matéria, `build-e-testes.md` descreve os arneses
que entram direto numa fase e fotografam.

---

## Os erros que aparecem

| Sintoma | Causa provável |
|---|---|
| A matéria abre e volta sozinha para a seleção | o nome em `materias.json` não bate com a chave de `Game::InitializeBossFactory` |
| O professor aparece e não ataca | `ConfigurarFasesDeArquivo` devolveu falso; o log começa com `FASES:` e diz por quê |
| O professor congela no fim da terceira fase | falta `StateFinal` no conjunto |
| Os projéteis nascem todos em cima do professor | `posicionarNoDono` verdadeiro num ataque que nasce em outro lugar |
| Os projéteis saem todos para a direita | uma `motion` ativou enquanto o projétil estava parado |
| A hitbox do professor está do dobro do tamanho | `GetSpriteWidth()` é a largura do quadro, não da folha |
