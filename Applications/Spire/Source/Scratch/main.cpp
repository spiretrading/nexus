#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QHBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorContextMenu.hpp"

class Widget : public QWidget {
  public:
    Widget() {
      auto layout = new QHBoxLayout(this);
      m_label = new QLabel("Right click to show context menu", this);
      layout->addWidget(m_label);
    }

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
      auto menu = OrderImbalanceIndicatorContextMenu(this);
      menu.connect_export_table_signal([=] { m_label->setText("Export Table"); });
      menu.connect_export_chart_signal([=] { m_label->setText("Export Chart"); });
      menu.connect_reset_signal([=] { m_label->setText("Reset All Filters"); });
      menu.connect_security_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Security column shown");
          } else {
            m_label->setText("Security column hidden");
          }
        });
      menu.connect_side_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Side column shown");
          } else {
            m_label->setText("Side column hidden");
          }
        });
      menu.connect_size_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Size column shown");
          } else {
            m_label->setText("Size column hidden");
          }
        });
      menu.connect_reference_price_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Reference Px column shown");
          } else {
            m_label->setText("Reference Px column hidden");
          }
        });
      menu.connect_date_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Date column shown");
          } else {
            m_label->setText("Date column hidden");
          }
        });
      menu.connect_time_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Time column shown");
          } else {
            m_label->setText("Time column hidden");
          }
        });
      menu.connect_notional_value_toggled_signal([=] (auto b) {
          if(b) {
            m_label->setText("Notional Value column shown");
          } else {
            m_label->setText("Notional Value column hidden");
          }
        });
      menu.exec(event->globalPos());
    }

  private:
    QLabel* m_label;
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = Widget();
  w.resize(600, 300);
  w.show();
  application->exec();
}
