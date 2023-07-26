#include "Spire/Ui/FontStyleBox.hpp"
#include <QFontDatabase>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto INITIAL_STYLE = QString("Regular");

  auto get_font_weight(const QString& style) {
    auto s = style.toLower();
    if(s == "normal" || s == "regular") {
      return QFont::Normal;
    } else if(s == "bold") {
      return QFont::Bold;
    } else if(s == "medium") {
      return QFont::Medium;
    } else if(s == "black") {
      return QFont::Black;
    } else if(s == "light") {
      return QFont::Light;
    } else if(s == "thin") {
      return QFont::Thin;
    } else if(s == "semibold" || s == "semi bold" ||
        s == "demibold" || s == "demi bold") {
      return QFont::DemiBold;
    }
    auto s2 = s.midRef(2);
    if(s.startsWith("ex") || s.startsWith("ul")) {
      if(s2 == "tralight" || s2 == "tra light") {
        return QFont::ExtraLight;
      } else if(s2 == "trabold" || s2 == "tra bold") {
        return QFont::ExtraBold;
      }
    }
    if(s.contains("bold")) {
      return QFont::Bold;
    } else if(s.contains("medium")) {
      return QFont::Medium;
    } else if(s.contains("black")) {
      return QFont::Black;
    } else if(s.contains("light")) {
      return QFont::Light;
    } else if(s.contains("thin")) {
      return QFont::Thin;
    }
    return QFont::Normal;
  }

  auto get_font_styles(const QString& font_family) {
    auto font_database = QFontDatabase();
    auto styles = font_database.styles(font_family);
    std::sort(styles.begin(), styles.end(),
      [&] (auto& style1, auto& style2) {
        auto weight1 = get_font_weight(style1);
        auto weight2 = get_font_weight(style2);
        if(weight1 == weight2) {
          auto is_italic1 = style1.contains("italic", Qt::CaseInsensitive);
          auto is_italic2 = style2.contains("italic", Qt::CaseInsensitive);
          if(!is_italic1 && is_italic2) {
            return true;
          } else if(is_italic1 && !is_italic2) {
            return false;
          }
          return style1 < style2;
        }
        return weight1 < weight2;
      });
    return std::vector<QString>(styles.begin(), styles.end());
  }

  auto get_initial_style(const QString& font_family) {
    auto styles = QFontDatabase().styles(font_family);
    if(styles.contains(INITIAL_STYLE)) {
      return INITIAL_STYLE;
    }
    return styles[0];
  }
}

FontStyleBox* Spire::make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family, QWidget* parent) {
  return make_font_style_box(std::move(font_family),
    std::make_shared<LocalValueModel<QString>>(
      get_initial_style(font_family->get())), parent);
}

FontStyleBox* Spire::make_font_style_box(
    std::shared_ptr<ValueModel<QString>> font_family,
    std::shared_ptr<ValueModel<QString>> current, QWidget* parent) {
  auto settings = FontStyleBox::Settings();
  auto font_styles = std::make_shared<ArrayListModel<QString>>(
    get_font_styles(font_family->get()));
  settings.m_cases = font_styles;
  settings.m_current = std::move(current);
  settings.m_view_builder = [=] (auto& font_style) {
    auto font_database = QFontDatabase();
    auto label = make_label(font_style);
    auto family = [&] {
      if(QFontDatabase().writingSystems(font_family->get()).contains(
          QFontDatabase::Latin)) {
        return font_family->get();
      }
      return QString("Roboto");
    }();
    auto font = QFont(family);
    font.setWeight(font_database.weight(family, font_style));
    font.setItalic(font_database.italic(family, font_style));
    font.setPixelSize(scale_width(12));
    update_style(*label, [&] (auto& style) {
      style.get(Any()).set(Font(font));
    });
    label->setFixedHeight(scale_height(26));
    return label;
  };
  auto box = new FontStyleBox(std::move(settings), parent);
  auto update_box_style = [=] {
    update_style(*box, [] (auto& style) {
      style.get(Any() > is_a<ListItem>()).
        set(border_size(0)).
        set(vertical_padding(0));
    });
  };
  update_box_style();
  font_family->connect_update_signal([=] (auto& family) {
    box->get_current()->set("");
    clear(*font_styles);
    for(auto& style : get_font_styles(family)) {
      font_styles->push(style);
    }
    box->get_current()->set(get_initial_style(family));
    update_box_style();
  });
  return box;
}
