//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../Scene.h"
#include "../../Regras/Intercambio.h"
#include "../../Regras/Papeis.h"
#include "../../Regras/Cobertura.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/ContaDaNota.h"
#include "../../Aluno/ProvaDaSemana.h"
#include "../../Pesquisa/Experimento.h"
#include "../../Motor/Game.h"
#include "../../Motor/AudioSystem.h"
#include "../../Entrada/Borda.h"
#include "../../Interface/Devolutiva.h"
#include "../../Regras/Dificuldade.h"
#include "../../Interface/Hud.h"
#include "../../Interface/Comunicacao.h"
#include "../../Regras/Variacao.h"
#include "Colisoes.h"
#include "OuvinteDaBatalha.h"
#include "../../Regras/Ajuste.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"

class Font;
class Player;
class ProjectileManager;
class BattleHUD;
class FaixaDaNota;
class MolduraDoCampo;
class PlacarDaFase;
class RelogioDaFase;
class EfeitosDaBatalha;
class DesenhoSobreOsAlunos;
class ExtraPointItem;
class Orientador;
class PausaDaBatalha;
class TelaDoResultado;
class DevolutivaDaBatalha;
class BossProjectile;
class PlayerProjectile;

/// A cena da luta. Ela orquestra o quadro e guarda o estado da prova; quem conta a nota, mostra a
/// faixa, os menus e os efeitos, e acha as colisoes, sao os colaboradores dela (FaixaDaNota,
/// PausaDaBatalha, TelaDoResultado, EfeitosDaBatalha, DesenhoSobreOsAlunos, Colisoes). O que as
/// colisoes acham, ela paga e cobra (Colisoes::Regras).
class Battle : public Scene, private Colisoes::Regras {

public:

    explicit Battle(Game* game, int selectedStage);
    ~Battle() override;
    void Load() override;
    void VerificarColisoes();
    /// Uma fase do chefe acabou e outra comecou. Chamado pelo callback que o Boss registra na
    /// maquina de estados.
    void AoMudarDeFase(float novaDuracao);

    [[nodiscard]] Boss* Chefe() const { return mChefe; }

    /// O orientador do TCC, quando ha um em campo.
    [[nodiscard]] Orientador* OrientadorEmCampo() const { return mOrientador; }

    /// Paga uma conquista da dupla: um ganho bruto, inteiro (nao passa por Nota::PorJogador - ver
    /// Nota::Conta::Subir).
    void Conquistar(const float bruto) { if (EmCombate()) mConta.Subir(bruto); }

    /// A transacao do Thiago (Poder.h): o ponto de volta da nota, e o fim - ROLLBACK se ela caiu
    /// (devolve true), COMMIT se nao.
    [[nodiscard]] Nota::Conta::Marca MarcarNota() const { return mConta.Marcar(); }
    bool DesfazerNota(const Nota::Conta::Marca& marca) { return mConta.Desfazer(marca); }

    /// O que tem de se resolver antes de a nota decidir a luta (a transacao aberta): 'fechar' e
    /// chamado uma vez, no fim da fase que decide - ou nunca, se quem deixou a pendencia a tirar
    /// antes (com nulo).
    void DeixarPendencia(std::function<void()> fechar) { mPendencia = std::move(fechar); }
    void FecharPendencias();

    /// O try/catch do Luizinho (Poder.h): por 'segundos', o acerto que um aluno levaria nao tira
    /// nota - vira uma excecao capturada, e 'aoCapturar' fica sabendo de quem.
    void CapturarAcertos(float segundos, std::function<void(Player&)> aoCapturar);

    /// Se a luta esta valendo: nem carregando os projeteis, nem na tela de fim.
    /// Quem age por conta propria (o orientador) so age enquanto isto vale.
    [[nodiscard]] bool EmCombate() const { return !mCarregando && !mAcabando; }

    /// Todos os corpos do chefe em campo, e nao so o original. Normalmente e um so.
    [[nodiscard]] const std::vector<Boss*>& Corpos() const { return mCorpos; }

    /// O modo desta batalha (Dificuldade.h).
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const { return mNivel; }

    /// A semente desta luta (Variacao.h): de onde cada chefe tira o que muda de uma batalha para a
    /// outra dentro de cada fase.
    [[nodiscard]] std::uint32_t SementeDaBatalha() const { return mSemente; }
    /// O ajuste automatico de dificuldade desta prova (Ajuste.h): o degrau, e o que ele muda.
    [[nodiscard]] const Ajuste::Degrau& AjusteDaProva() const { return mAjuste; }

