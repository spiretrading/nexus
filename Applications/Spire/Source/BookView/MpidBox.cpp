#include "Spire/BookView/MpidBox.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto BOOK_QUOTE_TYPE_INDEX = 0;
  const auto USER_ORDER_TYPE_INDEX = 1;
  const auto PREVIEW_TYPE_INDEX = 2;

  QString make_id(const BookEntry& entry) {
    if(auto quote = get<BookQuote>(&entry)) {
      return QString::fromStdString(quote->m_mpid);
    } else if(auto order = get<BookViewModel::UserOrder>(&entry)) {
      return QString::fromStdString('@' + order->m_destination);
    } else if(auto preview = get<OrderFields>(&entry)) {
      return QString::fromStdString('@' + preview->m_destination);
    }
    return {};
  }
}

MpidBox::MpidBox(std::shared_ptr<BookEntryModel> current,
    std::shared_ptr<ValueModel<int>> level,
    std::shared_ptr<ValueModel<bool>> is_top_mpid)
    : m_current(std::move(current)),
      m_current_status(OrderStatus::NONE),
      m_current_transition(0),
      m_is_reset(true),
      m_is_settled(false),
      m_level(std::move(level)),
      m_current_level(m_level->get()),
      m_is_top_mpid(std::move(is_top_mpid)) {
  auto label = make_label(make_read_only_to_text_model(m_current, &make_id));
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

void MpidBox::update_row_state(int type_index) {
  if(type_index == m_current_type_index) {
    return;
  }
  if(m_current_type_index) {
    if(*m_current_type_index == BOOK_QUOTE_TYPE_INDEX) {
      unmatch(*this, PriceLevelRow(m_current_level));
    } else if(*m_current_type_index == USER_ORDER_TYPE_INDEX) {
      unmatch(*this, UserOrderRow(OrderStatus::NONE));
      if(m_current_status != OrderStatus::NONE) {
        unmatch(*this, UserOrderRow(m_current_status));
        m_current_status = OrderStatus::NONE;
      }
      if(m_is_settled) {
        unmatch(*this, SettledRow());
        m_is_settled = false;
      }
      m_current_transition = 0;
    } else {
      unmatch(*this, PreviewRow());
    }
  }
  m_current_type_index = type_index;
  if(type_index == BOOK_QUOTE_TYPE_INDEX) {
    match(*this, PriceLevelRow(m_current_level));
  } else if(type_index == USER_ORDER_TYPE_INDEX) {
    match(*this, UserOrderRow(OrderStatus::NONE));
  } else {
    match(*this, PreviewRow());
  }
}

void MpidBox::update_venue_state(const BookEntry& entry) {
  if(auto quote = get<BookQuote>(&entry)) {
    if(quote->m_venue != m_current_venue) {
      if(m_current_venue) {
        unmatch(*this, VenueRow(m_current_venue));
      }
      m_current_venue = quote->m_venue;
      match(*this, VenueRow(quote->m_venue));
    }
  } else if(m_current_venue) {
    unmatch(*this, VenueRow(m_current_venue));
    m_current_venue = Venue();
  }
}

void MpidBox::update_status(const BookEntry& entry) {
  auto order = get<BookViewModel::UserOrder>(&entry);
  auto transition = [&] () -> std::uint64_t {
    if(!order) {
      return 0;
    }
    return order->m_transition;
  }();
  auto status = [&] () -> OrderStatus {
    if(!order || transition == 0) {
      return OrderStatus::NONE;
    } else if(order->m_highlight == OrderStatus::CANCELED ||
        order->m_highlight == OrderStatus::FILLED ||
        order->m_highlight == OrderStatus::REJECTED) {
      return order->m_highlight;
    } else if(order->m_highlight == OrderStatus::PARTIALLY_FILLED) {
      return OrderStatus::FILLED;
    } else if(order->m_highlight == OrderStatus::EXPIRED ||
        order->m_highlight == OrderStatus::DONE_FOR_DAY) {
      return OrderStatus::CANCELED;
    }
    return OrderStatus::NONE;
  }();
  auto is_settled = status != OrderStatus::NONE && m_is_reset;
  m_is_reset = false;
  if(status == m_current_status && transition == m_current_transition &&
      is_settled == m_is_settled) {
    return;
  }
  if(m_current_status != OrderStatus::NONE) {
    unmatch(*this, UserOrderRow(m_current_status));
  }
  if(m_is_settled) {
    unmatch(*this, SettledRow());
  }
  m_current_status = status;
  m_current_transition = transition;
  m_is_settled = is_settled;
  if(m_is_settled) {
    match(*this, SettledRow());
  }
  if(m_current_status != OrderStatus::NONE) {
    match(*this, UserOrderRow(m_current_status));
  }
}

void MpidBox::on_current(const BookEntry& entry) {
  update_row_state(entry.which());
  update_venue_state(entry);
  update_status(entry);
}

void MpidBox::on_level(int level) {
  if(level == m_current_level) {
    return;
  }
  if(m_current_type_index == BOOK_QUOTE_TYPE_INDEX) {
    unmatch(*this, PriceLevelRow(m_current_level));
    match(*this, PriceLevelRow(level));
  }
  m_current_level = level;
}

void MpidBox::on_is_top_mpid(bool is_top) {
  if(is_top) {
    match(*this, TopVenueRow());
  } else {
    unmatch(*this, TopVenueRow());
  }
}
