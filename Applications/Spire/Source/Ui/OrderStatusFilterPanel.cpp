#include "Spire/Ui/OrderStatusFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  constexpr auto BODY_PADDING = 18;

  constexpr auto LIVE_STATUSES = std::array{
    OrderStatus::PENDING_NEW, OrderStatus::NEW, OrderStatus::PARTIALLY_FILLED,
    OrderStatus::SUSPENDED, OrderStatus::STOPPED, OrderStatus::PENDING_CANCEL,
    OrderStatus::CANCEL_REJECT};

  constexpr auto TERMINAL_STATUSES = std::array{
    OrderStatus::REJECTED, OrderStatus::EXPIRED, OrderStatus::CANCELED,
    OrderStatus::FILLED, OrderStatus::DONE_FOR_DAY};
}

struct OrderStatusFilterPanel::PresetButtonContainer : QWidget {
  std::shared_ptr<AssociativeValueModel<Preset>> m_preset;
  std::unordered_map<Preset, Button*> m_buttons;

  explicit PresetButtonContainer(QWidget* parent = nullptr)
      : QWidget(parent),
        m_preset(std::make_shared<AssociativeValueModel<Preset>>()) {
    auto layout = make_hbox_layout(this);
    layout->setSpacing(scale_width(4));
    add_button(layout, Preset::ALL);
    add_button(layout, Preset::LIVE);
    add_button(layout, Preset::TERMINAL);
    add_button(layout, Preset::CUSTOM);
    on_preset(m_preset->get(), true);
  }

  static const QString& display_text(Preset preset) {
    if(preset == Preset::ALL) {
      static const auto value = QObject::tr("All Orders");
      return value;
    } else if(preset == Preset::LIVE) {
      static const auto value = QObject::tr("Live");
      return value;
    } else if(preset == Preset::TERMINAL) {
      static const auto value = QObject::tr("Terminal");
      return value;
    } else {
      static const auto value = QObject::tr("Custom");
      return value;
    }
  }

  Button* make_preset_button(Preset preset) {
    auto button = make_label_button(display_text(preset));
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    update_style(*button, [] (auto& style) {
      style.get((Checked() && !Hover() && !Press()) > Body()).
        set(BackgroundColor(QColor(0x7F5EEC))).
        set(TextColor(QColor(0xFFFFFF)));
      style.get((Checked() && Disabled()) > Body()).
        set(BackgroundColor(QColor(0xC8C8C8))).
        set(TextColor(QColor(0xFFFFFF)));
    });
    button->connect_click_signal([=] {
      m_preset->set(preset);
    });
    return button;
  }

  void add_button(QHBoxLayout* layout, Preset preset) {
    auto button = make_preset_button(preset);
    m_buttons.emplace(preset, button);
    layout->addWidget(button, 0, Qt::AlignTop);
    m_preset->get_association(preset)->connect_update_signal(
      std::bind_front(&PresetButtonContainer::on_preset, this, preset));
  }

  void on_preset(Preset preset, bool is_selected) {
    if(is_selected) {
      match(*m_buttons[preset], Checked());
    } else {
      unmatch(*m_buttons[preset], Checked());
    }
  }
};

OrderStatusFilterPanel::OrderStatusFilterPanel(QWidget* parent)
  : OrderStatusFilterPanel(
      std::make_shared<ArrayListModel<OrderStatus>>(), parent) {}

OrderStatusFilterPanel::OrderStatusFilterPanel(
    std::shared_ptr<OrderStatusListModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  m_list_box = make_order_status_list_box(m_current);
  m_list_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_list_box->set_placeholder(QObject::tr("Enter order status"));
  m_list_box->setMinimumSize(scale(160, 26));
  m_list_box->setMaximumHeight(scale_height(80));
  m_button_container = new PresetButtonContainer();
  m_button_container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  m_body = new QWidget();
  m_body->installEventFilter(this);
  auto body_layout = make_hbox_layout(m_body);
  body_layout->setSpacing(scale_width(BODY_PADDING));
  body_layout->addWidget(m_button_container);
  body_layout->addWidget(m_list_box);
  auto panel = new FilterPanel(*m_body);
  enclose(*this, *panel);
  proxy_style(*this, *panel);
  panel->connect_reset_signal(
    std::bind_front(&OrderStatusFilterPanel::on_reset, this));
  if(m_current->get_size() == 0) {
    m_button_container->m_preset->set(Preset::ALL);
  } else {
    m_button_container->m_preset->set(Preset::CUSTOM);
  }
  m_list_box->setEnabled(m_button_container->m_preset->get() == Preset::CUSTOM);
  m_button_container->m_preset->connect_update_signal(
    std::bind_front(&OrderStatusFilterPanel::on_preset, this));
  m_connection = m_current->connect_operation_signal(
    std::bind_front(&OrderStatusFilterPanel::on_operation, this));
}

const std::shared_ptr<OrderStatusListModel>&
    OrderStatusFilterPanel::get_current() const {
  return m_current;
}

connection OrderStatusFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool OrderStatusFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body && event->type() == QEvent::Resize) {
    auto available_width =
      m_body->width() - m_button_container->width() - scale_width(BODY_PADDING);
    m_list_box->setMaximumWidth(std::max(0,
      std::max(m_list_box->minimumWidth(), available_width)));
  }
  return QWidget::eventFilter(watched, event);
}

bool OrderStatusFilterPanel::focusNextPrevChild(bool next) {
  if(next) {
    auto focused = focusWidget();
    if(focused && m_list_box->isAncestorOf(focused)) {
      auto candidate = focused->nextInFocusChain();
      while(candidate != focused) {
        if(candidate->isVisible() && candidate->isEnabled() &&
            (candidate->focusPolicy() & Qt::TabFocus) &&
            !m_body->isAncestorOf(candidate) && isAncestorOf(candidate)) {
          candidate->setFocus(Qt::TabFocusReason);
          return true;
        }
        candidate = candidate->nextInFocusChain();
      }
    }
  }
  return QWidget::focusNextPrevChild(next);
}

void OrderStatusFilterPanel::submit() {
  m_submit_signal(m_current);
}

void OrderStatusFilterPanel::on_preset(Preset preset) {
  if(preset != Preset::CUSTOM) {
    clear(*m_current);
    if(preset == Preset::LIVE) {
      for(auto status : LIVE_STATUSES) {
        m_current->push(status);
      }
    } else if(preset == Preset::TERMINAL) {
      for(auto status : TERMINAL_STATUSES) {
        m_current->push(status);
      }
    }
    submit();
  }
  m_list_box->setEnabled(preset == Preset::CUSTOM);
}

void OrderStatusFilterPanel::on_reset() {
  m_button_container->m_preset->set(Preset::ALL);
}

void OrderStatusFilterPanel::on_operation(
    const OrderStatusListModel::Operation& operation) {
  if(m_button_container->m_preset->get() != Preset::CUSTOM) {
    return;
  }
  visit(operation,
    [&] (const OrderStatusListModel::AddOperation&) {
      submit();
    },
    [&] (const OrderStatusListModel::RemoveOperation&) {
      submit();
    },
    [&] (const OrderStatusListModel::UpdateOperation&) {
      submit();
    });
}
