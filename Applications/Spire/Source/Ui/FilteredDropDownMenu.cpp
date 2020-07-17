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
  m_list_selection_connection = m_menu_list->connect_selected_signal(
    [=] (const auto& value) { on_item_selected(value); });
  set_items(items);
  installEventFilter(this);
  m_menu_list->installEventFilter(this);
}

bool FilteredDropDownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == this) {
    if(event->type() == QEvent::KeyPress &&
        m_menu_list->isVisible()) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Escape) {
        clear();
      } else if(m_menu_list->isVisible()) {
        if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return &&
            !m_last_activated_item.isValid()) {
          auto item = std::move(m_menu_list->get_value(0));
          if(item.isValid()) {
            m_list_selection_connection.disconnect();
            on_item_selected(item);
          }
        }
      }
    }
  } else if(watched == m_menu_list) {
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
        auto painter = QPainter(this);
        auto font = QFont("Roboto");
        font.setPixelSize(scale_height(12));
        painter.setFont(font);
        auto metrics = QFontMetrics(font);
        painter.fillRect(cursorRect().right() - scale_width(2),
          (height() / 2) - ((metrics.ascent() + scale_height(4)) / 2) - 1,
          metrics.horizontalAdvance(item_text),
          metrics.ascent() + scale_height(4), QColor("#0078D7"));
        painter.setPen(Qt::white);
        painter.drawText(QPoint(cursorRect().right() - scale_width(2),
          (height() / 2) + (metrics.ascent() / 2) - 1), item_text);
      }
    }
  }
  if(m_menu_list->isActiveWindow()) {
    auto painter = QPainter(this);
    draw_border(QColor("#4B23A0"), painter);
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

void FilteredDropDownMenu::draw_border(const QColor& color,
    QPainter& painter) {
  painter.save();
  painter.setPen(color);
  painter.drawRect(0, 0, width() - 1, height() - 1);
  painter.restore();
}

void FilteredDropDownMenu::on_editing_finished() {
  if(text() == m_item_delegate.displayText(m_current_item)) {
    return;
  }
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
  auto item = m_menu_list->get_value(0);
  if(item.isValid() && !text().isEmpty()) {
    on_item_selected(item);
  } else {
    setText(m_item_delegate.displayText(m_current_item));
  }
}

void FilteredDropDownMenu::on_item_activated(const QVariant& item) {
  m_last_activated_item = item;
  setText(m_item_delegate.displayText(item));
}

void FilteredDropDownMenu::on_item_selected(const QVariant& item) {
  m_current_item = item;
  setText(m_item_delegate.displayText(m_current_item));
  m_selected_signal(m_current_item);
  clear();
  m_list_selection_connection = m_menu_list->connect_selected_signal(
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
