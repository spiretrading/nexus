#include "Spire/Ui/FontFamilyBox.hpp"
#include <map>
#include <QFontDatabase>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct FamilyMetrics {
    QFont m_font;
    int m_width;
  };

  const FamilyMetrics& get_family_metrics(const QString& family) {
    static auto metrics = std::map<QString, FamilyMetrics>();
    if(auto i = metrics.find(family); i != metrics.end()) {
      return i->second;
    }
    auto database = QFontDatabase();
    auto family_metrics = FamilyMetrics();
    family_metrics.m_font = QFont([&] {
      if(database.writingSystems(family).contains(QFontDatabase::Latin)) {
        return family;
      }
      return QString("Roboto");
    }());
    family_metrics.m_font.setWeight(QFont::Normal);
    family_metrics.m_font.setPixelSize(scale_width(12));
    family_metrics.m_width =
      QFontMetrics(family_metrics.m_font).horizontalAdvance(family);
    return metrics.emplace(family, std::move(family_metrics)).first->second;
  }

  int get_max_family_width(const ListModel<QString>& families) {
    auto width = 0;
    for(auto i = 0; i != families.get_size(); ++i) {
      width = std::max(width, get_family_metrics(families.get(i)).m_width);
    }
    return width;
  }
}

FontFamilyBox* Spire::make_font_family_box(const QString& current,
    QWidget* parent) {
  auto families = QFontDatabase().families();
  auto font_families = std::make_shared<ArrayListModel<QString>>();
  for(auto& family : families) {
    font_families->push(family);
  }
  return make_font_family_box(std::move(font_families),
    std::make_shared<LocalValueModel<QString>>(current), parent);
}

FontFamilyBox* Spire::make_font_family_box(
    std::shared_ptr<ListModel<QString>> font_families,
    std::shared_ptr<ValueModel<QString>> current, QWidget* parent) {
  auto width = get_max_family_width(*font_families);
  auto settings = FontFamilyBox::Settings();
  settings.m_cases = std::move(font_families);
  settings.m_current = std::move(current);
  settings.m_view_builder = [width] (auto& value) {
    auto label = make_label(value);
    update_style(*label, [&] (auto& style) {
      style.get(Any()).set(Font(get_family_metrics(value).m_font));
    });
    label->setFixedHeight(scale_height(26));
    label->setMinimumWidth(width);
    return label;
  };
  auto box = new FontFamilyBox(std::move(settings), parent);
  update_style(*box, [] (auto& style) {
    style.get(Any() > is_a<DropDownBox>() > is_a<DropDownList>() >
        is_a<ListView>() > is_a<ListItem>()).
      set(border_size(0)).
      set(vertical_padding(0));
  });
  invalidate_descendant_layouts(*box);
  return box;
}
