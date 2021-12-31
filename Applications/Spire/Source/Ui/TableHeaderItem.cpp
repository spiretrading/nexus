#include "Spire/Ui/TableHeaderItem.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
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

    std::shared_ptr<ValueModel<TableHeaderItem::Order>> m_order;
    scoped_connection m_order_connection;

    explicit SortIndicator(
        std::shared_ptr<ValueModel<TableHeaderItem::Order>> order)
        : m_order(std::move(order)) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      on_order(m_order->get());
      m_order_connection = m_order->connect_update_signal(
        std::bind_front(&SortIndicator::on_order, this));
    }

    void on_order(TableHeaderItem::Order order) {
      if(auto previous_icon = layout()->takeAt(0)) {
        delete previous_icon->widget();
        delete previous_icon;
      }
      auto icon = [&] () -> QWidget* {
        if(order == TableHeaderItem::Order::ASCENDING) {
          return new Icon(ASCENDING_IMAGE());
        } else if(order == TableHeaderItem::Order::DESCENDING) {
          return new Icon(DESCENDING_IMAGE());
        } else if(order == TableHeaderItem::Order::NONE) {
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
    match(*button, TableHeaderItem::FilterButton());
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

TableHeaderItem::TableHeaderItem(
    std::shared_ptr<ValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto name_label = make_label(make_field_value_model(m_model, &Model::m_name));
  match(*name_label, Label());
  auto sort_indicator =
    new SortIndicator(make_field_value_model(m_model, &Model::m_order));
  m_filter_button = make_filter_button();
  m_sash = make_sash();
  auto inner_layout = new QHBoxLayout();
  inner_layout->setContentsMargins({});
  inner_layout->addWidget(name_label);
  inner_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  inner_layout->addWidget(sort_indicator);
  inner_layout->addWidget(m_filter_button);
  inner_layout->addWidget(m_sash);
  auto hover_element = new Box(nullptr);
  hover_element->setFixedSize(scale(18, 2));
  match(*hover_element, HoverElement());
  auto hover_layout = new QHBoxLayout();
  hover_layout->setContentsMargins({});
  hover_layout->addWidget(hover_element);
  hover_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto outer_layout = new QVBoxLayout(this);
  outer_layout->setContentsMargins({scale_width(8), scale_height(8), 0, 0});
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
  m_connection = m_model->connect_update_signal(
    std::bind_front(&TableHeaderItem::on_update, this));
  on_update(m_model->get());
}

const std::shared_ptr<ValueModel<TableHeaderItem::Model>>&
    TableHeaderItem::get_model() const {
  return m_model;
}

bool TableHeaderItem::is_resizeable() const {
  return m_is_resizeable;
}

void TableHeaderItem::set_is_resizeable(bool is_resizeable) {
  if(m_is_resizeable == is_resizeable) {
    return;
  }
  m_is_resizeable = is_resizeable;
  m_sash->setVisible(m_is_resizeable);
  if(m_is_resizeable) {
    layout()->setContentsMargins({scale_width(8), scale_height(8), 0, 0});
  } else {
    layout()->setContentsMargins(
      {scale_width(8), scale_height(8), scale_width(8), 0});
  }
}

connection TableHeaderItem::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeaderItem::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_button->connect_clicked_signal(slot);
}

void TableHeaderItem::mouseReleaseEvent(QMouseEvent* event) {
  auto order = m_model->get().m_order;
  if(order == Order::NONE || order == Order::DESCENDING) {
    m_sort_signal(Order::ASCENDING);
  } else if(order == Order::ASCENDING) {
    m_sort_signal(Order::DESCENDING);
  }
}

void TableHeaderItem::on_update(const Model& model) {
  m_filter_button->setVisible(model.m_has_filter);
  auto& stylist = find_stylist(*this);
  if(model.m_order != Order::UNORDERED) {
    if(!stylist.is_match(Sortable())) {
      stylist.match(Sortable());
    }
  } else if(stylist.is_match(Sortable())) {
    stylist.unmatch(Sortable());
  }
}