    /// Poe um segundo corpo em campo, com os ataques da fase nomeada. Chamada pelo chefe ao entrar
    /// numa fase que declara "clone" em fases.json.
    void DividirChefe(const std::string& faseDoClone);

    /// Tira de campo os corpos extras, deixando so o original.
    void JuntarCorpos();

    /// Os jogadores em campo, na ordem das posicoes do painel.
    [[nodiscard]] const std::vector<Player*>& Jogadores() const { return mJogadores; }

    /// Em quem um ataque mirado mira.
    [[nodiscard]] Player* Alvo() const;

    /// Em qual posicao esta a vez: a de quem mais incomodou o professor (Papeis.h) ou, se os dois
    /// incomodam parecido, a do revezamento. A vez dada (DarAVez) vale para o proximo disparo.
    [[nodiscard]] size_t IndiceDoAlvo() const { return mMira.Alvo(mJogadores.size()); }

    /// Em quem a mira esta presa (o marcador vermelho sobre o aluno); -1 quando os dois revezam.
    [[nodiscard]] int FocoDaMira() const { return mMira.Foco(mJogadores.size()); }

    /// O jogador de uma posicao, ou nullptr fora da lista. E como um projetil
    /// resolve o numero que guardou.
    [[nodiscard]] Player* JogadorEm(size_t posicao) const;

    /// Passa a vez para o proximo jogador. Chamado uma vez por disparo, em Boss::ExecuteAttack - o
    /// unico lugar por onde todo ataque de todo chefe passa.
    void RevezarAlvo();

    /// Da a vez a um jogador, antes de um disparo que e dele - mesmo que a mira esteja no outro.
    void DarAVez(size_t posicao);
    [[nodiscard]] ProjectileManager* GerenteDeProjeteis() const { return mProjeteis.get(); }

    /// A fonte do placar (Carlito), para quem escreve codigo no campo (EtiquetaDeCodigo). E da
    /// batalha, que vive mais que os atores dela.
    [[nodiscard]] Font* FonteDoPlacar() const { return mFonteDoPlacar.get(); }

    SDL_FRect GetPlayfieldBounds() const override;
    /// Um aluno gastou um Ponto Extra. Recebe quem gastou porque em dupla o efeito e no outro: a
    /// tela nao e limpa, o parceiro fica imune.
    void AoGastarPontoExtra(Player* quemGastou);

    /// Segundos desde que o painel do resultado apareceu (zero antes dele).
    [[nodiscard]] float TempoNoResultado() const;

    /// Na dupla, fora do TCC (la o Botao 3 e o poder do orientador), o aluno da um sinal ao
    /// parceiro: um anel na cor dele e um som curto (Comunicacao.h).
    void Sinalizar(const Player& quem);

    /// O outro aluno em campo, ou nullptr se este estiver sozinho.
    [[nodiscard]] Player* OutroJogador(const Player* quem) const;
    void SoltarPontoExtra(Vector2 position);

    void EncerrarBatalha(bool aprovado);

    /// A nota da batalha em curso. Quem precisa dela e a maquina de estados do chefe, para decidir
    /// entre a repescagem e o fim.
    [[nodiscard]] float NotaAtual() const { return mConta.Valor(); }
    /// Quantas vezes a dupla foi acertada nesta batalha.
    [[nodiscard]] int AcertosSofridos() const { return mConta.AcertosSofridos(); }
    /// O indice da materia desta batalha (Materias::Lista).
    [[nodiscard]] int MateriaDaBatalha() const { return mMateria; }

    /// A janela perdeu o foco: a luta pausa sozinha (Pausa::PausaAoPerderOFoco).
    void AoPerderOFoco() override;

protected:
    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

private:
    // ---- As colisoes (Colisoes.h): elas acham, a batalha paga e cobra ----
    std::unique_ptr<Colisoes> mColisoes;
    void AlunoAcertado(Player& aluno, const std::string& causa, float multiplicador, bool porUmTiro) override;
    void AlunoRaspou(Player& aluno, const Vector2& onde) override;
    /// Se o projetil ia no parceiro de quem o derrubou, e uma cobertura (Cobertura.h): paga inteira, com
    /// a faisca dela e o valeu. Devolve se foi.
    bool ProjetilDerrubado(const PlayerProjectile& tiro, const BossProjectile& projetil) override;
    void ProfessorAcertado(const PlayerProjectile& tiro, const CircleColliderComponent& corpo, bool ouviu) override;
    void PontoExtraPego(Player& aluno, bool primeiroDoQuadro) override;

    // ---- Funções Auxiliares de Lógica Interna ----
    void CriarHUD();
    void CriarJogadores();
    void CriarChefe();

