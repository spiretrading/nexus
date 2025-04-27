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
    updateGeometry();
    body->setAttribute(Qt::WA_DontShowOnScreen, false);
  }

  QWidget* unmount() {
    auto item = layout()->takeAt(0);
    auto body = item->widget();
    body->setAttribute(Qt::WA_DontShowOnScreen);
    delete item;
    return body;
  }
};

using FRow = StateSelector<void, struct FRowTag>;

struct FRowCover : QWidget {
  QColor m_background_color;

  FRowCover(QWidget* body)
      : QWidget(body) {
    setLayout(make_vbox_layout());
    match(*this, FRow());
    auto item = new FTableItem();
    layout()->addWidget(item);
  }

  FTableItem* get_item() {
    return static_cast<FTableItem*>(layout()->itemAt(0)->widget());
  }

  void mount() {
    get_item()->mount();
  }

  void unmount() {
    move(-10000, -10000);
    auto item = get_item();
    auto body = item->unmount();
    delete body;
  }

  void paintEvent(QPaintEvent* event) override {
    auto painter = QPainter(this);
    painter.fillRect(QRect(QPoint(0, 0), size()), m_background_color);
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
    QTimer::singleShot(0, this, [=] {
      if(std::find(m_recycled_rows.begin(), m_recycled_rows.end(), row) !=
          m_recycled_rows.end()) {
        row->hide();
      }
    });
  }

  FRowCover* make_row_cover() {
    if(m_recycled_rows.empty()) {
      auto row = new FRowCover(this);
      connect_style_signal(*row,
        std::bind_front(&FTableBody::on_cover_style, this, std::ref(*row)));
      return row;
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
    on_cover_style(*row);
    return row;
  }

  void add_row() {
    mount_row();
  }

  void on_cover_style(FRowCover& cover) {
    auto& stylist = find_stylist(cover);
    cover.m_background_color = Qt::transparent;
    for(auto& property : stylist.get_computed_block()) {
      property.visit(
        [&] (const BackgroundColor& color) {
          stylist.evaluate(color, [=, &cover] (auto color) {
            cover.m_background_color = color;
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
  auto window = QWidget();
  auto body = new FTableBody();
  enclose(window, *body);
  update_style(*body, [] (auto& style) {
    style.get(Any() > +FRow() > Any() > Any()).
      set(BackgroundColor(QColor(0xFF0000)));
    style.get(ChildSelector(Any() > +FRow() > Any() > Any(), Any() > Any())).
      set(TextColor(QColor(0x00FF00)));
  });
  body->add();
  window.show();
  QTimer::singleShot(1000, [&] {
    body->remove();
    QTimer::singleShot(1000, [&] {
      body->add();
    });
  });
  application.exec();
}
