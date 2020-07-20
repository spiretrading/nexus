#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

FilteredDropDownMenu::FilteredDropDownMenu(const std::vector<QVariant>& items,
    QWidget* parent)
    : TextInputWidget(parent),
      m_was_click_focused(false) {
  setAttribute(Qt::WA_Hover);
  setFocusPolicy(Qt::StrongFocus);
  connect(this, &QLineEdit::editingFinished, this,
    &FilteredDropDownMenu::on_editing_finished);
  connect(this, &QLineEdit::textEdited, this,
    &FilteredDropDownMenu::on_text_edited);
  if(!items.empty()) {
    m_current_item = items.front();
    setText(m_item_delegate.displayText(m_current_item));
  }
  m_menu_list = new DropDownList({}, false, this);
  m_menu_list->connect_activated_signal([=] (const auto& value) {
    on_item_activated(value);
  });
  m_list_selection_connection = m_menu_list->connect_value_selected_signal(
    [=] (const auto& value) { on_item_selected(value); });
  set_items(items);
  m_menu_list->installEventFilter(this);
}

bool FilteredDropDownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_menu_list) {
    if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
      update();
    }
  }
  return TextInputWidget::eventFilter(watched, event);
}

void FilteredDropDownMenu::focusInEvent(QFocusEvent* event) {
  TextInputWidget::focusInEvent(event);
  if(event->reason() == Qt::FocusReason::MouseFocusReason) {
    m_was_click_focused = true;
  }
}

void FilteredDropDownMenu::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    setText(m_item_delegate.displayText(m_current_item));
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_last_activated_item.isValid()) {
      on_item_selected(m_last_activated_item);
      return;
    }
    if(!text().isEmpty() && m_menu_list->isVisible()) {
      auto item = std::move(m_menu_list->get_value(0));
      if(item.isValid()) {
        m_list_selection_connection.disconnect();
        on_item_selected(item);
      }
    }
    return;
  }
  TextInputWidget::keyPressEvent(event);
}

void FilteredDropDownMenu::mousePressEvent(QMouseEvent* event) {
  TextInputWidget::mousePressEvent(event);
  if(m_was_click_focused) {
    selectAll();
    m_was_click_focused = false;
  }
}

void FilteredDropDownMenu::paintEvent(QPaintEvent* event) {
  TextInputWidget::paintEvent(event);
  if(!text().isEmpty()) {
    auto item = std::move(m_menu_list->get_value(0));
    if(item.isValid()) {
      auto item_text = m_item_delegate.displayText(item);
      if(item_text.startsWith(text(), Qt::CaseInsensitive) &&
          item_text.length() != text().length()) {
        item_text = item_text.remove(0, text().length());
        draw_highlight(item_text);
      }
    }
  }
  if(m_menu_list->isActiveWindow()) {
    draw_border(QColor("#4B23A0"));
  }
}

connection FilteredDropDownMenu::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void FilteredDropDownMenu::set_items(const std::vector<QVariant>& items) {
  m_items = items;
  m_menu_list->set_items(create_widget_items(m_items));
  m_menu_list->setFixedWidth(width());
}

const std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items) {
  return create_widget_items(items, "");
}

const std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items, const QString& filter_text) {
  auto widget_items = std::vector<DropDownItem*>();
  widget_items.reserve(items.size());
  for(const auto& item : items) {
    if(filter_text.isEmpty() || m_item_delegate.displayText(item).startsWith(
        filter_text, Qt::CaseInsensitive)) {
      auto item_widget = new DropDownItem(item, this);
      item_widget->setFixedHeight(scale_height(20));
      widget_items.push_back(item_widget);
    }
  }
  return widget_items;
}

void FilteredDropDownMenu::draw_border(const QColor& color) {
  auto painter = QPainter(this);
  painter.setPen(color);
  painter.drawRect(0, 0, width() - 1, height() - 1);
}

void FilteredDropDownMenu::draw_highlight(const QString& highlight_text) {
  auto painter = QPainter(this);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  auto highlight_x_pos = metrics.horizontalAdvance(text()) + PADDING() +
    scale_width(3);
  painter.fillRect(highlight_x_pos,
    (height() / 2) - ((metrics.ascent() + scale_height(4)) / 2) - 1,
    metrics.horizontalAdvance(highlight_text),
    metrics.ascent() + scale_height(4), QColor("#0078D7"));
  painter.setPen(Qt::white);
  painter.drawText(QPoint(highlight_x_pos,
    (height() / 2) + (metrics.ascent() / 2) - 1), highlight_text);
}

void FilteredDropDownMenu::on_editing_finished() {
  if(text().isEmpty()) {
    setText(m_item_delegate.displayText(m_current_item));
    return;
  }
  if(m_last_activated_item.isValid()) {
    on_item_selected(m_last_activated_item);
    setText(m_item_delegate.displayText(m_last_activated_item));
    m_last_activated_item = QVariant();
    return;
  }
  auto item = std::move(m_menu_list->get_value(0));
  if(item.isValid() &&
      m_item_delegate.displayText(item).startsWith(text(),
      Qt::CaseInsensitive)) {
    on_item_selected(item);
    return;
  }
  setText(m_item_delegate.displayText(m_current_item));
}

void FilteredDropDownMenu::on_item_activated(const QVariant& item) {
  m_last_activated_item = item;
  setText(m_item_delegate.displayText(item));
}

void FilteredDropDownMenu::on_item_selected(const QVariant& item) {
  m_current_item = item;
  setText(m_item_delegate.displayText(m_current_item));
  m_selected_signal(m_current_item);
  m_list_selection_connection = m_menu_list->connect_value_selected_signal(
    [=] (const auto& value) { on_item_selected(value); });
  set_items(m_items);
  update();
}

void FilteredDropDownMenu::on_text_edited(const QString& text) {
  m_last_activated_item = QVariant();
  if(text.isEmpty()) {
    m_menu_list->set_items(create_widget_items(m_items));
    m_menu_list->show();
    return;
  }
  auto items = create_widget_items(m_items, text);
  if(items.empty()) {
    m_menu_list->hide();
    return;
  }
  m_menu_list->set_items(items);
  m_menu_list->show();
}