    /// O orientador, se a materia pede um e alguem foi escolhido. Quem escolhe e a
    /// EscolhaDoOrientador, e o escolhido chega pelo Game (Game::Orientador).
    void CriarOrientador();



    // ---- O placar: o que o chefe anuncia na fase ("anuncio" em fases.json) ----
    /// O placar diz o anuncio da fase, ou o aviso do intercambio enquanto ele esta aceso.
    void AtualizarPlacar();

    // ---- O intercambio: os alunos trocam de lugar ("intercambio" em fases.json) ----
    void CriarIntercambio();
    void AtualizarIntercambio(float deltaTime);

    // ---- Membros de Estado da Batalha ----

    /// A nota, os acertos sofridos e as fases limpas: quem paga e cobra passa por aqui.
    Nota::Conta mConta;
    int mMateria{};

    /// Fecha a fase em curso (Nota::Conta::FecharFase) e, se ela foi limpa, avisa.
    void EncerrarFase();

    /// Quanto tempo o aviso de fase limpa ainda fica na tela.
    float mAvisoDeFaseLimpa{};

    Dificuldade::Nivel mNivel = Dificuldade::Nivel::Normal;

    std::uint32_t mSemente = 0;
    Ajuste::Degrau mAjuste;

    // ---- Quem ouve a batalha (OuvinteDaBatalha) ----

    std::vector<std::unique_ptr<OuvinteDaBatalha>> mOuvintes;

    /// Anuncia a todos os ouvintes, na ordem em que entraram:
    /// Avisar([&](OuvinteDaBatalha& o) { o.AoLimparFase(*this); }).
    template <typename Aviso>
    void Avisar(Aviso&& aviso) {
        for (const auto& o : mOuvintes) aviso(*o);
    }

    // ---- O que se desenha na luta ----

    /// As faiscas, as ondas do Ponto Extra e as moedas (EfeitosDaBatalha.h).
    std::unique_ptr<EfeitosDaBatalha> mEfeitos;
    /// Por cima dos alunos: o feixe e o escudo do Ponto Extra, o sinal, o valeu, a mira presa e a
    /// hitbox (DesenhoSobreOsAlunos.h).
    std::unique_ptr<DesenhoSobreOsAlunos> mDesenhoSobreOsAlunos;

    /// Quando cada aluno deu o ultimo sinal (mRelogioDaLuta): a espera entre um e outro.
    std::array<float, 2> mUltimoSinal{Comunicacao::kNuncaSinalizou, Comunicacao::kNuncaSinalizou};

    // ---- A HUD do campo ----

    /// O fundo, os trilhos e o "prof" (MolduraDoCampo.h).
    std::unique_ptr<MolduraDoCampo> mMoldura;
    /// A capsula do tempo da fase e o reloginho, ao lado do professor (RelogioDaFase.h).
    std::unique_ptr<RelogioDaFase> mRelogio;
    /// O quadro com o anuncio da fase (PlacarDaFase.h).
    std::unique_ptr<PlacarDaFase> mPlacar;

    std::function<void()> mPendencia;

    /// O try do Luizinho: quanto falta dele, e quem fica sabendo de cada captura.
    float mCapturando = 0.0f;
    std::function<void(Player&)> mAoCapturar;

    /// O aluno para as contas da cobertura: onde ele esta, o colisor e se algo o atinge.
    [[nodiscard]] Cobertura::Aluno AlunoDaCobertura(const Player* aluno) const;

    /// Na dupla, o projetil destrutivel que vai num aluno brilha na cor dele (Cobertura::AlunoNoCaminho):
    /// quem esta embaixo sabe que derruba-lo e a cobertura. E a mesma conta de ProjetilDerrubado.
    void MarcarOCaminhoDosAlunos();

    /// A posicao (0 ou 1) do aluno 'ator', ou -1 se ele nao e um aluno.
    [[nodiscard]] int PosicaoDoAluno(const Actor* ator) const;

    /// Segundos de luta: sem carregamento, sem pausa e sem o resultado.
    float mRelogioDaLuta = 0.0f;
    /// Na dupla, quando cada aluno gastou o Ponto Extra pela ultima vez (mRelogioDaLuta): dois gastos
    /// em PontoExtra::kJanelaDaSincronia sao a sincronia.
    std::array<float, 2> mUltimoPontoExtra{PontoExtra::kNuncaGastou, PontoExtra::kNuncaGastou};

    // ---- Ponteiros observadores do Boss e do Player, que vão estar dentro ----
    Boss* mChefe{};

    /// O original mais os clones vivos.
    std::vector<Boss*> mCorpos;

    /// Os jogadores, em vez de um.
    std::vector<Player*> mJogadores;

