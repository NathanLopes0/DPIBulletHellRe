### Refatoração
Esse projeto é uma refatoração do projeto abaixo, usando técnicas de Engenharia de Software pra melhorar manutenção e escalabilidade visando adicionar os 10 bosses e outros sistemas.

## Informações Iniciais
INF 216 - Projeto e Implementação de Jogos Digitais, 2023.2
Autor - Nathan da Silva Lopes

# DPI Bullet Hell
DPI Bullet Hell é um jogo onde o jogador é um aluno tentando passar pelas matérias do curso de Ciência da Computação para se formar. Minha intenção com a criação desse jogo é ter um jogo de fácil aprendizado de controles com uma temática facilmente identificável no meio acadêmico, e que possa ser reutilizado com poucas mudanças. A audiência inicial são alunos e professores de Ciência da Computação da UFV, que possivelmente reconhecerão as artes e referências do jogo.

## Mecânicas Básicas
O jogo é um "Bullet Hell", com um formato parecido com jogos de nave antigos, exceto que a nave é um aluno e o boss são professores do DPI. Além disso, ele não tem um número de Vidas e sim, uma ***Nota***, que começa em 40 a cada fase. Os projéteis dos alunos são ***Dúvidas*** que eles jogam nos professores que aumentam sua Nota. Ser acertado diminui sua Nota. Depois de receber muitas Dúvidas, os professores jogam ***Pontos Extras*** no aluno que pode ser coletado e guardado pra usar depois.

![](https://github.com/ufv-inf216/projeto-final-NathanLopes0/blob/main/gameplay1.gif)

O aluno pode ser movido com as setas do teclado ou as teclas WASD, como o jogador preferir. A Barra de Espaço lança as Dúvidas, e a tecla B usa um dos Pontos Extras que o aluno tenha coletado para limpar a tela dos projéteis dos professores e ganhar aproximadamente 10 pontos, mas **fica sobrecarregado e não pode lançar Dúvidas por alguns segundos**.


## Condições de Vitória e Derrota
Um aluno passa de fase quando chega no fim do tempo da fase com nota maior ou igual a sessenta. Caso termine com menos, ele deve refazer a fase, e sua nota volta para a inicial (40).

Ele tem que passar de 3 provas (fases) de uma matéria pra passar dela por completo, então, por exemplo, depois que passar da fase do professor Ricardo 3 vezes, ele passará da matéria. Nessa demonstração há apenas 2 professores completos (INF 213, INF 250), e deve-se passar de 3 provas de cada. O objetivo é acumular 60 pontos ou mais no final das provas.

Por enquanto, há uma das 3 fases de INF 330, mas as outras duas o professor fica parado.

O jogo acaba em derrota se, a qualquer momento, sua nota chegar a 0.

### Versão e Funcionalidades

Versão 0.4.1

Principais funcionalidades que devem ser testadas:

* Movimentação -> Velocidade do Jogador, **tamanho da área jogável adequado?**
* Colisão -> Colisões adequeadas, agradáveis?. É uma colisão "acreditável"? Ou as vezes não parece que haverá colisão e o jogador perde pontos mesmo assim?
* Dificuldade -> Velocidade das "Balas" e Movimento das mesas, **Progressão de Dificuldade entre matérias**
* Artes -> Temática das artes, Qualidade da visualização **(cores adequadas, é possível enxergar o que é necessário, em qualquer que seja o momento do jogo?)**
* Ataques dos Chefes -> Interessantes? É possível aprender uma estratégia rápida pra aumentar as chances de escapar?
