#include "Spire/Ui/TableHeader.hpp"
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FixedHorizontalLayout.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void reset_filter_panel_height(QWidget& panel) {
    panel.setMinimumHeight(0);
    panel.setMaximumHeight(QWIDGETSIZE_MAX);
  }

  void dismiss_filter_panel(QBoxLayout& layout, QWidget& panel) {
    layout.removeWidget(&panel);
    reset_filter_panel_height(panel);
    panel.hide();
  }

  QVariantAnimation* make_height_animation(
      QWidget& panel, int start, int end) {
    auto animation = new QVariantAnimation(&panel);
    animation->setDuration(100);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(start);
    animation->setEndValue(end);
    QObject::connect(animation, &QVariantAnimation::valueChanged,
      [&panel] (const QVariant& value) {
        panel.setFixedHeight(value.toInt());
      });
    return animation;
  }
}

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> items,
    std::shared_ptr<TableFilter> filter, QWidget* parent)
    : QWidget(parent),
      m_items(std::move(items)),
      m_filter(std::move(filter)),
      m_filter_panel(nullptr),
      m_closing_filter_panel(nullptr),
      m_is_closing_filters(false) {
  m_widths = std::make_shared<ArrayListModel<int>>();
  auto body = new QWidget();
  auto layout = new FixedHorizontalLayout(body);
  for(auto i = 0; i != m_items->get_size(); ++i) {
    auto item = new TableHeaderItem(make_list_value_model(m_items, i));
    item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    if(i == 0) {
      update_style(*item, [] (auto& style) {
        style.get(Any() > TableHeaderItem::Container()).
          set(PaddingLeft(scale_width(8)));
      });
    }
    m_widths->push(item->sizeHint().width());
    item->setFixedWidth(m_widths->get(i));
    item->connect_sort_signal(
      std::bind_front(&TableHeader::on_sort, this, i));
    item->is_filter_open()->connect_update_signal(
      std::bind_front(&TableHeader::on_filter_open, this, i));
    layout->addWidget(item);
    m_item_views.push_back(item);
    link(*this, *item);
    item->installEventFilter(this);
  }
  auto spacer_model = TableHeaderItem::Model{"", "",
    TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE};
  auto spacer = new TableHeaderItem(
    std::make_shared<LocalValueModel<TableHeaderItem::Model>>(spacer_model));
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  spacer->setFocusPolicy(Qt::NoFocus);
  spacer->set_is_resizeable(false);
  update_style(*spacer, [] (auto& style) {
    style.get(Any() > TableHeaderItem::Label()).
      set(Visibility::NONE);
    style.get(Any() > TableHeaderItem::ActiveElement()).
      set(Visibility::NONE);
  });
  layout->addWidget(spacer);
  link(*this, *spacer);
  auto inner_box = new Box(body);
  inner_box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  update_style(*inner_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF)));
  });
  m_scroll_box = new ScrollBox(inner_box);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER);
  m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto outer_body = new QWidget();
  m_body_layout = make_vbox_layout(outer_body);
  m_body_layout->addWidget(m_scroll_box);
  auto outer_box = new Box(outer_body);
  update_style(*outer_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_width(1))).
      set(BorderBottomColor(0xE0E0E0));
  });
  enclose(*this, *outer_box);
  proxy_style(*this, *outer_box);
  m_filter_connection = m_filter->connect_filter_signal(
    std::bind_front(&TableHeader::on_filter, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableHeader::on_widths_operation, this));
}

const std::shared_ptr<ListModel<TableHeaderItem::Model>>&
    TableHeader::get_items() const {
  return m_items;
}

const std::shared_ptr<TableFilter>& TableHeader::get_filter() const {
  return m_filter;
}

const std::shared_ptr<ListModel<int>>& TableHeader::get_widths() const {
  return m_widths;
}

TableHeaderItem* TableHeader::get_item(int column) {
  if(column < 0 || column >= std::ssize(m_item_views)) {
    return nullptr;
  }
  return m_item_views[column];
}

ScrollBox& TableHeader::get_scroll_box() {
  return *m_scroll_box;
}