    /// O orientador do TCC, ou nulo. O ator e da cena, como todos.
    Orientador* mOrientador = nullptr;

    /// Em quem os ataques mirados vao: o incomodo, o revezamento e a vez dada (Papeis.h).
    Papeis::Mira mMira;
    /// As mecanicas que esta execucao desliga (Experimento.h), lidas ao criar a batalha.
    Experimento::Condicao mCondicao;

    // O gerente de projéteis: Um sistema especializado, possuído pela cena.
    std::unique_ptr<ProjectileManager> mProjeteis;

    // O HUD.
    std::unique_ptr<BattleHUD> mHUD;
    /// A fonte da faixa da nota, que e tambem a dos menus e da tela de carregamento.
    std::unique_ptr<Font> mFonteDaNota;
    /// A faixa da nota, na base da tela (FaixaDaNota.h).
    std::unique_ptr<FaixaDaNota> mFaixa;

    /// A Carlito tem a notacao inteira, inclusive os sobrescritos e o delta: a do placar, do "prof"
    /// e da tela de carregamento.
    std::unique_ptr<Font> mFonteDoPlacar;

    /// O relogio do Intercambio da fase atual. Sem Intercambio ele fica
    /// invalido, e um relogio invalido nunca avisa nem troca (Intercambio.h).
    Intercambio::Relogio mRelogioDoIntercambio;

    /// A fase de que o relogio e. Trocar de fase zera o relogio: o periodo
    /// conta do comeco de cada fase, e nao da batalha.
    std::string mFaseDoIntercambio;

    /// Um marcador por aluno, no lugar onde ele vai cair. So aparecem durante
    /// o aviso, e andam junto com o destino - que muda quando os alunos andam.
    std::vector<Actor*> mMarcadoresDoIntercambio;

    // O que o placar diz enquanto o aviso esta aceso e do chefe: ver
    // Boss::AvisoDoIntercambio.

    /// Logo acima dos projeteis (88 a 92) e abaixo do aluno (100).
    static constexpr int kOrdemDoMarcador = 95;


    std::vector<ExtraPointItem*> mPontosExtras;

    // ---- O fim da luta ----

    bool mAcabando = false;
    /// O painel do resultado, com o toca aqui (TelaDoResultado.h): criado no fim, quando se sabe o
    /// resumo.
    std::unique_ptr<TelaDoResultado> mResultado;

    /// A devolutiva (onde a nota caiu e por que) e a justica da dupla (o que cada aluno fez): um
    /// ouvinte da batalha, que esta guarda em mOuvintes (DevolutivaDaBatalha.h).
    DevolutivaDaBatalha* mDevolutiva = nullptr;
    /// O que as horas complementares perguntam da prova (Horas.h).
    class ContagemDaProva* mContagem = nullptr;

    /// Esta luta e a prova da semana (ProvaDaSemana.h): a materia dela, na semana dela - a semente e
    /// a mesma para todo mundo, e a nota entra no ranking da semana. Nunca na simulacao.
    bool mProvaDaSemana = false;
    ProvaDaSemana::Semana mSemanaDaProva;
    /// Poe a nota no ranking da semana e devolve o lugar de quem jogou ("2o de 5"); vazio para o
    /// visitante. 'posicao' sai com o lugar, ou zero.
    std::string EntrarNoRankingDaSemana(int& posicao);

    SoundHandle mMusica;

    // ---- Tela de Carregamento (pré-aquecimento dos pools de projétil) ----
    bool mCarregando = false;
    std::vector<std::string> mTiposACarregar; // nomes ainda não pré-aquecidos
    int mTiposNoTotal = 0;   // quantos tipos existem no total (para a barra de progresso)

    // A tela de carregamento cobre a batalha inteira - um veu escuro com o nome da materia e uma
    // barra enchendo.
    std::unique_ptr<class TelaDeCarregamento> mTelaDeCarregamento;

    void ComecarCarregamento();
    void AvancarCarregamento(float deltaTime); // processa 1 tipo de projétil por chamada
    void TerminarCarregamento();

    // ---- A pausa, so no computador (PausaDaBatalha.h) ----

    std::unique_ptr<PausaDaBatalha> mPausa;
    /// Abre o menu, congela os atores e para o aluno e a musica.
    void AbrirPausa();
    /// A contagem acabou: os atores e a musica voltam.
    void Retomar();

    /// Sai da batalha no meio, para 'destino', sem registrar a nota.
    void SairDaBatalha(SceneType destino);

    /// O ESC e borda, e nao estado: segurado, ele abriria e fecharia a pausa a cada quadro.
    Borda mBordaEsc{true};
};

#endif // DPIBULLETHELLRE_BATTLE_H.
