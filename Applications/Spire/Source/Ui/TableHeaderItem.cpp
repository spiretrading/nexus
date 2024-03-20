#include "Spire/Ui/TableHeaderItem.hpp"
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
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
      auto layout = make_hbox_layout(this);
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
    update_style(*button, [] (auto& style) {
      style.get(Any() > Body()).set(BackgroundColor(Qt::transparent));
      style.get(Any() > Body() > Body()).set(Fill(QColor(0xC8C8C8)));
      style.get(Press() || Any() > (Body() && Hover())).
        set(BackgroundColor(QColor(0xF2F2F2)));
    });
    return button;
  }

  auto make_sash() {
    auto sash = new QWidget();
    sash->setFixedWidth(scale_width(5));
    sash->setCursor(Qt::SplitHCursor);
    auto resize_handle = new Box(nullptr);
    resize_handle->setFixedWidth(scale_width(1));
    update_style(*resize_handle, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
    });
    enclose(*sash, *resize_handle, Qt::AlignRight);
    return sash;
  }
}

TableHeaderItem::TableHeaderItem(
    std::shared_ptr<ValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_is_resizeable(true) {
  auto name_label = make_label(make_field_value_model(m_model, &Model::m_name));
  match(*name_label, Label());
  auto sort_indicator =
    new SortIndicator(make_field_value_model(m_model, &Model::m_order));
  m_filter_button = make_filter_button();
  match(*m_filter_button, FilterButton());
  m_sash = make_sash();
  m_sash->installEventFilter(this);
  auto hover_element = new Box(nullptr);
  hover_element->setFixedSize(scale(18, 2));
  match(*hover_element, HoverElement());
  auto contents = new QWidget();
  contents->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto contents_layout = make_hbox_layout(contents);
  contents_layout->setContentsMargins({scale_width(8), 0, 0, 0});
  contents_layout->addWidget(name_label);
  contents_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  contents_layout->addWidget(sort_indicator);
  contents_layout->addWidget(m_filter_button);
  auto top_layout = make_hbox_layout();
  top_layout->addWidget(contents);
  top_layout->addWidget(m_sash);
  auto bottom_layout = make_hbox_layout();
  bottom_layout->setContentsMargins({scale_width(8), 0, 0, 0});
  bottom_layout->addWidget(hover_element);
  bottom_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto layout = make_vbox_layout(this);
  layout->setContentsMargins({0, scale_height(8), 0, 0});
  layout->addLayout(top_layout);
  layout->addLayout(bottom_layout);
  auto style = StyleSheet();
  style.get(Any() > Label()).set(TextColor(QColor(0x808080)));
  style.get(Hover() > Label()).set(TextColor(QColor(0x4B23A0)));
  style.get(Any() > HoverElement()).set(Visibility::INVISIBLE);
  style.get((Hover() && Sortable()) > HoverElement()).
    set(BackgroundColor(0x4B23A0)).
    set(Visibility::VISIBLE);
  style.get(Filtered() > FilterButton() > Body() > Body()).
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

Button& TableHeaderItem::get_filter_button() {
  return *m_filter_button;
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
    layout()->setContentsMargins({0, scale_height(8), 0, 0});
  } else {
    layout()->setContentsMargins({0, scale_height(8), scale_width(8), 0});
  }
}

connection TableHeaderItem::connect_start_resize_signal(
    const StartResizeSignal::slot_type& slot) const {
  return m_start_resize_signal.connect(slot);
}

connection TableHeaderItem::connect_end_resize_signal(
    const EndResizeSignal::slot_type& slot) const {
  return m_end_resize_signal.connect(slot);
}

connection TableHeaderItem::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeaderItem::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_button->connect_click_signal(slot);
}

QSize TableHeaderItem::minimumSizeHint() const {
  auto size_hint = QWidget::minimumSizeHint();
  if(!m_is_resizeable) {
    return {0, size_hint.height()};
  }
  return size_hint;
}

bool TableHeaderItem::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_sash) {
    if(event->type() == QEvent::MouseButtonPress) {
      event->accept();
      m_start_resize_signal();
      return true;
    } else if(event->type() == QEvent::MouseButtonRelease) {
      event->accept();
      m_end_resize_signal();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
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
  m_filter_button->setVisible(model.m_filter != TableFilter::Filter::NONE);
  auto& stylist = find_stylist(*this);
  if(model.m_order != Order::UNORDERED) {
    if(!stylist.is_match(Sortable())) {
      stylist.match(Sortable());
    }
  } else if(stylist.is_match(Sortable())) {
    stylist.unmatch(Sortable());
  }
  if(model.m_filter != TableFilter::Filter::UNFILTERED) {
    if(!stylist.is_match(Filtered())) {
      stylist.match(Filtered());
    }
  } else if(stylist.is_match(Filtered())) {
    stylist.unmatch(Filtered());
  }
}
