#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

TableItem::TableItem(QWidget& body, QWidget* parent)
    : TableItem(parent) {
  mount(body);
}

TableItem::TableItem(QWidget* parent)
    : QWidget(parent),
      m_styles{Qt::transparent, Qt::transparent, Qt::transparent,
        Qt::transparent, Qt::transparent},
      m_click_observer(*this),
      m_focus_observer(*this),
      m_mouse_observer(*this) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = make_hbox_layout(this);
  m_click_observer.connect_click_signal(m_active_signal);
  m_focus_observer.connect_state_signal(
    std::bind_front(&TableItem::on_focus, this));
  m_mouse_observer.connect_mouse_signal(
    std::bind_front(&TableItem::on_mouse, this));
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableItem::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Current()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_color(QColor(0x4B23A0)));
  });
}

TableItem::~TableItem() {
  if(auto item = layout()->takeAt(0)) {
    auto body = item->widget();
    body->setAttribute(Qt::WA_DontShowOnScreen);
    body->setParent(nullptr);
    delete item;
  }
}

const TableItem::Styles& TableItem::get_styles() const {
  return m_styles;
}

const QWidget& TableItem::get_body() const {
  return const_cast<TableItem*>(this)->get_body();
}

QWidget& TableItem::get_body() {
  if(auto item = layout()->itemAt(0)) {
    return *item->widget();
  }
  return *this;
}

connection TableItem::connect_active_signal(
    const ActiveSignal::slot_type& slot) const {
  return m_active_signal.connect(slot);
}

QSize TableItem::sizeHint() const {
  auto& body = get_body();
  if(&body == this) {
    return layout()->sizeHint();
  }
  return body.sizeHint();
}

void TableItem::mount(QWidget& body) {
  setFocusProxy(&body);
  setFocusPolicy(focusPolicy());
  layout()->addWidget(&body);
  updateGeometry();
  body.setAttribute(Qt::WA_DontShowOnScreen, false);
}

QWidget* TableItem::unmount() {
  auto item = layout()->takeAt(0);
  auto body = item->widget();
  body->setAttribute(Qt::WA_DontShowOnScreen);
  delete item;
  return body;
}

void TableItem::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::FOCUS_IN) {
    m_active_signal();
  }
}

void TableItem::on_mouse(QWidget& target, const QMouseEvent& event) {
  if(event.type() == QEvent::MouseButtonPress &&
      event.button() == Qt::MouseButton::LeftButton &&
      m_focus_observer.get_state() == FocusObserver::State::NONE) {
    setFocus(Qt::FocusReason::MouseFocusReason);
    m_active_signal();
  }
}

void TableItem::on_style() {
  auto& stylist = find_stylist(*this);
  m_styles = {Qt::transparent, Qt::transparent, Qt::transparent,
    Qt::transparent, Qt::transparent};
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_background_color = color;
          if(auto row = parentWidget()) {
            if(auto body = row->parentWidget()) {
              body->update();
            }
          }
        });
      },
      [&] (const BorderTopColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_border_top_color = color;
          if(auto row = parentWidget()) {
            if(auto body = row->parentWidget()) {
              body->update();
            }
          }
        });
      },
      [&] (const BorderRightColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_border_right_color = color;
          if(auto row = parentWidget()) {
            if(auto body = row->parentWidget()) {
              body->update();
            }
          }
        });
      },
      [&] (const BorderBottomColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_border_bottom_color = color;
          if(auto row = parentWidget()) {
            if(auto body = row->parentWidget()) {
              body->update();
            }
          }
        });
      },
      [&] (const BorderLeftColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_border_left_color = color;
          if(auto row = parentWidget()) {
            if(auto body = row->parentWidget()) {
              body->update();
            }
          }
        });
      });
  }
}
