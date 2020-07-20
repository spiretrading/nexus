#include "Spire/Ui/StaticDropDownMenu.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

StaticDropDownMenu::StaticDropDownMenu(const std::vector<QVariant>& items,
  QWidget* parent)
  : StaticDropDownMenu(items, "", parent) {}

StaticDropDownMenu::StaticDropDownMenu(const std::vector<QVariant>& items,
    const QString& display_text, QWidget* parent)
    : QWidget(parent),
      m_display_text(display_text),
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
  setFocusPolicy(Qt::StrongFocus);
  if(!items.empty()) {
    m_current_item = items.front();
  }
  m_menu_list = new DropDownList({}, true, this);
  m_menu_selection_connection = m_menu_list->connect_value_selected_signal(
    [=] (const auto& value) { on_item_selected(value); });
  set_items(items);
}

int StaticDropDownMenu::item_count() const {
  return m_menu_list->item_count();
}

void StaticDropDownMenu::insert_item(DropDownItem* item) {
  m_menu_list->insert_item(item);
}

void StaticDropDownMenu::remove_item(int index) {
  m_menu_list->remove_item(index);
}

void StaticDropDownMenu::set_items(const std::vector<QVariant>& items) {
  auto widget_items = std::vector<DropDownItem*>();
  widget_items.reserve(items.size());
  for(auto& item : items) {
    auto item_widget = new DropDownItem(item, this);
    item_widget->setFixedHeight(scale_height(20));
    widget_items.push_back(item_widget);
  }
  m_menu_list->set_items(widget_items);
  m_menu_list->setFixedWidth(width());
}

const QVariant& StaticDropDownMenu::get_current_item() const {
  return m_current_item;
}

void StaticDropDownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->isActiveWindow()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  if(m_display_text.isEmpty()) {
    draw_item_text(m_item_delegate.displayText(m_current_item), painter);
  } else {
    draw_item_text(m_display_text, painter);
  }
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + PADDING()),
    scale_height(11)), m_dropdown_image);
}

void StaticDropDownMenu::resizeEvent(QResizeEvent* event) {
  m_menu_list->setFixedWidth(width());
}

connection StaticDropDownMenu::connect_index_selected_signal(
    const IndexSelectedSignal::slot_type& slot) const {
  return m_menu_list->connect_index_selected_signal(slot);
}

connection StaticDropDownMenu::connect_value_selected_signal(
    const ValueSelectedSignal::slot_type& slot) const {
  return m_value_selected_signal.connect(slot);
}

void StaticDropDownMenu::draw_item_text(const QString& text,
    QPainter& painter) {
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  painter.drawText(QPoint(PADDING(),
    (height() / 2) + (metrics.ascent() / 2) - 1),
    metrics.elidedText(text, Qt::ElideRight, width() - (PADDING() * 3)));
}

void StaticDropDownMenu::on_item_selected(const QVariant& value) {
  m_current_item = value;
  m_value_selected_signal(m_current_item);
  update();
}
