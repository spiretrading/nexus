#include "Spire/Ui/StaticDropDownMenu.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto INPUT_TIMEOUT_MS = 1000;

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
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))),
      m_disabled_dropdown_image(imageFromSvg(":/Icons/arrow-down-grey.svg",
        scale(6, 4))) {
  setFocusPolicy(Qt::StrongFocus);
  if(!items.empty()) {
    m_current_item = items.front();
  }
  m_menu_list = new DropDownList({}, true, this);
  if(m_display_text.isEmpty()) {
    m_menu_activated_connection = m_menu_list->connect_activated_signal(
      [=] (const auto& value) { on_item_activated(value); });
  }
  m_menu_selection_connection = m_menu_list->connect_value_selected_signal(
    [=] (const auto& value) { on_item_selected(value); });
  m_menu_list->installEventFilter(this);
  set_items(items);
  connect(&m_input_timer, &QTimer::timeout, this,
    &StaticDropDownMenu::on_input_timeout);
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

bool StaticDropDownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_menu_list) {
    if(event->type() == QEvent::KeyPress) {
      on_key_press(static_cast<QKeyEvent*>(event));
    } else if(event->type() == QEvent::Show && m_entered_text.isEmpty()) {
      m_menu_list->set_highlight(m_item_delegate.displayText(m_current_item));
    } else if(event->type() == QEvent::Hide) {
      m_preview_text.clear();
      on_input_timeout();
      update();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void StaticDropDownMenu::keyPressEvent(QKeyEvent* event) {
  on_key_press(event);
}

void StaticDropDownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->isActiveWindow()) {
    draw_border(QColor("#4B23A0"), painter);
  } else {
    draw_border(QColor("#C8C8C8"), painter);
  }
  if(isEnabled()) {
    draw_background(Qt::white, painter);
  } else {
    draw_background(Qt::transparent, painter);
  }
  if(!m_preview_text.isEmpty()) {
    draw_item_text(m_preview_text, painter);
  } else if(m_display_text.isEmpty()) {
    draw_item_text(m_item_delegate.displayText(m_current_item), painter);
  } else {
    draw_item_text(m_display_text, painter);
  }
  if(isEnabled()) {
    draw_arrow(m_dropdown_image, painter);
  } else {
    draw_arrow(m_disabled_dropdown_image, painter);
  }
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

void StaticDropDownMenu::draw_arrow(const QImage& arrow_image,
    QPainter& painter) {
  painter.drawImage(QPoint(width() - (arrow_image.width() + PADDING()),
    scale_height(11)), arrow_image);
}

void StaticDropDownMenu::draw_background(const QColor& color,
    QPainter& painter) {
  painter.fillRect(1, 1, width() - 2, height() - 2, color);
}

void StaticDropDownMenu::draw_border(const QColor& color, QPainter& painter) {
  painter.setPen(color);
  painter.drawRect(0, 0, width() - 1, height() - 1);
}

void StaticDropDownMenu::draw_item_text(const QString& text,
    QPainter& painter) {
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  if(isEnabled()) {
    painter.setPen(Qt::black);
  } else {
    painter.setPen(QColor("#A0A0A0"));
  }
  painter.drawText(QPoint(PADDING(),
    (height() / 2) + (metrics.ascent() / 2) - 1),
    metrics.elidedText(text, Qt::ElideRight, width() - (PADDING() * 3)));
}

void StaticDropDownMenu::on_input_timeout() {
  m_input_timer.stop();
  m_entered_text.clear();
}

void StaticDropDownMenu::on_item_activated(const QVariant& value) {
  m_preview_text = m_item_delegate.displayText(value);
  update();
}

void StaticDropDownMenu::on_item_selected(const QVariant& value) {
  m_preview_text.clear();
  m_current_item = value;
  m_value_selected_signal(m_current_item);
  update();
}

void StaticDropDownMenu::on_key_press(QKeyEvent* event) {
  if(event->key() >= Qt::Key_Exclam && event->key() <= Qt::Key_AsciiTilde) {
    m_input_timer.start(INPUT_TIMEOUT_MS);
    m_entered_text.push_back(event->text());
    if(m_menu_list->set_highlight(m_entered_text)) {
      m_menu_list->show();
    } else if(m_entered_text.size() == 1) {
      on_input_timeout();
    }
  }
}
