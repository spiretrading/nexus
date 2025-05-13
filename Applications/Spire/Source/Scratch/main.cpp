#include <QApplication>
#include <QPainter>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

struct FTableItem : QWidget {
  FTableItem() {
    auto layout = make_hbox_layout(this);
  }

  void mount() {
    auto body = make_label(QString("Hello"));
    layout()->addWidget(body);
  }

  QWidget* unmount() {
    auto item = layout()->takeAt(0);
    auto body = item->widget();
    delete item;
    return body;
  }
};

struct FRowCover : QWidget {
  QColor m_background_color;

  FRowCover(QWidget* parent = nullptr)
      : QWidget(parent) {
    setLayout(make_vbox_layout());
    auto item = new FTableItem();
    layout()->addWidget(item);
    update_style(*this, [] (auto& style) {
      auto row_selector =
        ChildSelector(+Any(), ChildSelector(Any(), Any()));
      style.get(row_selector).set(BackgroundColor(QColor(0xFF0000)));
      style.get(ChildSelector(row_selector, ChildSelector(Any(), Any())));
    });
    connect_style_signal(*this, std::bind_front(&FRowCover::on_style, this));
  }

  FTableItem* get_item() {
    return static_cast<FTableItem*>(layout()->itemAt(0)->widget());
  }

  void mount() {
    get_item()->mount();
  }

  void unmount() {
    auto item = get_item();
    auto body = item->unmount();
    delete body;
  }

  void paintEvent(QPaintEvent* event) override {
    auto painter = QPainter(this);
    painter.fillRect(QRect(QPoint(0, 0), size()), m_background_color);
  }

  void on_style() {
    auto& stylist = find_stylist(*this);
    m_background_color = Qt::transparent;
    for(auto& property : stylist.get_computed_block()) {
      property.visit(
        [&] (const BackgroundColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_background_color = color;
          });
        });
    }
    update();
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto row = new FRowCover();
  row->mount();
  row->show();
  auto is_remove = true;
  auto foo = std::function<void ()>();
  foo = [&] {
    if(is_remove) {
      row->unmount();
    } else {
      row->mount();
    }
    is_remove = !is_remove;
    QTimer::singleShot(1000, foo);
  };
  QTimer::singleShot(1000, foo);
  application.exec();
}
