#include "Spire/Ui/ListView.hpp"
#include <QEvent>
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
  : ListView(std::move(list_model), default_view_builder,
      std::make_shared<LocalCurrentModel>(),
      std::make_shared<LocalSelectionModel>(), parent) {}

ListView::ListView(std::shared_ptr<ArrayListModel> list_model,
  ViewBuilder view_builder, QWidget* parent)
  : ListView(std::move(list_model), std::move(view_builder),
      std::make_shared<LocalCurrentModel>(),
      std::make_shared<LocalSelectionModel>(), parent) {}

ListView::ListView(std::shared_ptr<ArrayListModel> list_model,
    ViewBuilder view_builder, std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<SelectionModel> selection_model, QWidget* parent)
    : QWidget(parent),
      m_list_model(std::move(list_model)),
      m_view_builder(std::move(view_builder)),
      m_current_model(std::move(current_model)),
      m_selection_model(std::move(selection_model)),
      m_direction(Qt::Vertical),
      m_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_selection_mode(SelectionMode::SINGLE),
      m_does_selection_follow_focus(true),
      m_item_gap(DEFAULT_GAP),
      m_overflow_gap(DEFAULT_OVERFLOW_GAP) {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto item = new ListItem(m_view_builder(*m_list_model, i));
    m_list_items.push_back(item);
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

Qt::Orientation ListView::get_direction() const {
  return m_direction;
}

void ListView::set_direction(Qt::Orientation direction) {
  if(direction == m_direction) {
    return;
  }
  m_direction = direction;
  update_layout();
}

ListView::EdgeNavigation ListView::get_edge_navigation() const {
  return m_navigation;
}

void ListView::set_edge_navigation(EdgeNavigation navigation) {
  m_navigation = navigation;
}

ListView::Overflow ListView::get_overflow() const {
  return m_overflow;
}

void ListView::set_overflow(Overflow overflow) {
  if(overflow == m_overflow) {
    return;
  }
  m_overflow = overflow;
  update_layout();
}

ListView::SelectionMode ListView::get_selection_mode() const {
  return m_selection_mode;
}

void ListView::set_selection_mode(SelectionMode selection_mode) {
  m_selection_mode = selection_mode;
}

bool ListView::does_selection_follow_focus() const {
  return m_does_selection_follow_focus;
}

void ListView::set_selection_follow_focus(bool does_selection_follow_focus) {
  m_does_selection_follow_focus = does_selection_follow_focus;
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

void ListView::update_layout() {
  auto& body_layout = m_container->get_layout();
  while(auto item = body_layout.takeAt(body_layout.count() - 1)) {
    delete item;
  }
  auto direction = [&] {
    if(m_direction == Qt::Orientation::Vertical) {
      return QBoxLayout::LeftToRight;
    }
    return QBoxLayout::TopToBottom;
  }();
  body_layout.setDirection(direction);
  body_layout.setSpacing(m_overflow_gap);
  auto max_size = [&] {
    if(m_overflow == Overflow::NONE) {
      return QWIDGETSIZE_MAX;
    } else if(m_direction == Qt::Orientation::Vertical) {
      return m_container->height();
    }
    return m_container->width();
  }();
  auto i = m_list_items.begin();
  while(i != m_list_items.end()) {
    auto remaining_size = max_size;
    auto inner_layout = new QBoxLayout(reverse(direction));
    inner_layout->setContentsMargins({});
    inner_layout->setSpacing(m_item_gap);
    while(i != m_list_items.end()) {
      auto item_size = [&] {
        if(m_direction == Qt::Orientation::Vertical) {
          return (*i)->height();
        } else {
          return (*i)->width();
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
        item_layout->addSpacerItem(
          new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
      }
      item_layout->addWidget(*i);
      if(m_direction == Qt::Orientation::Vertical) {
        item_layout->addSpacerItem(
          new QSpacerItem(0, 0, QSizePolicy::Expanding));
      }
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
      });
  }
  if(has_update) {
    update_layout();
  }
}
