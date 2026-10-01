//
// Montagem e busca de caminhos de arquivo.
//

#pragma once

#include <functional>
#include <string>

/**
 * CAMADA PURA (mesma disciplina de PathAim, PathShapes, RegrasDeAtaque,
 * FasesDeAtaque e Progresso)
 *
 * Nao toca em disco, nao conhece SDL. So manipula texto de caminho e implementa
 * a BUSCA pela pasta base - recebendo de fora a pergunta "esta pasta serve?".
 * Quem responde essa pergunta olhando o disco de verdade e CaminhosArquivo.cpp.
 *
 * POR QUE ISSO EXISTE
 *
 * O jogo tinha 42 caminhos escritos a mao na forma "../Assets/...". Isso amarra
 * a execucao a UM diretorio: o jogo so roda se for lancado de exatamente um nivel
 * abaixo da pasta que contem Assets. Na maquina de Arcade, onde o executavel e
 * lancado de onde o sistema quiser, esses caminhos simplesmente nao resolvem.
 *
 * Ler do lugar errado da sprite faltando, que se ve na hora. O sistema de
 * matricula vai ESCREVER em disco, e gravar no lugar errado perde nota de aluno
 * em silencio - por isso a base precisa estar resolvida antes daquilo existir.
 */
namespace Caminhos {

    /**
     * @brief Junta uma base e um caminho relativo com UMA barra entre eles.
     *
     * Normaliza '\' para '/'. As duas convivem no Windows, e um caminho com as
     * duas misturadas atravessa a API do sistema mas fica ilegivel no log - que e
     * onde a gente vai olhar quando algo nao abrir.
     *
     * Base vazia devolve o relativo, e vice-versa: assim quem chama nao precisa
     * tratar o caso de a base ainda nao ter sido descoberta.
     */
    std::string Juntar(const std::string& base, const std::string& relativo);

    /**
     * @brief O diretorio um nivel acima. Devolve vazio quando nao ha acima.
     *
     * Ignora barra sobrando no fim: "C:/jogo/build/" e "C:/jogo/build" sobem os
     * dois para "C:/jogo".
     */
    std::string DiretorioPai(const std::string& caminho);

    /// Responde se um diretorio candidato e a base procurada. Injetada de fora
    /// para que a busca possa ser testada sem disco nenhum.
    using Predicado = std::function<bool(const std::string&)>;

    /**
     * @brief Sobe a partir de 'partida' ate achar um diretorio que sirva. PURA.
     *
     * Testa a propria 'partida' primeiro, depois o pai, e assim por diante, no
     * maximo 'maxNiveis' vezes. Devolve o primeiro que serve, ou vazio.
     *
     * Sobe em vez de procurar em lugares fixos porque a pasta de build varia
     * demais entre ambientes - cmake-build-debug no CLion, build no terminal,
     * Debug/Release no Visual Studio - e todas elas tem a raiz do projeto acima.
     */
    std::string ProcurarSubindo(const std::string& partida, int maxNiveis,
                               const Predicado& serve);

}
