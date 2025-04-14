#include "Spire/BookView/MpidBox.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

MpidBox::MpidBox(
    std::shared_ptr<MpidModel> current, std::shared_ptr<ValueModel<int>> level,
    std::shared_ptr<ValueModel<bool>> is_top_mpid)
    : m_current(std::move(current)),
      m_level(std::move(level)),
      m_current_level(-1),
      m_is_top_mpid(std::move(is_top_mpid)) {
  auto label = make_label(make_read_only_to_text_model(
    m_current, std::bind_front(&MpidBox::make_id, this)));
  enclose(*this, *label);
  proxy_style(*this, *label);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(PaddingLeft(scale_width(4))).
      set(PaddingRight(scale_width(2)));
  });
  on_current(m_current->get());
  on_level(m_level->get());
  on_is_top_mpid(m_is_top_mpid->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&MpidBox::on_current, this));
  m_level_connection =
    m_level->connect_update_signal(std::bind_front(&MpidBox::on_level, this));
  m_is_top_mpid_connection = m_is_top_mpid->connect_update_signal(
    std::bind_front(&MpidBox::on_is_top_mpid, this));
}

std::shared_ptr<MpidModel> MpidBox::get_current() const {
  return m_current;
}

std::shared_ptr<ValueModel<int>> MpidBox::get_level() const {
  return m_level;
}

std::shared_ptr<ValueModel<bool>> MpidBox::is_top_mpid() const {
  return m_is_top_mpid;
}

QString MpidBox::make_id(const Mpid& mpid) const {
  if(mpid.m_origin == Mpid::Origin::USER_ORDER ||
      mpid.m_origin == Mpid::Origin::PREVIEW) {
    return QString::fromStdString('@' + mpid.m_id);
  }
  return QString::fromStdString(mpid.m_id);
}

void MpidBox::on_current(const Mpid& mpid) {
  if(mpid.m_origin != m_current_origin) {
    if(m_current_origin) {
      if(*m_current_origin == Mpid::Origin::PREVIEW) {
        unmatch(*this, PreviewRow());
      } else if(*m_current_origin == Mpid::Origin::USER_ORDER) {
        unmatch(*this, UserOrderRow());
      } else if(*m_current_origin == Mpid::Origin::BOOK_QUOTE) {
        unmatch(*this, PriceLevelRow(m_current_level));
      }
    }
    m_current_origin = mpid.m_origin;
    if(mpid.m_origin == Mpid::Origin::PREVIEW) {
      match(*this, PreviewRow());
    } else if(mpid.m_origin == Mpid::Origin::USER_ORDER) {
      match(*this, UserOrderRow());
    } else {
      match(*this, PriceLevelRow(m_level->get()));
    }
  }
  if(mpid.m_market != m_current_market) {
    if(!m_current_market.IsEmpty()) {
      unmatch(*this, MarketRow(m_current_market));
    }
    m_current_market = mpid.m_market;
    if(!mpid.m_market.IsEmpty()) {
      if(m_current_origin == Mpid::Origin::BOOK_QUOTE) {
        match(*this, MarketRow(mpid.m_market));
      }
    }
  }
}

void MpidBox::on_level(int level) {
  if(level == m_current_level) {
    return;
  }
  if(m_current_origin == Mpid::Origin::BOOK_QUOTE) {
    unmatch(*this, PriceLevelRow(m_current_level));
    match(*this, PriceLevelRow(level));
  }
  m_current_level = level;
}

void MpidBox::on_is_top_mpid(bool is_top) {
  if(is_top) {
    match(*this, TopMarketRow());
  } else {
    unmatch(*this, TopMarketRow());
  }
}
