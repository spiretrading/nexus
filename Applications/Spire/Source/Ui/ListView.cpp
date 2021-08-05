#include "Spire/Ui/ListView.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;

  QWidget* default_view_builder(const ArrayListModel& model, int index) {
    return make_label(displayTextAny(model.at(index)));
  }

  auto reverse(QBoxLayout::Direction direction) {
    if(direction == QBoxLayout::TopToBottom) {
      return QBoxLayout::LeftToRight;
    }
    return QBoxLayout::TopToBottom;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(ListItemGap(scale_height(DEFAULT_GAP))).
      set(ListOverflowGap(scale_width(DEFAULT_OVERFLOW_GAP)));
    return style;
  }
}

struct ListView::BodyContainer : QWidget {
  QWidget* m_body;

  BodyContainer()
      : m_body(new QWidget(this)) {
    auto layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins({});
    m_body->setLayout(layout);
  }

  QBoxLayout& get_layout() {
    return *static_cast<QBoxLayout*>(m_body->layout());
  }

  QSize sizeHint() const override {
    if(m_body) {
      return m_body->sizeHint();
    }
    return QSize();
  }

  bool event(QEvent* event) override {
    if(event->type() == QEvent::LayoutRequest) {
      updateGeometry();
    }
    return QWidget::event(event);
  }
};

ListView::ListView(std::shared_ptr<ArrayListModel> list_model, QWidget* parent)
  : ListView(std::move(list_model), default_view_builder, parent) {}

ListView::ListView(std::shared_ptr<ArrayListModel> list_model,
  ViewBuilder view_builder, QWidget* parent)
  : ListView(std::move(list_model), std::move(view_builder),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<LocalValueModel<optional<std::any>>>(), parent) {}

ListView::ListView(std::shared_ptr<ArrayListModel> list_model,
    ViewBuilder view_builder, std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<SelectionModel> selection_model, QWidget* parent)
    : QWidget(parent),
      m_list_model(std::move(list_model)),
      m_view_builder(std::move(view_builder)),
      m_current_model(std::move(current_model)),
      m_selection_model(std::move(selection_model)),
      m_direction(Qt::Vertical),
      m_edge_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_selection_mode(SelectionMode::SINGLE),
      m_item_gap(DEFAULT_GAP),
      m_overflow_gap(DEFAULT_OVERFLOW_GAP) {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto item = new ListItem(m_view_builder(*m_list_model, i));
    m_items.emplace_back(new ItemEntry{item, i});
    item->connect_current_signal([=, item = m_items.back().get()] {
      on_current(*item);
    });
    if(m_selection_model->get_current() &&
        is_equal(*m_selection_model->get_current(), m_list_model->at(i))) {
      item->set_selected(true);
    }
  }
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  m_container = new BodyContainer();
  m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_box = new Box(m_container);
  layout->addWidget(m_box);
  setLayout(layout);
  set_style(*this, DEFAULT_STYLE());
  m_container->installEventFilter(this);
  update_layout();
  proxy_style(*this, *m_box);
  connect_style_signal(*this, [=] { on_style(); });
}

const std::shared_ptr<ArrayListModel>& ListView::get_list_model() const {
  return m_list_model;
}

const std::shared_ptr<ListView::CurrentModel>&
    ListView::get_current_model() const {
  return m_current_model;
}

const std::shared_ptr<ListView::SelectionModel>&
    ListView::get_selection_model() const {
  return m_selection_model;
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize ListView::sizeHint() const {
  return m_container->m_body->size() + m_box->size() - m_container->size();
}

bool ListView::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    update_layout();
  }
  return QWidget::eventFilter(watched, event);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
    case Qt::Key_PageUp:
      navigate_home();
      break;
    case Qt::Key_End:
    case Qt::Key_PageDown:
      navigate_end();
      break;
    case Qt::Key_Up:
      if(m_direction == Qt::Orientation::Vertical) {
        navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        cross_previous();
      }
      break;
    case Qt::Key_Down:
      if(m_direction == Qt::Orientation::Vertical) {
        navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        cross_next();
      }
      break;
    case Qt::Key_Left:
      if(m_direction == Qt::Orientation::Horizontal) {
        navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        cross_previous();
      }
      break;
    case Qt::Key_Right:
      if(m_direction == Qt::Orientation::Horizontal) {
        navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        cross_next();
      }
      break;
    default:
      QWidget::keyPressEvent(event);
      break;
  }
}

void ListView::navigate_home() {
  navigate(1, -1, EdgeNavigation::CONTAIN);
}

void ListView::navigate_end() {
  navigate(-1, static_cast<int>(m_items.size()), EdgeNavigation::CONTAIN);
}

void ListView::navigate_next() {
  navigate(1, m_current_model->get_current().value_or(-1), m_edge_navigation);
}

void ListView::navigate_previous() {
  navigate(-1, m_current_model->get_current().value_or(-1), m_edge_navigation);
}

void ListView::navigate(
    int direction, int start, EdgeNavigation edge_navigation) {
  if(m_items.empty()) {
    return;
  }
  auto i = start;
  do {
    i += direction;
    if(i < 0 || i >= static_cast<int>(m_items.size())) {
      if(edge_navigation == EdgeNavigation::CONTAIN) {
        return;
      } else if(direction == -1) {
        i = static_cast<int>(m_items.size()) - 1;
      } else {
        i = 0;
      }
    }
  } while(i != start && !m_items[i]->m_item->isEnabled());
  if(i == m_current_model->get_current()) {
    return;
  }
  m_items[i]->m_item->setFocus();
}

