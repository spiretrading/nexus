#include "Spire/Ui/EmptyState.hpp"
#include <QApplication>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_empty_label(const QString& name) {
    auto label = make_label(name);
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    font.setItalic(true);
    update_style(*label, [&] (auto& style) {
      style.get(Any()).
        set(TextAlign(Qt::AlignCenter)).
        set(text_style(font, QColor(0xA0A0A0))).
        set(horizontal_padding(scale_width(9))).
        set(vertical_padding(scale_height(6)));
    });
    return label;
  }
}

EmptyState::EmptyState(QString label, QWidget& parent)
    : QWidget(&parent),
      m_panel_horizontal_border_width(0),
      m_click_observer(*this) {
  auto empty_label = make_empty_label(label);
  enclose(*this, *empty_label);
  proxy_style(*this, *empty_label);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowFlags(Qt::Popup | (m_panel->windowFlags() & ~Qt::Tool));
  m_panel->set_closed_on_focus_out(true);
  m_panel->installEventFilter(this);
  on_panel_style();
  m_click_observer.connect_click_signal(
    std::bind_front(&EmptyState::on_click, this));
  m_panel_style_connection = connect_style_signal(*m_panel,
    std::bind_front(&EmptyState::on_panel_style, this));
  parent.installEventFilter(this);
}

QSize EmptyState::sizeHint() const {
  if(!m_size_hint) {
    auto width = std::max(m_panel->parentWidget()->size().width() -
      m_panel_horizontal_border_width, layout()->sizeHint().width());
    m_size_hint.emplace(width, layout()->sizeHint().height());
  }
  return *m_size_hint;
}

bool EmptyState::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

bool EmptyState::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel->parentWidget()) {
    if(event->type() == QEvent::Resize ||
        event->type() == QEvent::LayoutRequest) {
      m_size_hint = none;
      updateGeometry();
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Tab || key_event.key() == Qt::Key_Backtab) {
      hide();
      QApplication::sendEvent(window()->parentWidget(), event);
    } else if(key_event.key() == Qt::Key_Space ||
        key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void EmptyState::on_click() {
  hide();
}

void EmptyState::on_panel_style() {
  m_panel_horizontal_border_width = 0;
  auto& stylist = find_stylist(*m_panel);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_border_width += size;
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_border_width += size;
        });
      });
  }
}
