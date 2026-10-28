//
// A fase 1 do Lucas: uma pagina sendo renderizada no navegador.
//

#include "Pagina.h"

#include <algorithm>
#include <set>

namespace Pagina {

    std::array<Retangulo, kRegioes> Layout(const float largura) {
        const float util = largura - 2.0f * kMargem;
        const float coluna = (util - 2.0f * kEntreRegioes) / 4.0f;   // nav e aside 1, main 2
        std::array<Retangulo, kRegioes> r;
        r[static_cast<size_t>(Regiao::Header)] = {kMargem, kYDoHeader, util, kAlturaDoHeader};
        r[static_cast<size_t>(Regiao::Nav)] = {kMargem, kYDoCorpo, coluna, kAlturaDoCorpo};
        r[static_cast<size_t>(Regiao::Main)] = {kMargem + coluna + kEntreRegioes, kYDoCorpo, 2.0f * coluna, kAlturaDoCorpo};
        r[static_cast<size_t>(Regiao::Aside)] = {kMargem + 3.0f * coluna + 2.0f * kEntreRegioes, kYDoCorpo, coluna, kAlturaDoCorpo};
        return r;
    }

    std::string NomeDaRegiao(const Regiao r) {
        switch (r) {
            case Regiao::Header: return "header";
            case Regiao::Nav:    return "nav";
            case Regiao::Main:   return "main";
            case Regiao::Aside:  return "aside";
        }
        return "body";
    }

    Retangulo Tamanho(const Tipo t, const Retangulo& regiao) {
        const float cabe = regiao.w - 20.0f;
        switch (t) {
            case Tipo::Botao:   return {0, 0, std::min(130.0f, cabe), 40.0f};
            case Tipo::Imagem:  return {0, 0, std::min(150.0f, cabe), 96.0f};
            case Tipo::Marquee: return {0, 0, cabe, 36.0f};
            case Tipo::Tabela:  return {0, 0, std::min(200.0f, cabe), 126.0f};
            case Tipo::Link:    return {0, 0, std::min(120.0f, cabe), 30.0f};
            case Tipo::Input:   return {0, 0, std::min(220.0f, cabe), 38.0f};
        }
        return {};
    }

    namespace {
        std::string Atributo(Variacao::Sorteio& s, const Tipo t, const Regiao ancora) {
            static const std::vector<std::string> kCliques = {"enviar()", "comprar()", "curtir()", "salvar()"};
            static const std::vector<std::string> kImagens = {"gato.png", "logo.svg", "meme.gif", "foto.jpg"};
            static const std::vector<std::string> kEntradas = {"text", "email", "password", "search"};
            switch (t) {
                case Tipo::Botao:  return "onclick=\"" + s.Um(kCliques) + "\"";
                case Tipo::Imagem: return "src=\"" + s.Um(kImagens) + "\"";
                case Tipo::Input:  return "type=\"" + s.Um(kEntradas) + "\"";
                case Tipo::Link:   return "href=\"#" + NomeDaRegiao(ancora) + "\"";
                default:           return "";
            }
        }

        /// Uma tentativa de pagina: cada regiao com os tipos que cabem nela.
        std::vector<Elemento> Tentar(Variacao::Sorteio& s, const float largura) {
            const auto layout = Layout(largura);
            struct Doque { Regiao regiao; int de; int ate; std::vector<Tipo> tipos; };
            const std::vector<Doque> plano = {
                {Regiao::Nav, 2, 3, {Tipo::Link, Tipo::Link, Tipo::Botao, Tipo::Input}},
                {Regiao::Main, 2, 3, {Tipo::Imagem, Tipo::Tabela, Tipo::Marquee, Tipo::Input, Tipo::Botao}},
                {Regiao::Aside, 1, 2, {Tipo::Imagem, Tipo::Botao, Tipo::Link}},
            };
            std::vector<Elemento> pagina;
            for (const Doque& d : plano) {
                const Retangulo& regiao = layout[static_cast<size_t>(d.regiao)];
                float y = regiao.y + 12.0f;
                const int quantos = s.Inteiro(d.de, d.ate);
                for (int k = 0; k < quantos; ++k) {
                    Elemento e;
                    e.tipo = s.Um(d.tipos);
                    e.regiao = d.regiao;
                    Retangulo caixa = Tamanho(e.tipo, regiao);
                    if (y + caixa.h > regiao.y + regiao.h - 8.0f) break;   // nao cabe: fica de fora
                    caixa.x = regiao.x + 10.0f;
                    caixa.y = y;
                    y += caixa.h + kEntreElementos;
                    e.caixa = caixa;
                    if (e.tipo == Tipo::Link) {
                        std::vector<Regiao> outras;
                        for (const Regiao r : {Regiao::Nav, Regiao::Main, Regiao::Aside}) if (r != d.regiao) outras.push_back(r);
                        e.ancora = s.Um(outras);
                    }
                    e.atributo = Atributo(s, e.tipo, e.ancora);
                    pagina.push_back(e);
                }
            }
            return pagina;
        }
    }

    std::vector<Elemento> Gerar(Variacao::Sorteio& s, const float largura) {
        std::vector<Elemento> pagina;
        for (int tentativa = 0; tentativa < 32; ++tentativa) {
            pagina = Tentar(s, largura);
            std::set<Tipo> tipos;
            for (const Elemento& e : pagina) tipos.insert(e.tipo);
            if (pagina.size() >= 6u && tipos.size() >= 3u) break;
        }
        return pagina;
    }

    std::string Placar(const Elemento& e) {
        std::string tag;
        switch (e.tipo) {
            case Tipo::Botao:   tag = "button"; break;
            case Tipo::Imagem:  tag = "img"; break;
            case Tipo::Marquee: tag = "marquee"; break;
            case Tipo::Tabela:  tag = "table"; break;
            case Tipo::Link:    tag = "a"; break;
            case Tipo::Input:   tag = "input"; break;
        }
        return "<" + tag + (e.atributo.empty() ? "" : " " + e.atributo) + ">\nbody > " + NomeDaRegiao(e.regiao);
    }

    std::vector<std::vector<std::array<float, 2>>> Celulas(const Retangulo& tabela) {
        std::vector<std::vector<std::array<float, 2>>> linhas;
        for (int r = 0; r < kLinhasDaTabela; ++r) {
            std::vector<std::array<float, 2>> linha;
            for (int c = 0; c < kColunasDaTabela; ++c) {
                linha.push_back({tabela.x + (static_cast<float>(c) + 0.5f) * tabela.w / kColunasDaTabela,
                                 tabela.y + (static_cast<float>(r) + 0.5f) * tabela.h / kLinhasDaTabela});
            }
            linhas.push_back(linha);
        }
        return linhas;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.porElemento = Dificuldade::Escolher(n, kPorElemento, 0.95f);
        d.carregando = Dificuldade::Escolher(n, kCarregando, 0.7f);
        return d;
    }
}
