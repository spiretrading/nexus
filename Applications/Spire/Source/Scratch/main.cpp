#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

using PriceLevel = StateSelector<void, struct PriceLevelTag>;

struct FTableItem : QWidget {
  FTableItem() {
    auto layout = make_hbox_layout(this);
  }

  void mount() {
    auto body = make_label(QString("Hello"));
    match(*body, PriceLevel());
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

  FRowCover(QWidget* body)
      : QWidget(body) {
    setLayout(make_vbox_layout());
    auto item = new FTableItem();
    layout()->addWidget(item);
    update_style(*this, [] (auto& style) {
      auto row_selector =
        ChildSelector(+Any(), ChildSelector(Any(), PriceLevel()));
      style.get(row_selector).set(BackgroundColor(QColor(0xFF0000)));
      style.get(ChildSelector(row_selector, ChildSelector(Any(), Any()))).
        set(TextColor(QColor(0x00FF00)));
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
    unmatch(*body, PriceLevel());
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

struct FTableBody : QWidget {
  FRowCover* m_row;

  FTableBody() {
    setLayout(make_vbox_layout());
    m_row = new FRowCover(this);
    layout()->addWidget(m_row);
  }

  void add() {
    m_row->mount();
  }

  void remove() {
    m_row->unmount();
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto body = FTableBody();
  body.add();
  body.show();
  auto is_remove = true;
  auto foo = std::function<void ()>();
  foo = [&] {
    if(is_remove) {
      body.remove();
    } else {
      body.add();
    }
    is_remove = !is_remove;
    QTimer::singleShot(1000, foo);
  };
  QTimer::singleShot(1000, foo);
  application.exec();
}