void ListView::cross_previous() {
  cross(-1);
}

void ListView::cross_next() {
  cross(1);
}

void ListView::cross(int direction) {
  if(m_items.empty()) {
    return;
  }
  if(m_navigation_box.isNull()) {
    if(auto current = m_current_model->get_current()) {
      m_navigation_box = m_items[*current]->m_item->frameGeometry();
    } else {
      m_navigation_box = m_items.front()->m_item->frameGeometry();
    }
  }
  auto i = m_current_model->get_current().value_or(-1) + direction;
  auto navigation_box = m_navigation_box;
  auto candidate = -1;
  while(i >= 0 && i != static_cast<int>(m_items.size())) {
    if(m_items[i]->m_item->isEnabled()) {
      if(candidate == -1) {
        if(m_direction == Qt::Orientation::Horizontal) {
          navigation_box.moveTop(m_items[i]->m_item->pos().y());
        } else {
          navigation_box.moveLeft(m_items[i]->m_item->pos().x());
        }
      }
      if((m_direction == Qt::Orientation::Horizontal &&
          direction * m_navigation_box.y() < direction * navigation_box.y() ||
          m_direction == Qt::Orientation::Vertical &&
          direction * m_navigation_box.x() < direction * navigation_box.x()) &&
          navigation_box.intersects(m_items[i]->m_item->frameGeometry())) {
        candidate = i;
        if(direction == 1) {
          break;
        }
      } else if(candidate != -1) {
        break;
      }
    }
    i += direction;
  }
  if(candidate == -1 || candidate == m_current_model->get_current()) {
    return;
  }
  m_items[candidate]->m_item->setFocus();
  m_navigation_box = navigation_box;
}

void ListView::update_layout() {
  auto& body_layout = m_container->get_layout();
  while(auto item = body_layout.takeAt(body_layout.count() - 1)) {
    delete item;
  }
  auto direction = [&] {
    if(m_direction == Qt::Orientation::Horizontal) {
      return QBoxLayout::TopToBottom;
    }
    return QBoxLayout::LeftToRight;
  }();
  body_layout.setDirection(direction);
  body_layout.setSpacing(m_overflow_gap);
  auto max_size = [&] {
    if(m_overflow == Overflow::NONE) {
      return QWIDGETSIZE_MAX;
    } else if(m_direction == Qt::Orientation::Horizontal) {
      return m_container->width();
    }
    return m_container->height();
  }();
  auto i = m_items.begin();
  while(i != m_items.end()) {
    auto remaining_size = max_size;
    auto inner_layout = new QBoxLayout(reverse(direction));
    inner_layout->setContentsMargins({});
    inner_layout->setSpacing(m_item_gap);
    while(i != m_items.end()) {
      auto item_size = [&] {
        if(m_direction == Qt::Orientation::Horizontal) {
          return (*i)->m_item->width();
        } else {
          return (*i)->m_item->height();
        }
      }();
      remaining_size -= item_size;
      if(remaining_size < 0 && remaining_size + item_size != max_size) {
        break;
      }
      remaining_size -= inner_layout->spacing();
      auto item_layout = new QBoxLayout(direction);
      item_layout->setContentsMargins({});
      if(m_direction == Qt::Orientation::Horizontal) {
        (*i)->m_item->setSizePolicy(
          QSizePolicy::Preferred, QSizePolicy::Expanding);
      } else {
        (*i)->m_item->setSizePolicy(
          QSizePolicy::Expanding, QSizePolicy::Preferred);
      }
      item_layout->addWidget((*i)->m_item);
      inner_layout->addLayout(item_layout);
      ++i;
    }
    inner_layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    body_layout.addLayout(inner_layout);
  }
  m_container->m_body->adjustSize();
  updateGeometry();
}

void ListView::on_current(ItemEntry& item) {
  m_navigation_box = item.m_item->frameGeometry();
  m_current_model->set_current(item.m_index);
}

void ListView::on_style() {
  auto& stylist = find_stylist(*this);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const ListItemGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_item_gap = gap;
          *has_update = true;
        });
      },
      [&] (const ListOverflowGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_overflow_gap = gap;
          *has_update = true;
        });
      },
      [&] (EnumProperty<Qt::Orientation> direction) {
        stylist.evaluate(direction, [=] (auto direction) {
          m_direction = direction;
          *has_update = true;
        });
      },
      [&] (EnumProperty<EdgeNavigation> edge_navigation) {
        stylist.evaluate(edge_navigation, [=] (auto edge_navigation) {
          m_edge_navigation = edge_navigation;
          *has_update = true;
        });
      },
      [&] (EnumProperty<Overflow> overflow) {
        stylist.evaluate(overflow, [=] (auto overflow) {
          m_overflow = overflow;
          *has_update = true;
        });
      },
      [&] (EnumProperty<SelectionMode> selection_mode) {
        stylist.evaluate(selection_mode, [=] (auto selection_mode) {
          m_selection_mode = selection_mode;
          *has_update = true;
        });
      });
  }
  if(has_update) {
    update_layout();
  }
}
