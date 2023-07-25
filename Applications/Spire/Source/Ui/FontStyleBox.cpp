#include "Spire/Ui/FontStyleBox.hpp"
#include <QFontDatabase>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto get_font_styles(const QString& font_family) {
    auto font_database = QFontDatabase();
    auto styles = font_database.styles(font_family);
    auto sorted_styles = std::vector<QString>();
    for(auto& style : styles) {
      sorted_styles.push_back(style);
    }
    std::sort(sorted_styles.begin(), sorted_styles.end(),
      [&] (auto& style1, auto& style2) {
        auto weight1 = font_database.weight(font_family, style1);
        auto weight2 = font_database.weight(font_family, style2);
        if(weight1 == weight2) {
          return style1 < style2;
        }
        return weight1 < weight2;
      });
    return sorted_styles;
  }

  QString get_initial_style(const QString& font_family) {
    auto styles = QFontDatabase().styles(font_family);
    if(styles.contains("Regular")) {
      return "Regular";
    }
    return styles[0];
  }
}

FontStyleBox* Spire::make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family,  QWidget* parent) {
  return make_font_style_box(std::move(font_family),
    std::make_shared<LocalValueModel<QString>>(
      get_initial_style(font_family->get())), parent);
}

FontStyleBox* Spire::make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family,
    std::shared_ptr<ValueModel<QString>> current, QWidget* parent) {
  auto font_database = QFontDatabase();
  auto settings = FontStyleBox::Settings();
  auto font_styles = std::make_shared<ArrayListModel<QString>>(
    get_font_styles(font_family->get()));
  settings.m_cases = font_styles;
  settings.m_current = std::move(current);
  settings.m_view_builder = [=] (auto& font_style) {
    auto label = make_label(font_style);
    auto family = font_family->get();
    auto font = QFont(family);
    font.setWeight(font_database.weight(family, font_style));
    font.setPixelSize(scale_width(12));
    update_style(*label, [&] (auto& style) {
      style.get(Any()).set(Font(font));
    });
    label->setFixedHeight(scale_height(26));
    return label;
  };
  auto box = new FontStyleBox(std::move(settings), parent);
  update_style(*box, [&] (auto& style) {
    style.get(Any() > is_a<ListItem>()).
      set(border_size(0)).
      set(vertical_padding(0));
  });
  font_family->connect_update_signal([=] (auto& family) {
    box->get_current()->set("");
    clear(*font_styles);
    for(auto& style : get_font_styles(family)) {
      font_styles->push(style);
    }
    box->get_current()->set(get_initial_style(family));
  });
  return box;
}
