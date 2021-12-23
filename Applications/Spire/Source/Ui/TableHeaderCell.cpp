#include "Spire/Ui/TableHeaderCell.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
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
        update_style(*icon, [] (auto& style) {
          style.get(Any()).set(BackgroundColor(Qt::transparent));
        });
        layout()->addWidget(icon);
      }
    }
  };

  auto make_filter_button() {
    static auto icon = imageFromSvg(":/Icons/filter.svg", scale(6, 6));
    auto button = make_icon_button(icon);
    button->setFixedSize(scale(16, 16));
    match(*button, TableHeaderCell::FilterButton());
    update_style(*button, [] (auto& style) {
      style.get(Body()).set(BackgroundColor(Qt::transparent));
      style.get(Body() / Body()).set(Fill(QColor(0xC8C8C8)));
      style.get(Press() || (Body() && Hover())).
        set(BackgroundColor(QColor(0xF2F2F2)));
    });
    return button;
  }

  auto make_sash() {
    auto sash = new QWidget();
    sash->setFixedWidth(scale_width(5));
    sash->setCursor(Qt::SizeHorCursor);
    auto resize_handle = new Box(nullptr);
    resize_handle->setFixedWidth(scale_width(1));
    update_style(*resize_handle, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
    });
    auto layout = new QHBoxLayout(sash);
    layout->setContentsMargins({});
    layout->addSpacerItem(
      new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->addWidget(resize_handle);
    return sash;
  }
}

TableHeaderCell::TableHeaderCell(
    std::shared_ptr<CompositeValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto name_label = make_label(m_model->get(&Model::m_name));
  match(*name_label, Label());
  auto sort_indicator = new SortIndicator(m_model->get(&Model::m_order));
  m_filter_button = make_filter_button();
  auto inner_layout = new QHBoxLayout();
  inner_layout->setContentsMargins({});
  inner_layout->addWidget(name_label);
  inner_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  inner_layout->addWidget(sort_indicator);
  inner_layout->addWidget(m_filter_button);
  inner_layout->addWidget(make_sash());
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
  style.get(Filtered() > FilterButton() / Body() / Body()).
    set(Fill(QColor(0x4B23A0)));
  set_style(*this, std::move(style));
  auto has_filter_model = m_model->get(&Model::m_has_filter);
  on_has_filter(has_filter_model->get());
  m_has_filter_connection = has_filter_model->connect_update_signal(
    std::bind_front(&TableHeaderCell::on_has_filter, this));
  auto order_model = m_model->get(&Model::m_order);
  on_order(order_model->get());
  m_order_connection = order_model->connect_update_signal(
    std::bind_front(&TableHeaderCell::on_order, this));
}

const std::shared_ptr<CompositeValueModel<TableHeaderCell::Model>>&
    TableHeaderCell::get_model() const {
  return m_model;
}

connection TableHeaderCell::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeaderCell::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_button->connect_clicked_signal(slot);
}

void TableHeaderCell::mouseReleaseEvent(QMouseEvent* event) {
  auto order = m_model->get().m_order;
  if(order == Order::NONE || order == Order::DESCENDING) {
    m_sort_signal(Order::ASCENDING);
  } else if(order == Order::ASCENDING) {
    m_sort_signal(Order::DESCENDING);
  }
}

void TableHeaderCell::on_has_filter(bool has_filter) {
  m_filter_button->setVisible(has_filter);
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
