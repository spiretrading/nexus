#include <QLayout>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct Fixture {
    QWidget* m_primary;
    QWidget* m_secondary;
    SplitView m_view;
    Qt::Orientation m_orientation;

    explicit Fixture(Qt::Orientation orientation)
        : m_primary(new QWidget()),
          m_secondary(new QWidget()),
          m_view(*m_primary, *m_secondary),
          m_orientation(orientation) {
      m_primary->setMinimumSize(80, 80);
      m_secondary->setMinimumSize(60, 60);
      update_style(m_view, [&] (auto& style) {
        style.get(Any()).set(orientation);
      });
      m_view.resize(600, 600);
    }

    void show() {
      m_view.show();
      QApplication::processEvents();
    }

    int get_size(const QWidget& widget) const {
      if(m_orientation == Qt::Orientation::Horizontal) {
        return widget.width();
      }
      return widget.height();
    }

    int get_content_size() const {
      return get_size(m_view) -
        get_size(*m_view.layout()->itemAt(1)->widget());
    }
  };
}

TEST_SUITE("SplitView") {
  UI_TEST_CASE("primary_ratio") {
    for(auto orientation : {Qt::Horizontal, Qt::Vertical}) {
      auto fixture = Fixture(orientation);
      fixture.m_view.set_primary_ratio(0.3);
      fixture.show();
      REQUIRE(fixture.m_view.get_primary_ratio() == 0.3);
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        static_cast<int>(0.3 * fixture.get_content_size()));
      fixture.m_view.resize(800, 800);
      QApplication::processEvents();
      REQUIRE(fixture.m_view.get_primary_ratio() == 0.3);
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        static_cast<int>(0.3 * fixture.get_content_size()));
      fixture.m_view.set_primary_ratio(0.7);
      QApplication::processEvents();
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        static_cast<int>(0.7 * fixture.get_content_size()));
      auto restored = Fixture(orientation);
      restored.m_view.set_primary_ratio(fixture.m_view.get_primary_ratio());
      restored.show();
      REQUIRE(restored.m_view.get_primary_ratio() == 0.7);
      REQUIRE(restored.get_size(*restored.m_primary) ==
        static_cast<int>(0.7 * restored.get_content_size()));
    }
  }

  UI_TEST_CASE("minimum_sizes") {
    for(auto orientation : {Qt::Horizontal, Qt::Vertical}) {
      auto fixture = Fixture(orientation);
      fixture.m_view.set_primary_ratio(0.1);
      fixture.show();
      REQUIRE(fixture.get_size(*fixture.m_primary) == 80);
      REQUIRE(fixture.m_view.get_primary_ratio() == 0.1);
      fixture.m_view.resize(1200, 1200);
      QApplication::processEvents();
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        static_cast<int>(0.1 * fixture.get_content_size()));
      fixture.m_view.set_primary_ratio(0.99);
      QApplication::processEvents();
      REQUIRE(fixture.get_size(*fixture.m_secondary) == 60);
      REQUIRE(fixture.m_view.get_primary_ratio() == 0.99);
    }
  }

  UI_TEST_CASE("collapsed_panels") {
    for(auto orientation : {Qt::Horizontal, Qt::Vertical}) {
      auto fixture = Fixture(orientation);
      fixture.m_view.set_primary_ratio(0);
      fixture.show();
      REQUIRE_FALSE(fixture.m_primary->isVisible());
      REQUIRE(fixture.m_secondary->isVisible());
      REQUIRE(fixture.m_view.get_primary_ratio() == 0);
      REQUIRE(fixture.get_size(*fixture.m_secondary) ==
        fixture.get_content_size());
      fixture.m_view.set_primary_ratio(1);
      QApplication::processEvents();
      REQUIRE(fixture.m_primary->isVisible());
      REQUIRE_FALSE(fixture.m_secondary->isVisible());
      REQUIRE(fixture.m_view.get_primary_ratio() == 1);
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        fixture.get_content_size());
      fixture.m_view.set_primary_ratio(0.4);
      QApplication::processEvents();
      REQUIRE(fixture.m_primary->isVisible());
      REQUIRE(fixture.m_secondary->isVisible());
      REQUIRE(fixture.get_size(*fixture.m_primary) ==
        static_cast<int>(0.4 * fixture.get_content_size()));
    }
  }
}
