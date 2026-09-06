#include <vector>
#include <doctest/doctest.h>
#include <QEventLoop>
#include <QTimer>
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/ColorBox.hpp"

using namespace Spire;

namespace {
  using FillType = BookViewLevelProperties::FillType;

  auto make_current(FillType type) {
    auto properties = BookViewLevelProperties::get_default();
    properties.m_fill_type = type;
    properties.m_color_scheme = {QColor(0x101010), QColor(0x202020),
      QColor(0x303030), QColor(0x404040)};
    return std::make_shared<LocalLevelPropertiesModel>(properties);
  }

  std::vector<ColorBox*> find_color_boxes(const QWidget& page) {
    auto boxes = std::vector<ColorBox*>();
    for(auto widget : page.findChildren<QWidget*>()) {
      if(auto box = dynamic_cast<ColorBox*>(widget)) {
        boxes.push_back(box);
      }
    }
    return boxes;
  }

  void wait() {
    auto loop = QEventLoop();
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
  }
}

TEST_SUITE("BookViewLevelPropertiesPage") {
  TEST_CASE("pending_gradient_update_does_not_overwrite_solid_colors") {
    run_test([] {
      auto current = make_current(FillType::GRADIENT);
      auto page = BookViewLevelPropertiesPage(current);
      auto color_boxes = find_color_boxes(page);
      REQUIRE(color_boxes.size() == 2);
      color_boxes.front()->get_current()->set(QColor(0xFF0000));
      REQUIRE(current->get().m_color_scheme[0] == QColor(0xFF0000));
      auto properties = current->get();
      properties.m_fill_type = FillType::SOLID;
      current->set(properties);
      auto scheme = current->get().m_color_scheme;
      wait();
      REQUIRE(current->get().m_color_scheme == scheme);
    });
  }

  TEST_CASE("flush_applies_a_pending_gradient_update") {
    run_test([] {
      auto current = make_current(FillType::GRADIENT);
      auto page = BookViewLevelPropertiesPage(current);
      auto initial = current->get().m_color_scheme;
      auto color_boxes = find_color_boxes(page);
      REQUIRE(color_boxes.size() == 2);
      color_boxes.front()->get_current()->set(QColor(0xFF0000));
      REQUIRE(current->get().m_color_scheme[1] == initial[1]);
      page.flush();
      auto scheme = current->get().m_color_scheme;
      REQUIRE(scheme[0] == QColor(0xFF0000));
      REQUIRE(scheme[1] != initial[1]);
      wait();
      REQUIRE(current->get().m_color_scheme == scheme);
    });
  }
}
