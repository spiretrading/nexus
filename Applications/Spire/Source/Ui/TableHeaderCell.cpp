#include "Spire/Ui/TableHeaderCell.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Sortable = StateSelector<void, struct SortableTag>;

  struct SortIndicator : QWidget {
    static const QImage& ASCENDING_IMAGE() {
      static auto image =
        imageFromSvg(":/Icons/sort_ascending.svg", scale(5, 6));
      return image;
    }

    static const QImage& DESCENDING_IMAGE() {
      static auto image =
        imageFromSvg(":/Icons/sort_descending.svg", scale(5, 6));
      return image;
    }

    std::shared_ptr<ValueModel<TableHeaderCell::Order>> m_order;
    scoped_connection m_order_connection;

    explicit SortIndicator(
        std::shared_ptr<ValueModel<TableHeaderCell::Order>> order)
        : m_order(std::move(order)) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      on_order(m_order->get());
      m_order_connection = m_order->connect_update_signal(
        std::bind_front(&SortIndicator::on_order, this));
    }

    void on_order(TableHeaderCell::Order order) {
      if(auto previous_icon = layout()->takeAt(0)) {
        delete previous_icon->widget();
        delete previous_icon;
      }
      auto icon = [&] () -> QWidget* {
        if(order == TableHeaderCell::Order::ASCENDING) {
          return new Icon(ASCENDING_IMAGE());
        } else if(order == TableHeaderCell::Order::DESCENDING) {
          return new Icon(DESCENDING_IMAGE());
        } else if(order == TableHeaderCell::Order::NONE) {
          return new Box(nullptr);
        }
        return nullptr;
      }();
      if(icon) {
        icon->setFixedSize(scale(5, 6));
        update_style(*icon, [&] (auto& style) {
          style.get(Any()).set(BackgroundColor(Qt::transparent));
        });
        layout()->addWidget(icon);
      }
    }
  };
}

TableHeaderCell::TableHeaderCell(
    std::shared_ptr<CompositeValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto name_label = make_label(m_model->get(&Model::m_name));
  match(*name_label, Label());
  auto sort_indicator = new SortIndicator(m_model->get(&Model::m_order));
  auto inner_layout = new QHBoxLayout();
  inner_layout->setContentsMargins({});
  inner_layout->addWidget(name_label);
  inner_layout->addWidget(sort_indicator);
  auto hover_element = new Box(nullptr);
  hover_element->setFixedSize(scale(18, 2));
  match(*hover_element, HoverElement());
  auto hover_layout = new QHBoxLayout();
  hover_layout->setContentsMargins({});
  hover_layout->addWidget(hover_element);
  hover_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto outer_layout = new QVBoxLayout(this);
  outer_layout->setContentsMargins({});
  outer_layout->addLayout(inner_layout);
  outer_layout->addLayout(hover_layout);
  auto style = StyleSheet();
  style.get(Any() > Label()).set(TextColor(QColor(0x808080)));
  style.get(Hover() > Label()).set(TextColor(QColor(0x4B23A0)));
  style.get(Any() > HoverElement()).set(Visibility::INVISIBLE);
  style.get((Hover() && Sortable()) > HoverElement()).
    set(BackgroundColor(0x4B23A0)).
    set(Visibility::VISIBLE);
  set_style(*this, std::move(style));
  auto order_model = m_model->get(&Model::m_order);
  on_order(order_model->get());
  m_order_connection = order_model->connect_update_signal(
    std::bind_front(&TableHeaderCell::on_order, this));
}

const std::shared_ptr<CompositeValueModel<TableHeaderCell::Model>>&
    TableHeaderCell::get_model() const {
  return m_model;
}

connection TableHeaderCell::connect_hide_signal(
    const HideSignal::slot_type& slot) const {
  return m_hide_signal.connect(slot);
}

connection TableHeaderCell::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

void TableHeaderCell::on_order(Order order) {
  auto& stylist = find_stylist(*this);
  if(order != Order::UNORDERED) {
    if(!stylist.is_match(Sortable())) {
      stylist.match(Sortable());
    }
  } else if(stylist.is_match(Sortable())) {
    stylist.unmatch(Sortable());
  }
}
