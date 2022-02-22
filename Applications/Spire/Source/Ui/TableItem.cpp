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
  enclose(*this, *m_button);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableItem::on_style, this));
}

const TableItem::Styles& TableItem::get_styles() const {
  return m_styles;
}

connection TableItem::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_button->connect_clicked_signal(slot);
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
