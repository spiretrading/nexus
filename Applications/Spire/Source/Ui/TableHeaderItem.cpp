#include "Spire/Ui/TableHeaderItem.hpp"
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto HEADER_NAME_COUNT = 2;

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
    button->setFocusPolicy(Qt::NoFocus);
    update_style(*button, [] (auto& style) {
      style.get(Any() > Body()).set(BackgroundColor(Qt::transparent));
      style.get(Any() > is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
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

  class HeaderNameListModel : public ListModel<QString> {
    public:
      explicit HeaderNameListModel(
        std::shared_ptr<ValueModel<TableHeaderItem::Model>> source)
        : m_source(std::move(source)),
          m_names{m_source->get().m_name, m_source->get().m_short_name},
          m_connection(m_source->connect_update_signal(
            std::bind_front(&HeaderNameListModel::on_current, this))) {}

      int get_size() const override {
        return HEADER_NAME_COUNT;
      }

      const QString& get(int index) const override {
        return m_names[index];
      }

      connection connect_operation_signal(
          const OperationSignal::slot_type& slot) const override {
        return m_transaction.connect_operation_signal(slot);
      }

    protected:
      void transact(const std::function<void()>& transaction) override {
        m_transaction.transact(transaction);
      }

    private:
      std::shared_ptr<ValueModel<TableHeaderItem::Model>> m_source;
      std::array<QString, HEADER_NAME_COUNT> m_names;
      scoped_connection m_connection;
      ListModelTransactionLog<QString> m_transaction;

      void on_current(const TableHeaderItem::Model& current) {
        auto current_names = std::array<QString, HEADER_NAME_COUNT>{
          current.m_name, current.m_short_name};
        m_transaction.transact([&] {
          for(auto i = 0; i < HEADER_NAME_COUNT; ++i) {
            if(current_names[i] != m_names[i]) {
              auto previous = m_names[i];
              m_names[i] = current_names[i];
              m_transaction.push(UpdateOperation(i, previous, m_names[i]));
            }
          }
        });
      }
  };
}

TableHeaderItem::TableHeaderItem(
    std::shared_ptr<ValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_is_resizeable(true) {
  auto name_label =
    new ResponsiveLabel(std::make_shared<HeaderNameListModel>(m_model));
  match(*name_label, Label());
  link(*this, *name_label);
  m_sort_indicator =
    new SortIndicator(make_field_value_model(m_model, &Model::m_order));
  link(*this, *m_sort_indicator);
  m_filter_button = make_filter_button();
  match(*m_filter_button, FilterButton());
  link(*this, *m_filter_button);
  m_sash = make_sash();
  m_sash->installEventFilter(this);
  auto hover_element = new Box(nullptr);
  hover_element->setFixedSize(scale(18, 2));
  match(*hover_element, HoverElement());
  auto contents = new QWidget();
  contents->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_contents_layout = make_hbox_layout(contents);
  m_contents_layout->setContentsMargins({scale_width(8), 0, 0, 0});
  m_contents_layout->addWidget(name_label);
  m_contents_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  m_contents_layout->addWidget(m_sort_indicator);
  m_contents_layout->addWidget(m_filter_button);
  auto top_layout = make_hbox_layout();
  top_layout->addWidget(contents);
  top_layout->addWidget(m_sash);
  m_bottom_layout = make_hbox_layout();
  m_bottom_layout->setContentsMargins({scale_width(8), 0, 0, 0});
  m_bottom_layout->addWidget(hover_element);
  m_bottom_layout->addSpacerItem(
    new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto layout = make_vbox_layout(this);
  layout->setContentsMargins({0, scale_height(8), 0, 0});
  layout->addLayout(top_layout);
  layout->addLayout(m_bottom_layout);
  auto style = StyleSheet();
  style.get(Any() > Label()).set(TextColor(QColor(0x808080)));
  style.get((Hover() && Sortable()) > Label()).set(TextColor(QColor(0x4B23A0)));
  style.get(Any() > HoverElement()).set(Visibility::INVISIBLE);
  style.get((Hover() && Sortable()) > HoverElement()).
    set(BackgroundColor(0x4B23A0)).
    set(Visibility::VISIBLE);
  style.get(Filtered() > FilterButton() > Body() > Body()).
    set(Fill(QColor(0x4B23A0)));
  set_style(*this, std::move(style));
  m_connection = m_model->connect_update_signal(
    std::bind_front(&TableHeaderItem::on_update, this));
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&TableHeaderItem::on_style, this));
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
  m_sort_indicator->setVisible(model.m_order != Order::UNORDERED);
  auto& spacer = *m_contents_layout->itemAt(1)->spacerItem();
  if(model.m_filter == TableFilter::Filter::NONE &&
      model.m_order == Order::UNORDERED) {
    if(spacer.sizeHint().width() != 0) {
      spacer.changeSize(0, 0);
      m_contents_layout->invalidate();
    }
  } else if(spacer.sizeHint().width() != 1) {
    spacer.changeSize(1, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contents_layout->invalidate();
  }
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

void TableHeaderItem::on_style() {
  auto paddings = std::make_shared<QMargins>();
  auto left_padding = std::make_shared<int>(-1);
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          paddings->setTop(size);
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          paddings->setRight(size);
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          paddings->setBottom(size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          *left_padding = size;
        });
      });
  }
  if(!paddings->isNull()) {
    layout()->setContentsMargins(*paddings);
  }
  if(*left_padding >= 0) {
    m_contents_layout->setContentsMargins({*left_padding, 0, 0, 0});
    m_bottom_layout->setContentsMargins({*left_padding, 0, 0, 0});
  }
}
