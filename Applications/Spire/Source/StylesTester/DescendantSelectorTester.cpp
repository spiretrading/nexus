#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

namespace {
  using ListicleStyleA = BasicProperty<int, struct ListicleStyleATag>;
  using ListicleStyleB = BasicProperty<int, struct ListicleStyleBTag>;
  using TokenStyleA = BasicProperty<int, struct TokenStyleATag>;
  using TokenStyleB = BasicProperty<int, struct TokenStyleBTag>;
  using TokenStyleC = BasicProperty<int, struct TokenStyleCTag>;
  using TokenStyleD = BasicProperty<int, struct TokenStyleDTag>;

  struct Container : QWidget {
    using QWidget::QWidget;
  };

  struct Listicle : QWidget {
    using QWidget::QWidget;
  };

  struct Div : QWidget {
    using QWidget::QWidget;
  };

  struct Token : QWidget {
    using QWidget::QWidget;
  };
}

TEST_SUITE("DescendantSelector") {
  TEST_CASE("equality") {
    REQUIRE(
      DescendantSelector(Foo(), Foo()) == DescendantSelector(Foo(), Foo()));
    REQUIRE(
      DescendantSelector(Foo(), Foo()) != DescendantSelector(Foo(), Bar()));
    REQUIRE(
      DescendantSelector(Foo(), Foo()) != DescendantSelector(Bar(), Foo()));
    REQUIRE(
      DescendantSelector(Foo(), Bar()) != DescendantSelector(Foo(), Foo()));
    REQUIRE(
      DescendantSelector(Bar(), Foo()) != DescendantSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(DescendantSelector(Foo(), Bar()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["B"], Foo());
      REQUIRE(updates.empty());
      match(*graph["C"], Bar());
      require_selection(updates, graph, {"C"}, {});
      match(*graph["H"], Bar());
      require_selection(updates, graph, {"H"}, {});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"H"});
    });
  }

  TEST_CASE("nested_selection") {
    run_test([] {
      auto container = new Container();
      auto listicle = new Listicle(container);
      auto div = new Div(listicle);
      auto listicle_style = StyleSheet();
      listicle_style.get(Any()).
        set(ListicleStyleA(123)).
        set(ListicleStyleB(456));
      set_style(*listicle, listicle_style);
      proxy_style(*listicle, *div);
      update_style(*listicle, [] (auto& style) {
        style.get(Any() >> is_a<Token>()).
          set(TokenStyleA(44)).
          set(TokenStyleB(55)).
          set(TokenStyleC(66));
        return style;
      });
      auto make_token = [] {
        auto token = new Token();
        auto style = StyleSheet();
        style.get(Any()).
          set(TokenStyleA(255)).
          set(TokenStyleB(0)).
          set(TokenStyleC(8)).
          set(TokenStyleD(12));
        style.get(Foo()).set(TokenStyleA(0xF2F2FF));
        style.get(Bar()).set(TokenStyleA(0xD0D0D0));
        style.get(Qux()).set(TokenStyleA(0xE2E0FF));
        set_style(*token, style);
        return token;
      };
      for(auto i = 0; i != 100; ++i) {
        auto token = make_token();
        token->setParent(listicle);
        match(*token, Qux());
        auto block = get_computed_block(*token);
        REQUIRE((find<TokenStyleA>(block) == TokenStyleA(44)));
        REQUIRE((find<TokenStyleB>(block) == TokenStyleB(55)));
        REQUIRE((find<TokenStyleC>(block) == TokenStyleC(66)));
        REQUIRE((find<TokenStyleD>(block) == TokenStyleD(12)));
        token->setParent(nullptr);
        delete token;
      }
    });
  }
}
