#include "Spire/Ui/StaticDropDownMenu.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

StaticDropDownMenu::StaticDropDownMenu(const std::vector<QString>& items,
    QWidget* parent)
    : QWidget(parent),
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
  setFocusPolicy(Qt::StrongFocus);
  if(!items.empty()) {
    m_current_text = items.front();
  }
  m_menu_list = new DropDownList({}, true, this);
  m_menu_list->connect_selected_signal([=] (const auto& value) {
    on_item_selected(value);
  });
  set_items(items);
}

void StaticDropDownMenu::set_list_width(int width) {
  m_menu_list->setFixedWidth(width);
}

void StaticDropDownMenu::set_current_text(const QString& text) {
  m_current_text = text;
}

void StaticDropDownMenu::set_items(const std::vector<QString>& items) {
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

const QString& StaticDropDownMenu::get_text() const {
  return m_current_text;
}

void StaticDropDownMenu::paintEvent(QPaintEvent* event) {
  // TODO: move to anon namespace
  auto PADDING = [] { return scale_width(8); };
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->isActiveWindow()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  painter.drawText(QPoint(PADDING(),
    (height() / 2) + (metrics.ascent() / 2) - 1),
    metrics.elidedText(m_current_text, Qt::ElideRight,
    width() - (PADDING() * 3)));
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + PADDING()),
    scale_height(11)), m_dropdown_image);
}

void StaticDropDownMenu::resizeEvent(QResizeEvent* event) {
  m_menu_list->setFixedWidth(width());
}

connection StaticDropDownMenu::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void StaticDropDownMenu::on_item_selected(const QVariant& value) {
  m_current_text = value.value<QString>();
  m_selected_signal(value);
  update();
}