optional<int> TableHeader::get_index(TableHeaderItem* item) const {
  auto i = std::find(m_item_views.begin(), m_item_views.end(), item);
  if(i != m_item_views.end()) {
    return std::distance(m_item_views.begin(), i);
  }
  return {};
}

connection TableHeader::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

bool TableHeader::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    if(auto index = get_index(static_cast<TableHeaderItem*>(watched));
        index && *index < m_widths->get_size()) {
      auto width = static_cast<TableHeaderItem*>(watched)->width();
      if(width != m_widths->get(*index)) {
        m_widths->set(*index, width);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TableHeader::close_filter_panel() {
  auto panel = m_filter_panel;
  m_filter_panel = nullptr;
  m_closing_filter_panel = panel;
  auto& body = panel->get_body();
  if(body.isWindow()) {
    body.hide();
  }
  auto animation = make_height_animation(*panel, panel->height(), 0);
  connect(animation, &QVariantAnimation::finished, this, [=] {
    dismiss_filter_panel(*m_body_layout, *panel);
    m_closing_filter_panel = nullptr;
  });
  animation->start(QAbstractAnimation::DeleteWhenStopped);
  m_filter_animation = animation;
}

void TableHeader::open_filter_panel(int index) {
  auto filter_widget = m_filter->make_filter_widget(index, *this);
  if(filter_widget) {
    auto panel = make_popup_panel(*filter_widget, this);
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto target_height = panel->sizeHint().height();
    panel->set_overflow_directions({});
    panel->setFixedHeight(0);
    m_body_layout->addWidget(panel);
    panel->show();
    auto animation = make_height_animation(*panel, 0, target_height);
    connect(animation, &QVariantAnimation::finished, this, [=] {
      reset_filter_panel_height(*panel);
      panel->set_overflow_directions(Qt::Horizontal);
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    m_filter_animation = animation;
    m_filter_panel = panel;
  } else {
    m_filter_panel = nullptr;
  }
}

void TableHeader::swap_filter_panel(int index) {
  auto old_panel = m_filter_panel;
  m_filter_panel = nullptr;
  m_body_layout->removeWidget(old_panel);
  reset_filter_panel_height(*old_panel);
  auto filter_widget = m_filter->make_filter_widget(index, *this);
  if(filter_widget) {
    auto panel = make_popup_panel(*filter_widget, this);
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panel->set_overflow_directions(Qt::Horizontal);
    m_body_layout->addWidget(panel);
    panel->show();
    m_filter_panel = panel;
  }
  auto& old_panel_body = old_panel->get_body();
  if(old_panel_body.isWindow()) {
    old_panel_body.hide();
  }
  old_panel->hide();
}

void TableHeader::on_filter(int column, TableFilter::Filter filter) {
  auto& item = m_items->get(column);
  if(item.m_filter != filter) {
    auto revised_item = item;
    revised_item.m_filter = filter;
    m_items->set(column, revised_item);
  }
}

void TableHeader::on_widths_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      m_item_views[operation.m_index]->setFixedWidth(
        operation.get_value());
    });
}

void TableHeader::on_sort(int index, TableHeaderItem::Order order) {
  m_sort_signal(index, order);
}

void TableHeader::on_filter_open(int index, bool is_open) {
  if(m_is_closing_filters) {
    return;
  }
  m_is_closing_filters = true;
  for(auto i = 0; i != std::ssize(m_item_views); ++i) {
    if(i != index && m_item_views[i]->is_filter_open()->get()) {
      m_item_views[i]->is_filter_open()->set(false);
    }
  }
  m_is_closing_filters = false;
  if(m_filter_animation) {
    m_filter_animation->stop();
  }
  if(m_closing_filter_panel) {
    dismiss_filter_panel(*m_body_layout, *m_closing_filter_panel);
    m_closing_filter_panel = nullptr;
  }
  if(is_open && m_filter_panel) {
    swap_filter_panel(index);
  } else if(is_open) {
    open_filter_panel(index);
  } else if(m_filter_panel) {
    close_filter_panel();
  }
}
