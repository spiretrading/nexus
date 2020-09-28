#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"
#include <algorithm>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/SecurityInput/SecurityInfoWidget.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto ICON_HEIGHT() {
    static auto height = scale_height(10);
    return height;
  }

  auto ICON_WIDTH() {
    static auto width = scale_width(10);
    return width;
  }

  const auto& SEARCH_ICON() {
    static auto icon = imageFromSvg(":/Icons/search.svg",
      {ICON_HEIGHT(), ICON_WIDTH()});
    return icon;
  }
}

SecurityInputLineEdit::SecurityInputLineEdit(Security security,
    Beam::Ref<SecurityInputModel> model, bool is_icon_visible, QWidget* parent)
    : SecurityInputLineEdit("", model, is_icon_visible, parent) {
  m_security = std::move(security);
}

SecurityInputLineEdit::SecurityInputLineEdit(const QString& initial_text,
    Beam::Ref<SecurityInputModel> model, bool is_icon_visible, QWidget* parent)
    : TextInputWidget(initial_text, parent),
      m_model(model.Get()),
      m_is_icon_visible(is_icon_visible),
      m_is_suggestion_disabled(false),
      m_last_key(Qt::Key_unknown) {
  setText(initial_text);
  parent->installEventFilter(this);
  connect(this, &TextInputWidget::textEdited, this,
    &SecurityInputLineEdit::on_text_edited);
  m_securities = new DropDownList({}, false, this);
  m_securities->findChild<ScrollArea*>()->setFocusProxy(this);
  m_securities->connect_activated_signal(
    [=] (auto& s) { on_activated(s.value<Security>()); });
  m_securities->connect_value_selected_signal([=] (auto& s) {
    on_commit(s.value<Security>());
  });
  m_securities->setVisible(false);
  window()->installEventFilter(this);
}

const Nexus::Security& SecurityInputLineEdit::get_security() const {
  return m_security;
}

bool SecurityInputLineEdit::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowDeactivate &&
        !m_securities->isActiveWindow()) {
      m_securities->hide();
    }
  } else if(watched == parent()) {
    if(event->type() == QEvent::Wheel) {
      m_securities->hide();
    }
  }
  return TextInputWidget::eventFilter(watched, event);
}

void SecurityInputLineEdit::focusInEvent(QFocusEvent* event) {
  TextInputWidget::focusInEvent(event);
  setCursorPosition(text().length());
}

void SecurityInputLineEdit::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Backspace:
      //if(m_last_key != Qt::Key_Backspace && m_securities->isVisible() &&
      //    selectionLength() > 0) {
      //  m_last_key = static_cast<Qt::Key>(event->key());
      //  m_is_suggestion_disabled = true;
      //  m_securities->clear_active_item();
      //  update();
      //  return;
      //}
      if(!text().isEmpty()) {
        m_is_suggestion_disabled = true;
      }
      if(selectionLength() > 0) {
        setText(text().remove(selectedText()));
        m_last_key = static_cast<Qt::Key>(event->key());
        m_securities->clear_active_item();
        return;
      }
      break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      {
        auto current_security = ParseSecurity(text().toUpper().toStdString());
        if(!current_security.GetSymbol().empty()) {
          on_commit(current_security);
        } else {
          on_commit(m_security);
        }
        return;
      }
    case Qt::Key_Delete:
      clear();
      m_securities->hide();
      m_is_suggestion_disabled = false;
      return;
  }
  TextInputWidget::keyPressEvent(event);
  m_last_key = static_cast<Qt::Key>(event->key());
}

void SecurityInputLineEdit::paintEvent(QPaintEvent* event) {
  TextInputWidget::paintEvent(event);
  //if(!text().isEmpty() && !m_is_suggestion_disabled &&
  //    m_last_key != Qt::Key_Backspace) {
  //  auto& item = m_securities->get_value(0);
  //  if(item.isValid()) {
  //    auto item_text = m_item_delegate.displayText(item);
  //    if(item_text.startsWith(text(), Qt::CaseInsensitive) &&
  //        item_text.length() != text().length()) {
  //      item_text = item_text.remove(0, text().length());
  //      auto painter = QPainter(this);
  //      auto font = QFont("Roboto");
  //      font.setPixelSize(scale_height(12));
  //      painter.setFont(font);
  //      auto metrics = QFontMetrics(font);
  //      auto highlight_x_pos = metrics.horizontalAdvance(text()) +
  //        get_padding() + scale_width(3);
  //      painter.fillRect(highlight_x_pos,
  //        (height() / 2) - ((metrics.ascent() + scale_height(4)) / 2) - 1,
  //        metrics.horizontalAdvance(item_text),
  //        metrics.ascent() + scale_height(4), QColor("#0078D7"));
  //      painter.setPen(Qt::white);
  //      painter.drawText(QPoint(highlight_x_pos,
  //        (height() / 2) + (metrics.ascent() / 2) - 1), item_text);
  //    }
  //  }
  //}
  if(m_is_icon_visible) {
    auto painter = QPainter(this);
    painter.drawImage(width() - ICON_WIDTH() - scale_width(8),
      height() - (height() / 2) - (ICON_HEIGHT() / 2), SEARCH_ICON());
  }
}

void SecurityInputLineEdit::showEvent(QShowEvent* event) {
  on_text_edited();
  m_securities->setFixedWidth(std::max(width(), scale_width(142)));
}

void SecurityInputLineEdit::on_activated(const Security& security) {
  setText(m_item_delegate.displayText(QVariant::fromValue(security)));
  m_security = ParseSecurity(text().toUpper().toStdString());
}

void SecurityInputLineEdit::on_commit(const Security& security) {
  m_security = security;
  Q_EMIT editingFinished();
}

void SecurityInputLineEdit::on_text_edited() {
  if(text().isEmpty()) {
    m_is_suggestion_disabled = false;
  }
  m_completions = m_model->autocomplete(text().toStdString());
  m_completions.then([=] (auto result) {
    auto completions = [&] {
      try {
        return result.Get();
      } catch(const std::exception&) {
        return std::vector<SecurityInfo>();
      }
    }();
    auto widget_items = std::vector<DropDownItem*>(completions.size());
    std::transform(completions.begin(), completions.end(),
      widget_items.begin(), [&] (const auto& item) {
        auto item_widget = new SecurityInfoWidget(item, this);
        return item_widget;
      });
    m_securities->set_items(widget_items);
    if(widget_items.empty()) {
      m_securities->hide();
    } else {
      m_securities->setVisible(true);
      m_securities->raise();
    }
    if(auto suggestion = m_securities->get_value(0); suggestion.isValid() &&
        !m_is_suggestion_disabled) {
      auto text_length = text().length();
      setText(m_item_delegate.displayText(suggestion));
      setSelection(text_length, text().length());
      //qDebug() << text();
      //qDebug() << m_last_key;
    }
    if(m_is_suggestion_disabled) {
      m_securities->clear_active_item();
    }
  });
  auto current_security = ParseSecurity(text().toUpper().toStdString());
  if(!current_security.GetSymbol().empty()) {
    m_security = current_security;
  }
}
