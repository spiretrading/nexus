#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

TableItem::TableItem(QWidget& component, QWidget* parent)
    : QWidget(parent) {
  m_styles.m_background_color = Qt::transparent;
  m_button = new Button(&component);
  m_button->connect_click_signal(m_active_signal);
  enclose(*this, *m_button);
  m_focus_observer.emplace(*m_button);
  m_focus_observer->connect_state_signal(
    std::bind_front(&TableItem::on_focus, this));
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableItem::on_style, this));
}

const TableItem::Styles& TableItem::get_styles() const {
  return m_styles;
}

const QWidget& TableItem::get_body() const {
  return m_button->get_body();
}

QWidget& TableItem::get_body() {
  return m_button->get_body();
}

connection TableItem::connect_active_signal(
    const ActiveSignal::slot_type& slot) const {
  return m_active_signal.connect(slot);
}

void TableItem::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::FOCUS_IN) {
    m_active_signal();
  }
}

void TableItem::on_style() {
  auto& stylist = find_stylist(*this);
  m_styles = {};
  m_styles.m_background_color = Qt::transparent;
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
