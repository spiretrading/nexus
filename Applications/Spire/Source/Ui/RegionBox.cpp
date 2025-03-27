#include "Spire/Ui/RegionBox.hpp"
#include <boost/iterator/counting_iterator.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/TagComboBox.hpp"

using namespace boost::iterators;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct RegionHash {
    std::size_t operator()(const Region& region) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, boost::hash_range(
        region.GetCountries().begin(), region.GetCountries().end()));
      auto markets = region.GetMarkets();
      boost::hash_combine(seed, boost::hash_range(
        markets.begin(), markets.end()));
      boost::hash_combine(seed, boost::hash_range(
        region.GetSecurities().begin(), region.GetSecurities().end()));
      return seed;
    }
  };

  std::vector<Region> to_tag_list(
      const Region& region, RegionQueryModel& regions) {
    auto tags = std::vector<Region>();
    for(auto& country : region.GetCountries()) {
      tags.push_back(*regions.parse(to_text(country)));
    }
    for(auto& market : region.GetMarkets()) {
      tags.push_back(*regions.parse(to_text(MarketToken(market))));
    }
    for(auto& security : region.GetSecurities()) {
      tags.push_back(*regions.parse(to_text(security)));
    }
    return tags;
  }

  void sort(AnyListModel& list) {
    auto comparator = [&] (const auto& lhs, const auto& rhs) {
      auto lhs_region = std::any_cast<Region&&>(list.get(lhs));
      auto rhs_region = std::any_cast<Region&&>(list.get(rhs));
      if(!lhs_region.GetCountries().empty() &&
          !rhs_region.GetCountries().empty()) {
        return to_text(*lhs_region.GetCountries().begin()) <
          to_text(*rhs_region.GetCountries().begin());
      } else if(!lhs_region.GetMarkets().empty() &&
          !rhs_region.GetMarkets().empty()) {
        return to_text(MarketToken(*lhs_region.GetMarkets().begin())) <
          to_text(MarketToken(*rhs_region.GetMarkets().begin()));
      } else if(!lhs_region.GetSecurities().empty() &&
          !rhs_region.GetSecurities().empty()) {
        return to_text(*lhs_region.GetSecurities().begin()) <
          to_text(*rhs_region.GetSecurities().begin());
      }
      if(!lhs_region.GetCountries().empty() ||
          !rhs_region.GetCountries().empty()) {
        return !lhs_region.GetCountries().empty();
      }
      if(!lhs_region.GetMarkets().empty() || !rhs_region.GetMarkets().empty()) {
        return !lhs_region.GetMarkets().empty();
      }
      return true;
    };
    list.transact([&] {
      for(auto i = 0; i < list.get_size(); ++i) {
        auto index = *std::upper_bound(
          make_counting_iterator(0), make_counting_iterator(i), i,
          [&] (auto lhs, auto rhs) {
            return comparator(lhs, rhs);
          });
        if(index != i) {
          list.move(i, index);
        }
      }
    });
  }
}

RegionBox::RegionBox(std::shared_ptr<RegionQueryModel> regions, QWidget* parent)
  : RegionBox(
      std::move(regions), std::make_shared<LocalRegionModel>(), parent) {}

RegionBox::RegionBox(std::shared_ptr<RegionQueryModel> regions,
    std::shared_ptr<RegionModel> current, QWidget* parent)
    : QWidget(parent),
      m_regions(std::move(regions)),
      m_current(std::move(current)),
      m_last_region(m_current->get()),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&RegionBox::on_current, this))) {
  auto current_model = std::make_shared<ArrayListModel<Region>>();
  auto tags = to_tag_list(m_current->get(), *m_regions);
  current_model->transact([&] {
    for(auto& tag : tags) {
      current_model->push(tag);
    }
    sort(*current_model);
  });
  m_tag_combo_box = new TagComboBox(m_regions, std::move(current_model),
    [] (const auto& list, auto index) {
      return new RegionListItem(list->get(index));
    });
  m_tag_combo_box->connect_submit_signal(
    std::bind_front(&RegionBox::on_submit, this));
  m_tag_operation_connection =
    m_tag_combo_box->get_current()->connect_operation_signal(
      std::bind_front(&RegionBox::on_tags_operation, this));
  m_tag_combo_box->installEventFilter(this);
  enclose(*this, *m_tag_combo_box);
  proxy_style(*this, *m_tag_combo_box);
  setFocusProxy(m_tag_combo_box);
}

const std::shared_ptr<RegionQueryModel>& RegionBox::get_regions() const {
  return m_regions;
}

const std::shared_ptr<RegionModel>& RegionBox::get_current() const {
  return m_current;
}

void RegionBox::set_placeholder(const QString& placeholder) {
  m_tag_combo_box->set_placeholder(placeholder);
}

bool RegionBox::is_read_only() const {
  return m_tag_combo_box->is_read_only();
}

void RegionBox::set_read_only(bool is_read_only) {
  m_tag_combo_box->set_read_only(is_read_only);
}

connection RegionBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void RegionBox::on_current(const Region& region) {
  if(region == m_last_region) {
    return;
  }
  m_last_region = region;
  auto regions = to_tag_list(region, *m_regions);
  auto blocker = shared_connection_block(m_tag_operation_connection);
  auto current = m_tag_combo_box->get_current();
  current->transact([&] {
    while(current->get_size() > regions.size()) {
      current->remove(current->get_size() - 1);
    }
    auto i = std::size_t(0);
    while(i < current->get_size() && i < regions.size()) {
      if(current->get(i) != regions[i]) {
        current->set(i, regions[i]);
      }
      ++i;
    }
    while(i < regions.size()) {
      current->push(regions[i]);
      ++i;
    }
    sort(*current);
  });
}

void RegionBox::on_submit(
    const std::shared_ptr<ListModel<Region>>& submission) {
  sort(*m_tag_combo_box->get_current());
  sort(*submission);
  auto region = Nexus::Region();
  for(auto r : *submission) {
    region += r;
  }
  m_submit_signal(region);
}

void RegionBox::on_tags_operation(const AnyListModel::Operation& operation) {
  auto update_current = [&] {
    auto region = Nexus::Region();
    for(auto r : *m_tag_combo_box->get_current()) {
      region += r;
    }
    auto blocker = shared_connection_block(m_current_connection);
    if(m_current->set(region) != QValidator::Invalid) {
      m_last_region = std::move(region);
    }
  };
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      update_current();
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      update_current();
    });
}
