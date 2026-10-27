//
// Created by nslop on 03/04/2024.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "Math.h"
#include "Scenes/Scene.h"
#include "Personagens.h"
#include "Progresso.h"
#include "Gabinete.h"
#include "Painel.h"
#include "Sessao.h"
#include "Configuracao.h"

class IBossFactory;
class Actor;

class Game {
public:

    // UMA MATERIA E O INDICE DELA em Materias::Carregadas().
    //
    // Aqui havia um enum com as dez materias escritas a mao. Ele nao descrevia
    // mais nada que o arquivo nao dissesse - chefe, coluna e desbloqueio ja vem
    // de la - e so dava a impressao de que a lista morava no C++. O indice NAO
    // vai para o disco: o save grava o codigo da materia.

    /// @param gabinete o que a linha de comando pediu. O padrao e janela; ver
    ///        Gabinete.h para o que muda no arcade.
    Game(int windowWidth, int windowHeight, const Gabinete::Configuracao& gabinete = {});
    ~Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsGameRunning = false; }

    //Actor functions
    void LoadInitialScene();

    //Texture
    class SDL_Texture* LoadTexture(const std::string& texturePath);

    /**
     * @brief A SILHUETA da imagem: o mesmo desenho, todo branco e opaco onde
     * ha pixel, transparente onde nao ha (Destrutivel::AlfaDaSilhueta). Tingida
     * com SDL_SetTextureColorMod, vira o contorno dos projeteis que o tiro do
     * aluno destroi. Guardada como as outras texturas, uma por imagem; nullptr
     * se a imagem nao abrir.
     */
    class SDL_Texture* LoadSilhueta(const std::string& texturePath);

    /**
     * @brief A textura guardada sob esta chave, ou nullptr.
     *
     * Para texturas que NAO vem de arquivo - a personagem composta e a primeira
     * delas. A chave nao e um caminho; ver Personagens::ChaveDaAparencia.
     */
    [[nodiscard]] class SDL_Texture* TexturaGuardada(const std::string& chave) const;

    /**
     * @brief Guarda uma textura ja pronta. O Game PASSA A SER DONO dela.
     *
     * Entra no mesmo cache das texturas de arquivo de proposito: assim ela e
     * destruida no mesmo lugar que todas as outras, sem regra de posse nova. Uma
     * chave ja ocupada e recusada, para nao vazar a textura que estava la.
     */
    void GuardarTextura(const std::string& chave, class SDL_Texture* textura);

    /**
     * @brief Destroi e esquece uma textura guardada.
     *
     * Existe para a tela de criacao de personagem, que compoe uma textura nova
     * a cada seta apertada. Sem isto, trocar de cabelo cem vezes deixaria cem
     * texturas vivas ate o jogo fechar - e numa maquina compartilhada, que fica
     * ligada o dia todo, isso se acumula aluno apos aluno.
     *
     * QUEM CHAMA GARANTE que ninguem mais aponta para ela.
     */
    void EsquecerTextura(const std::string& chave);

    /**
     * @brief Devolve (criando na primeira chamada) o xadrez magenta/preto usado
     * como marcador de textura ausente.
     *
     * E gerada em memoria de proposito: um placeholder que precisasse ser lido
     * do disco poderia falhar exatamente pelo mesmo motivo que a textura
     * original falhou.
     */
    SDL_Texture* GetPlaceholderTexture();

    //Camera functions
    Vector2& GetCameraPos() { return mCameraPos; }
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };
    class SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions
    [[nodiscard]] int GetWindowWidth() const { return mWindowWidth; }
    [[nodiscard]] int GetWindowHeight() const { return mWindowHeight; }

    //Audio System
    class AudioSystem *GetAudio() { return mAudio.get(); }

    //Scenes
    [[nodiscard]] Scene* GetScene() const { return mScene.get(); }
    void RequestSceneChange(Scene::SceneType nextScene);


    [[nodiscard]] Scene::SceneType GetCurrSceneType() const;

    IBossFactory* GetFactory(size_t n);

    // Funções pra retornar o estágio escolhido em StageSelect (retorna INF213 caso não tenha sido modificado,
    // porque é oq foi definido no construtor)
    [[nodiscard]] int GetSelectedStage() const { return mSelectedStage; }
    void SetSelectedStage(const int subject) { mSelectedStage = subject; }

    /**
     * @brief O orientador da proxima batalha: o indice da materia de que ele e
     * professor, ou -1 para batalha sem orientador.
     *
     * Fica aqui pelo mesmo motivo de mSelectedStage: quem escolhe e uma cena (a
     * Orientacao) e quem usa e outra (a Battle), e entre as duas o ChangeScene
     * so passa o que o Game guarda. A selecao de fases ZERA o orientador ao
     * entrar em qualquer batalha, entao ele vale so para o TCC em que foi
     * escolhido - nao sobra de uma partida para a seguinte.
     */
    void DefinirOrientador(const int materia) { mOrientador = materia; }
    [[nodiscard]] int Orientador() const { return mOrientador; }

    // ---- Notas -------------------------------------------------------------
    // O mapa unico de notas foi trocado por Progresso, que separa duas coisas
    // que antes eram a mesma: o RECORDE de cada materia e o PONTO DE RETOMADA
    // da proxima batalha. A nota da batalha em curso nao fica mais aqui - ela
    // vive em Battle::mGrade, que e onde ela muda.

    /// @brief A maior nota que o jogador ja tirou. Zero se nunca jogou.
    /// E o que a tela de selecao mostra e o que as regras de desbloqueio leem.
    /// A sobrecarga que existia aqui para receber um int cru sumiu junto com o
    /// enum: agora ela seria a mesma funcao.
    ///
    /// LE O PROGRESSO DO JOGADOR 1, como todas as consultas do mapa do curso: a
    /// materia que ele abriu e a que a dupla joga (requisitos-coop, RF3).
    [[nodiscard]] float GetMelhorNota(const int materia) const {
        return ProgressoDoMapa().MelhorNota(materia);
    }

    /**
     * @brief Como os rodapes nomeiam os botoes nesta sessao.
     *
     * No gabinete nao ha teclado, entao a tecla seria ruido; no computador ela
     * e o unico jeito de saber qual e o botao 2. A decisao fica AQUI, e nao em
     * cada cena, para nao haver uma tela que esqueca de perguntar.
     */
    [[nodiscard]] Painel::Jeito JeitoDoRodape() const {
        return mGabinete.arcade ? Painel::Jeito::SoOPainel : Painel::Jeito::ComATecla;
    }

    /// @brief Se o jogo abriu no gabinete (--arcade). Quem precisa saber e quem
    /// muda de comportamento la - a pausa da batalha, que no gabinete nao existe
    /// (Pausa.h).
    [[nodiscard]] bool NoGabinete() const { return mGabinete.arcade; }

    /**
     * @brief O modo de dificuldade das batalhas (Dificuldade.h).
     *
     * E DA MAQUINA, e nao do aluno: vem de Saves/opcoes.cfg ao abrir o jogo, e
     * a tela de Opcoes e quem o troca. A batalha o le ao carregar
     * (Battle::NivelDeDificuldade), e cada chefe escreve o proprio degrau com
     * Boss::NoNivel.
     */
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const { return mConfiguracao.dificuldade; }

    /// @brief Troca o modo e grava o arquivo de opcoes na hora. Se a gravacao
    /// falhar, o modo vale ate o jogo fechar (o motivo vai para o log).
    void DefinirDificuldade(Dificuldade::Nivel nivel);

    // ---- AS OUTRAS OPCOES DA MAQUINA (Configuracao.h) ----------------------
    //
    // Cada Definir aplica na hora - o som, a janela - e grava o arquivo, como
    // a dificuldade: sem um "salvar" que alguem esqueca de apertar.

    [[nodiscard]] int VolumeDaMusica() const { return mConfiguracao.volumeMusica; }
    [[nodiscard]] int VolumeDosEfeitos() const { return mConfiguracao.volumeEfeitos; }
    [[nodiscard]] bool TelaCheia() const { return mConfiguracao.telaCheia; }
    [[nodiscard]] bool MostrarHitbox() const { return mConfiguracao.mostrarHitbox; }

    void DefinirVolumeDaMusica(int volume);
    void DefinirVolumeDosEfeitos(int volume);
    void DefinirTelaCheia(bool cheia);
    void DefinirHitbox(bool mostrar);


    /// @brief Se a materia ja foi aprovada. A MESMA regra que o desbloqueio le,
    /// e nao uma comparacao com 60 repetida em quem desenha. Do jogador 1 (RF3).
    [[nodiscard]] bool Aprovado(const int materia) const {
        return ProgressoDoMapa().Aprovado(materia);
    }

    /// @brief A ultima nota que o jogador tirou nesta materia. NAO e por onde a
    /// batalha comeca - toda fase parte de Progresso::kNotaInicial.
    [[nodiscard]] float GetUltimaNota(const int materia) const {
        return ProgressoDoMapa().UltimaNota(materia);
    }

    /// @brief Registra o resultado de uma batalha: a retomada passa a ser esta
    /// nota, e o recorde so sobe. Chamado UMA vez, ao fim da batalha.
    ///
    /// E TAMBEM onde a ficha do aluno vai para o disco. Gravar aqui, e nao em quem
    /// chama, e o que garante que nenhum caminho de fim de batalha esqueca - era
    /// assim que a nota chegava ao armazenamento por efeito colateral antes de o
    /// Progresso existir, e nao quero repetir o padrao.
    void RegistrarNota(int subject, float nota);

    /**
     * @brief Passa a jogar como este aluno, carregando a ficha dele do disco.
     *
     * A matricula ja deve vir CANONICA (ver Matricula::Validar). Trocar de aluno
     * substitui o progresso inteiro: as notas do anterior nao podem ficar
     * penduradas na sessao do seguinte.
     *
     * Age sobre a posicao que esta entrando AGORA (ver PosicaoEntrando).
     */
    void IdentificarAluno(const std::string& matriculaCanonica);

    /**
     * @brief Passa a jogar sem se identificar.
     *
     * O progresso da sessao comeca zerado e NAO e gravado. Quem escolhe isto ve o
     * aviso na tela antes, nao depois.
     */
    void JogarComoVisitante();

    // ---- A entrada, de um ou de dois ---------------------------------------

    /// @brief Quantos jogam esta partida. So o menu principal decide.
    void DefinirModoDeJogo(Sessao::Modo modo);
    [[nodiscard]] Sessao::Modo ModoDeJogo() const { return mModoDeJogo; }
    [[nodiscard]] size_t QuantosJogadores() const { return mSessoes.size(); }

    /// @brief Qual posicao esta na tela de matricula ou de personagem agora.
    [[nodiscard]] size_t PosicaoEntrando() const { return mPosicaoEntrando; }

    /// @brief As sessoes, para quem precisa olhar todas (ver Sessao::JaEstaEmUso).
    [[nodiscard]] const std::vector<Sessao::Dados>& Sessoes() const { return mSessoes; }

    /**
     * @brief A posicao atual terminou de entrar: chama a proxima ou vai jogar.
     *
     * A REGRA MORA AQUI, e nao nas duas telas que terminam a entrada. A
     * matricula e a criacao de personagem mandavam as duas para a selecao de
     * fases, cada uma com a sua linha; com dois jogadores as duas precisariam da
     * mesma condicao nova, e bastaria uma esquecer para o jogador 2 nunca ser
     * chamado. Quem decide o que e "a proxima" e Sessao::Proximo, que e puro.
     *
     * Ja pede a troca de cena - quem chama so precisa dar return depois.
     */
    void AvancarEntrada();

    /// @brief Desiste da entrada em curso e volta a ser uma partida de um.
    ///
    /// Quem volta da tela de matricula do jogador 2 estaria deixando uma partida
    /// de dois pela metade; largar o modo ligado faria a proxima entrada pedir
    /// duas matriculas sem que ninguem tivesse escolhido isso.
    void CancelarEntrada();

    /**
     * @brief O que a proxima tela de ranking deve mostrar, e para onde ela volta.
     *
     * Fica no Game pelo mesmo motivo de mSelectedStage: quem
     * cria as cenas e o ChangeScene, que nao conhece a intencao de quem pediu a
     * troca. Sem isto, a tela nao teria como saber se foi aberta pelo menu (media
     * do curso, volta para o menu) ou pela selecao de fases (uma materia so,
     * volta para a selecao).
     */
    struct PedidoDeRanking {
        bool geral = true;                                 ///< media do curso, e nao uma materia
        int  materia = 0;                                  ///< so vale quando geral e falso
        Scene::SceneType voltarPara = Scene::SceneType::MainMenu;
    };

    void PedirRanking(const PedidoDeRanking& pedido) { mPedidoDeRanking = pedido; }
    [[nodiscard]] const PedidoDeRanking& RankingPedido() const { return mPedidoDeRanking; }

    /// @brief A matricula de uma posicao, ou Matricula::kVisitante.
    [[nodiscard]] const std::string& MatriculaDe(size_t posicao) const;

    /// @brief A aparencia de uma posicao. Vazia quer dizer "a padrao".
    [[nodiscard]] const Personagens::Aparencia& AparenciaDe(size_t posicao) const;

    /**
     * @brief Troca a aparencia e grava, se o aluno estiver identificado.
     *
     * Grava na hora, e nao no fim da batalha: quem acaba de montar a personagem
     * espera encontra-la do mesmo jeito ao carregar o perfil, mesmo que feche o
     * jogo sem jogar nada.
     */
    void DefinirAparencia(const Personagens::Aparencia& aparencia);

    /// @brief Se o progresso desta posicao vai para o disco. Visitante nao grava.
    [[nodiscard]] bool ProgressoEGravado(size_t posicao) const;

    // Verifica se uma matéria específica está desbloqueada para jogar
    bool IsStageUnlocked(int subject);

