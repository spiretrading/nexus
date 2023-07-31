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
  const auto DEMI_LIGHT = 35;

  int get_font_weight(const QString& style) {
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
    }
    auto s2 = s.midRef(2);
    if(s.startsWith("se") || s.startsWith("de")) {
      if(s2.contains("milight") || s2.contains("mi light")) {
        return DEMI_LIGHT;
      } else if(s2.contains("mibold") || s2.contains("mi bold")) {
        return QFont::DemiBold;
      }
    } else if(s.startsWith("ex") || s.startsWith("ul")) {
      if(s2.contains("tralight") || s2.contains("tra light")) {
        return QFont::ExtraLight;
      } else if(s2.contains("trabold") || s2.contains("tra bold")) {
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
    auto weight_styles = std::vector<std::tuple<int, QString>>();
    for(auto& style : styles) {
      weight_styles.push_back({get_font_weight(style), style});
    }
    std::sort(weight_styles.begin(), weight_styles.end(),
      [&] (auto& style1, auto& style2) {
        auto weight1 = get<0>(style1);
        auto weight2 = get<0>(style2);
        if(weight1 == weight2) {
          auto is_italic1 =
            get<1>(style1).contains("italic", Qt::CaseInsensitive);
          auto is_italic2 =
            get<1>(style2).contains("italic", Qt::CaseInsensitive);
          if(!is_italic1 && is_italic2) {
            return true;
          } else if(is_italic1 && !is_italic2) {
            return false;
          }
          return get<1>(style1) < get<1>(style2);
        }
        return weight1 < weight2;
      });
    auto result = std::vector<QString>();
    for(auto& style : weight_styles) {
      result.push_back(get<1>(style));
    }
    return result;
  }

  auto get_initial_style(const QString& font_family) {
    auto styles = QFontDatabase().styles(font_family);
    if(styles.isEmpty()) {
      return QString();
    }
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
    auto family = [&] {
      if(font_database.writingSystems(font_family->get()).contains(
          QFontDatabase::Latin)) {
        return font_family->get();
      }
      return QString("Roboto");
    }();
    auto font = font_database.font(family, font_style, -1);
    font.setPixelSize(scale_width(12));
    auto label = make_label(font_style);
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
