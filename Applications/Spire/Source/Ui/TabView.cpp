#include "Spire/Ui/TabView.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

struct Tab : QWidget {
  Tab(std::shared_ptr<ListModel<QString>> labels) {
    setMinimumWidth(scale_width(54));
    setMaximumWidth(scale_width(160));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto label = new ResponsiveLabel(std::move(labels));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).
        set(PaddingLeft(scale_width(8))).
        set(PaddingRight(scale_width(2))).
        set(TextColor(QColor(0x808080)));
    });
    auto divider = new Box(nullptr);
    divider->setFixedSize(scale(1, 14));
    auto divider_layout = new QVBoxLayout();
    divider_layout->setSpacing(0);
    divider_layout->setContentsMargins({});
    divider_layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    divider_layout->addWidget(divider);
    divider_layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto body_layout = new QHBoxLayout(body);
    body_layout->setSpacing(0);
    body_layout->setContentsMargins({});
    body_layout->addWidget(label);
    body_layout->addLayout(divider_layout);
    auto box = new Box(body);
    box->setSizePolicy(
      QSizePolicy::Expanding, box->sizePolicy().verticalPolicy());
    update_style(*box, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xEBEBEB))).
        set(border_size(scale_width(1))).
        set(border_color(QColor(Qt::transparent)));
      style.get(Hover()).set(BackgroundColor(QColor(0xE0E0E0)));
    });
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    layout->addWidget(box);
    proxy_style(*this, *box);
  }
};

TabView::TabView(QWidget* parent) {
  auto tabs = std::make_shared<ArrayListModel<QString>>();
  tabs->push("Hello");
  tabs->push("Goodbye");
  auto tab_list = new ListView(tabs,
    [] (const auto& model, auto index) {
      auto labels = std::make_shared<ArrayListModel<QString>>();
      labels->push(std::any_cast<QString>(model->get(index)));
      return new Tab(labels);
    });
  update_style(*tab_list, [] (auto& style) {
    style.get(Any()).
      set(Qt::Orientation::Horizontal).
      set(EdgeNavigation::WRAP);
  });
  auto scrollable_list_box = new ScrollableListBox(*tab_list);
  scrollable_list_box->setFixedHeight(scale_height(26));
  scrollable_list_box->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*scrollable_list_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(border_size(0));
  });
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->addWidget(scrollable_list_box);
}