private:
    /// Passam as opcoes de som e de janela para o AudioSystem e o SDL.
    void AplicarVolumes();
    void AplicarTelaCheia();

    /**
     * @brief O progresso que o MAPA DO CURSO le: o do jogador 1.
     *
     * A selecao de fases e uma so, e o desbloqueio e o dele - a materia que ele
     * abriu e a que a dupla joga (requisitos-coop, RF3). Existe como funcao, e
     * nao como mSessoes[0].progresso espalhado, para que esta decisao esteja
     * escrita num lugar e nao quinze vezes implicita.
     */
    [[nodiscard]] const Progresso& ProgressoDoMapa() const { return mSessoes.front().progresso; }

    /// A sessao da posicao que esta na tela de matricula ou de personagem.
    [[nodiscard]] Sessao::Dados& SessaoEntrando();
    [[nodiscard]] const Sessao::Dados& SessaoEntrando() const;

    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // SDL stuff
    class SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    std::unique_ptr<AudioSystem> mAudio;

    // Textura "faltando": xadrez magenta/preto gerado em memoria, devolvido
    // sempre que um arquivo de imagem nao carrega. E compartilhada por todos os
    // caminhos que falharam, entao NAO pode ser destruida junto com o cache.
    SDL_Texture* mPlaceholderTexture = nullptr;

    // Cache de texturas: caminho do arquivo -> textura ja carregada na GPU.
    // O Game e o DONO de todas elas e as destroi em Shutdown(). Nenhum
    // DrawComponent deve chamar SDL_DestroyTexture na textura que recebeu.
    std::map<std::string, SDL_Texture*> mTextureCache;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if game is running
    bool mIsGameRunning;

    // Camera X and Y position
    Vector2 mCameraPos;

    //Active scenes
    std::unique_ptr<Scene> mScene;

    //All Boss factories
    /// As fabricas de chefe, pelo NOME com que materias.json as chama.
    std::map<std::string, std::unique_ptr<IBossFactory>> mBossFactory;

    /**
     * @brief QUEM ESTA JOGANDO, uma entrada por posicao do painel.
     *
     * Eram tres membros soltos - mMatricula, mProgresso e mAparencia -, todos no
     * singular. Com duas posicoes eles virariam tres vetores soltos, e tres
     * vetores soltos sao o comeco de um estado inconsistente: basta um caminho
     * acrescentar em dois e esquecer o terceiro. Ver Sessao.h.
     *
     * NUNCA ESTA VAZIO. A posicao 0 existe desde o construtor, como visitante,
     * entao abrir o jogo e sair sem passar pela identificacao nunca grava nada
     * por engano - e nenhum leitor precisa checar o tamanho antes de ler.
     */
    std::vector<Sessao::Dados> mSessoes{1};

    /// Quantos jogam esta partida. Posto pelo menu e so por ele.
    Sessao::Modo mModoDeJogo = Sessao::Modo::Um;

    /// Qual posicao esta passando pela matricula/criacao AGORA. Ver
    /// AvancarEntrada - e so durante a entrada que este numero quer dizer algo.
    size_t mPosicaoEntrando = 0;

    PedidoDeRanking mPedidoDeRanking{};

    //Selected Stage, used by Battle on ChangeScene()
    int mSelectedStage{};

    /// Ver DefinirOrientador.
    int mOrientador = -1;

    /// As opcoes da maquina (Configuracao.h). Lidas do disco em
    /// LoadInitialScene, depois que a pasta base e descoberta.
    Configuracao::Dados mConfiguracao{};

    bool mPendingSceneChange;
    Scene::SceneType mNextScene;

    // ---- Gabinete ----------------------------------------------------------
    // Ver Gabinete.h. As duas esperas sao a MESMA classe com a condicao
    // trocada, e o limite zero as desliga: por isso nao ha "if (arcade)" dentro
    // de AtualizarGabinete - fora do gabinete elas nascem desligadas.
    //
    // DECLARADAS DEPOIS de mNextScene porque e nessa ordem que o construtor as
    // inicializa, e o compilador avisa quando as duas listas divergem.

    Gabinete::Configuracao mGabinete;

    /// Tempo sem ninguem mexer. Esgotando, volta ao menu.
    Gabinete::Contagem mOciosidade;

    /// Tempo com a combinacao de saida segurada. Esgotando, fecha o jogo.
    Gabinete::Contagem mSaidaDoOperador;

    /// Lidos em ProcessInput e consumidos em UpdateGame, que e onde existe
    /// deltaTime.
    bool mHouveEntrada = false;
    bool mSaidaSegurada = false;

    /// A volta ao menu e a saida do operador, quadro a quadro. A tela cheia
    /// nao esta aqui: ela acontece uma vez so, em Initialize.
    void AtualizarGabinete(float deltaTime);


    /// Monta a ficha de UMA posicao e grava. O unico lugar que faz isso, para
    /// nenhum caminho de gravacao esquecer um campo.
    bool GravarFicha(size_t posicao);

    void InitializeBossFactory();

    void ChangeScene(Scene::SceneType sceneType);

    bool HasPassed(int subject);

    int CountPassedInList(const std::vector<int>& subjects);
};
