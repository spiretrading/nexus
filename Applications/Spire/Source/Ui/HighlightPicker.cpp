#include "Spire/Ui/HighlightPicker.hpp"
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

HighlightPicker::HighlightPicker(QWidget& parent)
  : HighlightPicker(std::make_shared<LocalValueModel<Highlight>>(), parent) {}

HighlightPicker::HighlightPicker(std::shared_ptr<ValueModel<Highlight>> current,
    QWidget& parent)
    : QWidget(&parent),
      m_current(std::move(current)) {
  auto list_model = std::make_shared<ArrayListModel<Highlight>>();
  list_model->push({"#FFF1F1", "#B71C1C"});
  list_model->push({"#FFECFF", "#76008A"});
  list_model->push({"#F4F4FF", "#4C00DA"});
  list_model->push({"#DDF9FF", "#003698"});
  list_model->push({"#EBFFF0", "#007735"});
  list_model->push({"#FFFFC4", "#834A2D"});
  list_model->push({"#FFE6C9", "#A00000"});
  list_model->push({"#FFFFFF", "#000000"});

  list_model->push({"#FFABAA", "#FFFFFF"});
  list_model->push({"#F1A6F1", "#770088"});
  list_model->push({"#C1BAFF", "#4F00D7"});
  list_model->push({"#92CFE9", "#0000C6"});
  list_model->push({"#96D8A8", "#004E00"});
  list_model->push({"#FEEE7F", "#630000"});
  list_model->push({"#FFBF76", "#A00000"});
  list_model->push({"#C6C6C6", "#000000"});

  list_model->push({"#C33A39", "#FFFFFF"});
  list_model->push({"#A937B2", "#FFFFFF"});
  list_model->push({"#6F53C5", "#FFFFFF"});
  list_model->push({"#246FBC", "#FFFFFF"});
  list_model->push({"#218542", "#FFFFFF"});
  list_model->push({"#8A6729", "#FFFFFF"});
  list_model->push({"#AE4D1F", "#FFFFFF"});
  list_model->push({"#6E6E6E", "#FFFFFF"});

  list_model->push({"#780000", "#FFFFFF"});
  list_model->push({"#5F006F", "#FFFFFF"});
  list_model->push({"#361976", "#FFFFFF"});
  list_model->push({"#21148C", "#FFFFFF"});
  list_model->push({"#004516", "#FFFFFF"});
  list_model->push({"#4D2A00", "#FFFFFF"});
  list_model->push({"#680000", "#FFFFFF"});
  list_model->push({"#000000", "#FFFFFF"});
  m_palette = new ListView(std::move(list_model), [] (const std::shared_ptr<AnyListModel>& list, int index) {
    auto current = std::make_shared<LocalValueModel<HighlightSwatch::Highlight>>(std::any_cast<Highlight>(list->get(index)));
    auto swatch = new HighlightSwatch(current);
    swatch->setFixedSize(scale(22, 18));
    return swatch;
  });
  update_style(*m_palette, [] (auto& style) {
    style.get(Any()).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    style.get(Any() > is_a<ListItem>()).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(BackgroundColor(Qt::transparent)).
      set(horizontal_padding(scale_width(1))).
      set(vertical_padding(scale_height(1)));
    style.get(Any() > (is_a<ListItem>() && (Hover() || Current()))).
      set(border_color(QColor(0x4B23A0)));
  });
  enclose(*this, *m_palette);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->set_closed_on_focus_out(true);
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  on_current(m_current->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&HighlightPicker::on_current, this));
  m_palette->get_current()->connect_update_signal(
    std::bind_front(&HighlightPicker::on_palette_current, this));
}

const std::shared_ptr<ValueModel<HighlightPicker::Highlight>>&
    HighlightPicker::get_current() const {
  return m_current;
}

bool HighlightPicker::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_palette->setFixedWidth(m_palette->get_list_item(0)->sizeHint().width() * 8);
    m_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void HighlightPicker::on_current(const Highlight& highlight) {
}

void HighlightPicker::on_palette_current(optional<int> current) {
  if(!current) {
    return;
  }
  m_current->set(std::any_cast<Highlight>(m_palette->get_list()->get(*current)));
}
