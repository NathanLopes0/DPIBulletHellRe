//
// A fase 1 do Michel: a visao computacional - como a imagem se forma.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A fase 1 do Michel e a primeira aula de visao: a imagem e luz que passa por um furo. No meio do
/// campo fica a camera escura; o professor arremessa os objetos da cena na metade de cima - eles nao
/// machucam ninguem -, e o perigo e a IMAGEM de cada um, do outro lado do furo, de cabeca para baixo:
///   - na camera, o que cai na cena sobe na imagem, e o que vai para a direita vai para a esquerda. A
///     imagem do proprio professor esta embaixo, invertida, e e de la que os tiros saem;
///   - no foco, o aumento oscila (a imagem cresce e encolhe) e o furo anda de lado;
///   - no desfoque, o que esta fora do plano de foco vira um anel (o circulo de confusao). O foco
///     automatico persegue a altura do aluno: perto dele, a imagem e um ponto - nitida e certeira.
/// Quem joga olha para cima para saber o que vem de baixo.
namespace CameraEscura {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 3> kEtapas = {"Camera", "Foco", "Desfoque"};
    constexpr std::array<float, 3> kDuracoes = {7.0f, 6.0f, 7.0f};
    constexpr const char* kGatilhoDoObjeto = "objeto";
    constexpr const char* kGatilhoDaImagem = "imagem";
    constexpr const char* kGatilhoDoEnfeite = "enfeite";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A optica ----

    /// A imagem de 'objeto' pelo furo: do outro lado, de cabeca para baixo e 'aumento' vezes mais
    /// longe do furo.
    Vector2 Imagem(Vector2 objeto, Vector2 furo, float aumento);
    /// O caminho de volta: o ponto da cena cuja imagem cai em 'imagem'. E para la que o professor
    /// arremessa, para a imagem passar pelo aluno.
    Vector2 ObjetoDa(Vector2 imagem, Vector2 furo, float aumento);

    /// O aumento no instante 't' da etapa do foco: vai de 'menor' a 'maior' e volta em 'periodo'.
    float AumentoNoTempo(float t, float periodo, float menor, float maior);

    /// O raio do circulo de confusao de uma imagem na altura 'y', com o plano de foco em 'foco'.
    float RaioDoBorrao(float y, float foco, float ganho, float maximo);
    /// O foco automatico: anda de 'foco' ate 'alvo' com 'rapidez' por segundo (exponencial).
    float Focar(float foco, float alvo, float dt, float rapidez);
    /// As 'amostras' pontas do anel do borrao em volta de 'centro', a partir do angulo 'giro'
    /// (radianos). Com raio zero, todas caem no centro.
    std::vector<Vector2> Borrao(Vector2 centro, float raio, int amostras, float giro);

    /// O objeto some quando chega perto do furo (entra na camera): a 'margem' da altura dele.
    constexpr float kMargemDoFuro = 26.0f;

    // ---- O ritmo ----

    struct Degrau {
        float entreArremessos = 0.62f;
        int leque = 3;                   ///< objetos por arremesso, em leque
        float aberturaDoLeque = 26.0f;   ///< graus, de ponta a ponta
        float velocidadeDoObjeto = 115.0f;
        float aumentoMenor = 0.75f;
        float aumentoMaior = 1.45f;
        float periodoDoFoco = 3.4f;
        float passeioDoFuro = 0.22f;     ///< o furo anda de lado no foco: fracao da largura
        float rapidezDoFoco = 1.3f;
        int amostrasDoBorrao = 6;
        float ganhoDoBorrao = 0.3f;
        float borraoMaximo = 62.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
