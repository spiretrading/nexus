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
#include "Spire/Ui/Ui.hpp"

using namespace boost::iterators;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::vector<Region> to_tag_list(
      const Region& region, RegionQueryModel& regions) {
    auto tags = std::vector<Region>();
    for(auto& country : region.get_countries()) {
      tags.push_back(*regions.parse(to_text(country)));
    }
    for(auto& venue : region.get_venues()) {
      tags.push_back(*regions.parse(to_text(venue)));
    }
    for(auto& security : region.get_securities()) {
      tags.push_back(*regions.parse(to_text(security)));
    }
    return tags;
  }

  void sort(ListModel<Region>& list) {
    enum class EntryType : std::uint8_t {
      COUNTRY,
      VENUE,
      SECURITY,
      NONE
    };
    struct Entry {
      int m_index;
      EntryType m_type;
      QString m_key;

      Entry()
        : m_index(0),
          m_type(EntryType::NONE) {
      }
    };
    auto size = list.get_size();
    if(size <= 1) {
      return;
    }
    auto entries = std::vector<Entry>();
    entries.reserve(size);
    for(auto i = 0; i < size; ++i) {
      auto entry = Entry();
      entry.m_index = i;
      auto& region = list.get(i);
      auto& countries = region.get_countries();
      auto& venues = region.get_venues();
      auto& securities = region.get_securities();
      if(!countries.empty()) {
        entry.m_type = EntryType::COUNTRY;
        entry.m_key = to_text(*countries.begin());
      } else if(!venues.empty()) {
        entry.m_type = EntryType::VENUE;
        entry.m_key = to_text(*venues.begin());
      } else if(!securities.empty()) {
        entry.m_type = EntryType::SECURITY;
        entry.m_key = to_text(*securities.begin());
      } else {
        entry.m_type = EntryType::NONE;
      }
      entries.emplace_back(std::move(entry));
    }
    std::stable_sort(entries.begin(), entries.end(),
      [] (const auto& a, const auto& b) {
        if(a.m_type != b.m_type) {
          return a.m_type < b.m_type;
        }
        return a.m_key < b.m_key;
      });
    auto current_pos = std::vector<int>(size);
    std::iota(current_pos.begin(), current_pos.end(), 0);
    auto placed = std::vector<bool>(size, false);
    list.transact([&] {
      for(auto i = 0; i < size; ++i) {
        auto original = entries[i].m_index;
        if(placed[original]) {
          continue;
        }
        auto from = current_pos[original];
        auto to = i;
        if(from != to) {
          list.move(from, to);
          for(auto j = 0; j < size; ++j) {
            auto& pos = current_pos[j];
            if(pos >= to && pos < from) {
              ++pos;
            } else if(pos <= to && pos > from) {
              --pos;
            }
          }
          current_pos[original] = to;
        }
        placed[original] = true;
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
  invalidate_descendant_layouts(*this);
  adjustSize();
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
