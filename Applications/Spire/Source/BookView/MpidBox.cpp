#include "Spire/BookView/MpidBox.hpp"
#include <algorithm>
#include <QPointer>
#include "Spire/Spire/DeduplicatedValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto BOOK_QUOTE_TYPE_INDEX = 0;
  const auto USER_ORDER_TYPE_INDEX = 1;
  const auto PREVIEW_TYPE_INDEX = 2;

  OrderStatus to_highlight_status(OrderStatus status) {
    if(status == OrderStatus::CANCELED || status == OrderStatus::FILLED ||
        status == OrderStatus::REJECTED) {
      return status;
    } else if(status == OrderStatus::PARTIALLY_FILLED) {
      return OrderStatus::FILLED;
    } else if(status == OrderStatus::EXPIRED ||
        status == OrderStatus::DONE_FOR_DAY) {
      return OrderStatus::CANCELED;
    }
    return OrderStatus::NONE;
  }

  QString make_id(const BookEntry& entry) {
    if(auto quote = std::get_if<BookQuote>(&entry)) {
      return QString::fromStdString(quote->m_mpid);
    } else if(auto order = std::get_if<BookViewModel::UserOrder>(&entry)) {
      return QString::fromStdString('@' + order->m_destination);
    }
    return QString::fromStdString(
      '@' + std::get<OrderFields>(entry).m_destination);
  }
}

struct MpidBox::RowStates {
  struct State {
    Selector m_selector;
    int m_count;
  };
  QPointer<QWidget> m_row;
  std::vector<State> m_states;

  explicit RowStates(QWidget& row)
    : m_row(&row) {}

  void add(const Selector& selector) {
    auto i = std::ranges::find(m_states, selector, &State::m_selector);
    if(i != m_states.end()) {
      ++i->m_count;
      return;
    }
    m_states.emplace_back(selector, 1);
    if(m_row) {
      match(*m_row, selector);
    }
  }

  void remove(const Selector& selector) {
    auto i = std::ranges::find(m_states, selector, &State::m_selector);
    if(i == m_states.end()) {
      return;
    }
    --i->m_count;
    if(i->m_count == 0) {
      m_states.erase(i);
      if(m_row) {
        unmatch(*m_row, selector);
      }
    }
  }
};

MpidBox::MpidBox(std::shared_ptr<BookEntryModel> current,
    std::shared_ptr<ValueModel<int>> level,
    std::shared_ptr<ValueModel<bool>> is_top_mpid)
    : m_current(std::move(current)),
      m_level(std::move(level)),
      m_is_top_mpid(std::move(is_top_mpid)),
      m_current_status(OrderStatus::NONE),
      m_current_transition(0),
      m_current_level(m_level->get()),
      m_is_reset(true),
      m_is_settled(false) {
  auto label = make_label(make_deduplicated_value_model(
    make_read_only_to_text_model(m_current, &make_id)));
  enclose(*this, *label);
  proxy_style(*this, *label);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(PaddingLeft(scale_width(4))).
      set(PaddingRight(scale_width(2)));
  });
  on_current(m_current->get());
  on_is_top_mpid(m_is_top_mpid->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&MpidBox::on_current, this));
  m_level_connection =
    m_level->connect_update_signal(std::bind_front(&MpidBox::on_level, this));
  m_is_top_mpid_connection = m_is_top_mpid->connect_update_signal(
    std::bind_front(&MpidBox::on_is_top_mpid, this));
}

MpidBox::~MpidBox() {
  if(m_row) {
    for(auto& state : m_states) {
      m_row->remove(state);
    }
  }
}

const std::shared_ptr<BookEntryModel>& MpidBox::get_current() const {
  return m_current;
}

const std::shared_ptr<ValueModel<int>>& MpidBox::get_level() const {
  return m_level;
}

const std::shared_ptr<ValueModel<bool>>& MpidBox::is_top_mpid() const {
  return m_is_top_mpid;
}

void MpidBox::reset() {
  m_is_reset = true;
}

bool MpidBox::event(QEvent* event) {
  if(event->type() == QEvent::ParentChange) {
    update_row();
  }
  return QWidget::event(event);
}

void MpidBox::match_state(const Selector& selector) {
  if(std::ranges::find(m_states, selector) != m_states.end()) {
    return;
  }
  m_states.push_back(selector);
  match(*this, selector);
  if(m_row) {
    m_row->add(selector);
  }
}

void MpidBox::unmatch_state(const Selector& selector) {
  auto i = std::ranges::find(m_states, selector);
  if(i == m_states.end()) {
    return;
  }
  m_states.erase(i);
  unmatch(*this, selector);
  if(m_row) {
    m_row->remove(selector);
  }
}

