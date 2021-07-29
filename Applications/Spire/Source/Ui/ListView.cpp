#include "Spire/Ui/ListView.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;

  QWidget* default_view_builder(const ArrayListModel& model, int index) {
    return nullptr;
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
    m_body->setLayout(new QBoxLayout(QBoxLayout::TopToBottom));
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
      m_does_selection_follow_focus(true) {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    m_list_items.push_back(new ListItem(m_view_builder(*m_list_model, i)));
  }
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  m_container = new BodyContainer();
  layout->addWidget(new Box(m_container));
  setLayout(layout);
  set_style(*this, DEFAULT_STYLE());
  update_layout();
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
  m_overflow = overflow;
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

void ListView::update_layout() {
  auto& body_layout = m_container->get_layout();
  while(auto item = body_layout.takeAt(body_layout.count() - 1)) {
    delete item;
  }
  auto [direction, reverse_direction] = [&] {
    if(m_direction == Qt::Orientation::Vertical) {
      return std::tuple(QBoxLayout::TopToBottom, QBoxLayout::LeftToRight);
    }
    return std::tuple(QBoxLayout::LeftToRight, QBoxLayout::TopToBottom);
  }();
  body_layout.setDirection(direction);
  for(auto item : m_list_items) {
    auto item_layout = new QBoxLayout(reverse_direction);
    if(m_direction == Qt::Orientation::Horizontal) {
      item_layout->addSpacerItem(
        new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    }
    item_layout->addWidget(item);
    if(m_direction == Qt::Orientation::Vertical) {
      item_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    }
    body_layout.addLayout(item_layout);
  }
  m_container->m_body->adjustSize();
}
