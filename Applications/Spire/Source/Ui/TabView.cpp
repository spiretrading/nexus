#include "Spire/Ui/TabView.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace boost;
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
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto body_layout = new QHBoxLayout(body);
    body_layout->setSpacing(0);
    body_layout->setContentsMargins({});
    body_layout->addWidget(label);
    body_layout->addWidget(divider, 0, Qt::AlignVCenter);
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

TabView::TabView(QWidget* parent)
    : m_labels(std::make_shared<ArrayListModel<std::vector<QString>>>()) {
  m_tab_list = new ListView(m_labels,
    [] (const std::shared_ptr<ListModel<std::vector<QString>>>& model,
        auto index) {
      auto& labels = model->get(index);
      auto labels_model = std::make_shared<ArrayListModel<QString>>();
      for(auto& label : labels) {
        labels_model->push(label);
      }
      return new Tab(std::move(labels_model));
    });
  auto scrollable_list_box = new ScrollableListBox(*m_tab_list);
  scrollable_list_box->setFixedHeight(scale_height(26));
  scrollable_list_box->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*scrollable_list_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(border_size(0));
  });
  update_style(*m_tab_list, [] (auto& style) {
    style.get(Any()).
      set(Qt::Orientation::Horizontal).
      set(EdgeNavigation::WRAP);
    style.get(Any() >>
        (is_a<ListItem>() && (Any() || Hover() || Current() || Selected()))).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(padding(0)).
      set(border_size(0));
  });
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->addWidget(scrollable_list_box);
  m_tab_list->get_current()->connect_update_signal(
    std::bind_front(&TabView::on_current, this));
}

void TabView::add(const QString& label, QWidget& body) {
  add(std::vector{label}, body);
}

void TabView::add(std::vector<QString> labels, QWidget& body) {
  m_bodies.push_back(&body);
  m_labels->push(labels);
  if(!m_tab_list->get_current()->get()) {
    m_tab_list->get_current()->set(0);
  }
}

void TabView::on_current(optional<int> current) {
  auto layout = static_cast<QVBoxLayout*>(this->layout());
  if(layout->count() > 1) {
    auto item = layout->itemAt(1);
    layout->removeItem(item);
    item->widget()->hide();
    delete item;
  }
  if(current) {
    layout->addWidget(m_bodies[*current], 0, Qt::AlignTop | Qt::AlignLeft);
    m_bodies[*current]->show();
  }
}