void MpidBox::update_row() {
  auto row = [&] () -> QWidget* {
    auto item = parentWidget();
    if(!item) {
      return nullptr;
    }
    auto row = item->parentWidget();
    if(!row || !is_match(*row, Row())) {
      return nullptr;
    }
    return row;
  }();
  auto previous_row = [&] () -> QWidget* {
    if(m_row) {
      return m_row->m_row;
    }
    return nullptr;
  }();
  if(row == previous_row) {
    return;
  }
  if(m_row) {
    for(auto& state : m_states) {
      m_row->remove(state);
    }
    m_row = nullptr;
  }
  if(row) {
    m_row = find_extension<RowStates>(*row);
    for(auto& state : m_states) {
      m_row->add(state);
    }
  }
}

void MpidBox::update_row_state(int type_index) {
  if(type_index == m_current_type_index) {
    return;
  }
  if(m_current_type_index) {
    if(*m_current_type_index == BOOK_QUOTE_TYPE_INDEX) {
      unmatch_state(PriceLevelRow(m_current_level));
    } else if(*m_current_type_index == USER_ORDER_TYPE_INDEX) {
      unmatch_state(UserOrderRow(OrderStatus::NONE));
      if(m_current_status != OrderStatus::NONE) {
        unmatch_state(UserOrderRow(m_current_status));
        m_current_status = OrderStatus::NONE;
      }
      if(m_is_settled) {
        unmatch_state(SettledRow());
        m_is_settled = false;
      }
      m_current_transition = 0;
    } else if(*m_current_type_index == PREVIEW_TYPE_INDEX) {
      unmatch_state(PreviewRow());
    }
  }
  m_current_type_index = type_index;
  if(type_index == BOOK_QUOTE_TYPE_INDEX) {
    match_state(PriceLevelRow(m_current_level));
  } else if(type_index == USER_ORDER_TYPE_INDEX) {
    match_state(UserOrderRow(OrderStatus::NONE));
  } else if(type_index == PREVIEW_TYPE_INDEX) {
    match_state(PreviewRow());
  }
}

void MpidBox::update_venue_state(const BookEntry& entry) {
  if(auto quote = std::get_if<BookQuote>(&entry)) {
    if(quote->m_venue != m_current_venue) {
      if(m_current_venue) {
        unmatch_state(VenueRow(m_current_venue));
      }
      m_current_venue = quote->m_venue;
      if(m_current_venue) {
        match_state(VenueRow(m_current_venue));
      }
    }
  } else if(m_current_venue) {
    unmatch_state(VenueRow(m_current_venue));
    m_current_venue = Venue();
  }
}

void MpidBox::update_status(const BookEntry& entry) {
  auto order = std::get_if<BookViewModel::UserOrder>(&entry);
  auto transition = [&] () -> std::uint64_t {
    if(!order) {
      return 0;
    }
    return order->m_transition;
  }();
  auto status = [&] () -> OrderStatus {
    if(!order || transition == 0) {
      return OrderStatus::NONE;
    }
    return to_highlight_status(order->m_highlight);
  }();
  auto is_settled = [&] {
    if(m_is_reset) {
      return status != OrderStatus::NONE;
    }
    return m_is_settled && transition == m_current_transition;
  }();
  m_is_reset = false;
  if(status == m_current_status && transition == m_current_transition &&
      is_settled == m_is_settled) {
    return;
  }
  if(m_current_status != OrderStatus::NONE) {
    unmatch_state(UserOrderRow(m_current_status));
  }
  if(m_is_settled) {
    unmatch_state(SettledRow());
  }
  m_current_status = status;
  m_current_transition = transition;
  m_is_settled = is_settled;
  if(m_is_settled) {
    match_state(SettledRow());
  }
  if(m_current_status != OrderStatus::NONE) {
    match_state(UserOrderRow(m_current_status));
  }
}

void MpidBox::on_current(const BookEntry& entry) {
  update_row_state(static_cast<int>(entry.index()));
  update_venue_state(entry);
  update_status(entry);
}

void MpidBox::on_level(int level) {
  if(level == m_current_level) {
    return;
  }
  if(m_current_type_index == BOOK_QUOTE_TYPE_INDEX) {
    unmatch_state(PriceLevelRow(m_current_level));
    match_state(PriceLevelRow(level));
  }
  m_current_level = level;
}

void MpidBox::on_is_top_mpid(bool is_top) {
  if(is_top) {
    match_state(TopVenueRow());
  } else {
    unmatch_state(TopVenueRow());
  }
}
