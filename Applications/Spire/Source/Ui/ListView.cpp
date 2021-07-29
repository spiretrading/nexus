#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
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

  struct BodyContainer : QWidget {
    QWidget* m_body;

    BodyContainer(QWidget* body)
        : m_body(body) {
      m_body->setParent(this);
    }

    QSize sizeHint() const override {
      return m_body->sizeHint();
    }

    bool event(QEvent* event) override {
      if(event->type() == QEvent::LayoutRequest) {
        updateGeometry();
      }
      return QWidget::event(event);
    }
  };
}

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
  auto body = new QWidget();
  auto container = new BodyContainer(body);
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  layout->addWidget(new Box(container));
  auto body_layout = new QVBoxLayout();
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto list_item = new ListItem(m_view_builder(*m_list_model, i));
    auto item_layout = new QHBoxLayout();
    item_layout->addWidget(list_item);
    item_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    body_layout->addLayout(item_layout);
  }
  body->setLayout(body_layout);
  setLayout(layout);
  set_style(*this, DEFAULT_STYLE());
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
  m_direction = direction;
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
