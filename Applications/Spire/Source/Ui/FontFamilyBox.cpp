#include "Spire/Ui/FontFamilyBox.hpp"
#include <QFontDatabase>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

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
  auto settings = FontFamilyBox::Settings();
  settings.m_cases = std::move(font_families);
  settings.m_current = std::move(current);
  settings.m_view_builder = [] (auto& value) {
    auto family = [&] {
      if(QFontDatabase().writingSystems(value).contains(QFontDatabase::Latin)) {
        return value;
      }
      return QString("Roboto");
    }();
    auto font = QFont(family);
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    auto label = make_label(value);
    update_style(*label, [&] (auto& style) {
      style.get(Any()).set(Font(font));
    });
    label->setFixedHeight(scale_height(26));
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
