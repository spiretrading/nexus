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

struct FTableBody : QWidget {
  std::deque<FRowCover*> m_recycled_rows;

  FTableBody() {
    setLayout(make_vbox_layout());
  }

  void add() {
    add_row();
  }

  void remove() {
    auto item = layout()->takeAt(0);
    destroy(static_cast<FRowCover*>(item->widget()));
    delete item;
  }

  void destroy(FRowCover* row) {
    row->unmount();
    m_recycled_rows.push_back(row);
    row->hide();
  }

  FRowCover* make_row_cover() {
    if(m_recycled_rows.empty()) {
      return new FRowCover(this);
    }
    auto row = m_recycled_rows.front();
    m_recycled_rows.pop_front();
    return row;
  }

  FRowCover* mount_row() {
    auto row = make_row_cover();
    row->mount();
    layout()->addWidget(row);
    row->show();
    return row;
  }

  void add_row() {
    mount_row();
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto body = FTableBody();
  update_style(body, [] (auto& style) {
    style.get(Any() > +Any() > Any() > PriceLevel()).
      set(BackgroundColor(QColor(0xFF0000)));
    style.get(ChildSelector(
        Any() > +Any() > Any() > PriceLevel(), Any() > Any())).
      set(TextColor(QColor(0x00FF00)));
  });
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
