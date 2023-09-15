#include "Spire/Ui/TabView.hpp"
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using PrecedesCurrent = StateSelector<void, struct PrecedesCurrentTag>;

  struct Tab : QWidget {
    Tab(std::shared_ptr<ListModel<QString>> labels) {
      setMinimumWidth(scale_width(54));
      setMaximumWidth(scale_width(160));
      setFixedHeight(scale_height(26));
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      auto label = new ResponsiveLabel(std::move(labels));
      update_style(*label, [] (auto& style) {
        style.get(Any()).
          set(PaddingLeft(scale_width(8))).
          set(PaddingRight(scale_width(7))).
          set(TextColor(QColor(0x808080)));
      });
      auto divider = new Box(nullptr);
      divider->setFixedSize(scale(1, 14));
      match(*divider, TabView::Divider());
      auto body = new QWidget();
      body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      auto body_layout = make_hbox_layout(body);
      body_layout->addWidget(label);
      body_layout->addWidget(divider, 0, Qt::AlignVCenter);
      auto box = new Box(body);
      enclose(*this, *box);
      proxy_style(*this, *box);
      update_style(*this, [] (auto& style) {
        style.get(Any() > TabView::Divider()).
          set(BackgroundColor(QColor(0xC8C8C8)));
      });
    }
  };
}

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
  m_tab_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_tab_list->set_item_size_policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto scrollable_list_box = new ScrollableListBox(*m_tab_list);
  scrollable_list_box->get_scroll_box().set(
    ScrollBox::DisplayPolicy::NEVER, ScrollBox::DisplayPolicy::NEVER);
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
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(padding(0)).
      set(border_size(0));
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(BorderTopSize(scale_height(1))).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > (is_a<ListItem>() && (Current() || Hover())) >
        is_a<ResponsiveLabel>()).set(TextColor(QColor(Qt::black)));
    style.get(Any() > (is_a<ListItem>() && Hover())).
      set(BackgroundColor(QColor(0xE0E0E0)));
    style.get(Any() > (is_a<ListItem>() && Current())).
      set(BackgroundColor(QColor(0xFFFFFF)));
    style.get(Any() > (is_a<ListItem>() && (Current() || PrecedesCurrent())) >
      is_a<Tab>() > TabView::Divider()).set(Visibility::INVISIBLE);
  });
  update_style(*this, [] (auto& style) {
    style.get(FocusIn() > (is_a<ListItem>() && Current())).
      set(BorderTopColor(QColor(0x4B23A0)));
  });
  enclose(*this, *scrollable_list_box, Qt::AlignTop);
  m_tab_list->get_current()->connect_update_signal(
    std::bind_front(&TabView::on_current, this));
}

void TabView::add(const QString& label, QWidget& body) {
  add(std::vector{label}, body);
}

void TabView::add(std::vector<QString> labels, QWidget& body) {
  m_bodies.emplace_back(&body);
  m_labels->push(labels);
  if(!m_tab_list->get_current()->get()) {
    m_tab_list->get_current()->set(0);
  }
  updateGeometry();
}

QSize TabView::sizeHint() const {
  auto max_hint = QSize(0, 0);
  for(auto& body : m_bodies) {
    max_hint.rwidth() = std::max(max_hint.width(), body->sizeHint().width());
    max_hint.rheight() = std::max(max_hint.height(), body->sizeHint().height());
  }
  return m_tab_list->sizeHint() + max_hint;
}

void TabView::keyPressEvent(QKeyEvent* event) {
  auto direction = [&] {
    if(event->modifiers() == Qt::ControlModifier) {
      if(event->key() == Qt::Key_Tab) {
        return 1;
      } else if(event->key() == Qt::Key_Backtab) {
        return -1;
      }
    } else if(event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
      if(event->key() == Qt::Key_Backtab) {
        return -1;
      }
    }
    return 0;
  }();
  if(direction == 0) {
    return QWidget::keyPressEvent(event);
  }
  auto& current = m_tab_list->get_current();
  if(!current->get()) {
    if(m_tab_list->get_list()->get_size() > 0) {
      current->set(0);
    }
  } else if(direction == 1) {
    current->set((*current->get() + 1) % m_tab_list->get_list()->get_size());
  } else if(*current->get() == 0) {
    current->set(m_tab_list->get_list()->get_size() - 1);
  } else {
    current->set(*current->get() - 1);
  }
}

void TabView::on_current(optional<int> current) {
  if(m_current && *m_current > 0) {
    unmatch(*m_tab_list->get_list_item(*m_current - 1), PrecedesCurrent());
  }
  m_current = current;
  auto layout = static_cast<QBoxLayout*>(this->layout());
  if(layout->count() > 1) {
    auto item = layout->itemAt(1);
    layout->removeItem(item);
    item->widget()->hide();
    delete item;
  }
  if(current) {
    layout->addWidget(m_bodies[*current].get());
    m_bodies[*current]->show();
    if(*current > 0) {
      match(*m_tab_list->get_list_item(*m_current - 1), PrecedesCurrent());
    }
  }
}
