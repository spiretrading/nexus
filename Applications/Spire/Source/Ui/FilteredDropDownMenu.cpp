#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

FilteredDropDownMenu::FilteredDropDownMenu(std::vector<QVariant> items,
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
  m_item_activated_connection = m_menu_list->connect_activated_signal(
    [=] (const auto& value) { on_item_activated(value); });
  m_list_selection_connection = m_menu_list->connect_value_selected_signal(
    [=] (const auto& value) { on_item_selected(value, true); });
  set_items(std::move(items));
  m_menu_list->installEventFilter(this);
}

bool FilteredDropDownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_menu_list) {
    if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
      m_menu_list->setFixedWidth(width());
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

void FilteredDropDownMenu::focusOutEvent(QFocusEvent* event) {
  if(!m_menu_list->isActiveWindow()) {
    TextInputWidget::focusOutEvent(event);
  }
}

void FilteredDropDownMenu::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Delete:
      setText("");
      return;
    case Qt::Key_Escape:
      setText(m_item_delegate.displayText(m_current_item));
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(m_last_activated_item.isValid()) {
        on_item_selected(m_last_activated_item, true);
        return;
      }
      if(!text().isEmpty() && m_menu_list->isVisible()) {
        auto item = m_menu_list->get_value(0);
        if(item.isValid()) {
          on_item_selected(item, true);
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
    auto item = m_menu_list->get_value(0);
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

const QVariant& FilteredDropDownMenu::get_item() const {
  return m_current_item;
}

void FilteredDropDownMenu::set_items(std::vector<QVariant> items) {
  m_items = std::move(items);
  m_menu_list->set_items(std::move(create_widget_items(m_items)));
  m_menu_list->setFixedWidth(width());
}

std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items) {
  return create_widget_items(items, "");
}

std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items, const QString& filter_text) {
  auto widget_items = std::vector<DropDownItem*>();
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
  } else if(m_last_activated_item.isValid()) {
    on_item_selected(m_last_activated_item);
    setText(m_item_delegate.displayText(m_last_activated_item));
    m_last_activated_item = QVariant();
  } else if(m_menu_list->get_value(0).isValid() &&
      m_item_delegate.displayText(m_menu_list->get_value(0)).startsWith(text(),
      Qt::CaseInsensitive)) {
    on_item_selected(m_menu_list->get_value(0));
  } else {
    setText(m_item_delegate.displayText(m_current_item));
  }
}

void FilteredDropDownMenu::on_item_activated(const QVariant& item) {
  m_last_activated_item = item;
  setText(m_item_delegate.displayText(item));
}

void FilteredDropDownMenu::on_item_selected(const QVariant& item) {
  on_item_selected(item, false);
}

void FilteredDropDownMenu::on_item_selected(const QVariant& item,
    bool emit_finished) {
  m_current_item = item;
  setText(m_item_delegate.displayText(m_current_item));
  if(emit_finished) {
    Q_EMIT editingFinished();
  }
  m_menu_list->hide();
  set_items(m_items);
  update();
}

void FilteredDropDownMenu::on_text_edited(const QString& text) {
  m_last_activated_item = QVariant();
  if(text.isEmpty()) {
    m_menu_list->set_items(std::move(create_widget_items(m_items)));
    m_menu_list->show();
    return;
  }
  auto items = std::move(create_widget_items(m_items, text));
  if(items.empty()) {
    m_menu_list->hide();
    return;
  }
  m_menu_list->set_items(items);
  m_menu_list->show();
}
