#include "Spire/Ui/RegionBox.hpp"
#include <boost/iterator/counting_iterator.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include <QMoveEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/TagComboBox.hpp"

using namespace boost::iterators;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

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

  void sort(const std::shared_ptr<AnyListModel>& list) {
    auto comparator = [=] (const auto& lhs, const auto& rhs) {
      auto lhs_region = std::any_cast<Region&&>(list->get(lhs));
      auto rhs_region = std::any_cast<Region&&>(list->get(rhs));
      if(!lhs_region.GetCountries().empty() &&
        !rhs_region.GetCountries().empty()) {
        return displayText(*lhs_region.GetCountries().begin()) <
          displayText(*rhs_region.GetCountries().begin());
      } else if(!lhs_region.GetMarkets().empty() &&
        !rhs_region.GetMarkets().empty()) {
        return displayText(MarketToken(*lhs_region.GetMarkets().begin())) <
          displayText(MarketToken(*rhs_region.GetMarkets().begin()));
      } else if(!lhs_region.GetSecurities().empty() &&
        !rhs_region.GetSecurities().empty()) {
        return displayText(*lhs_region.GetSecurities().begin()) <
          displayText(*rhs_region.GetSecurities().begin());
      }
      if(!lhs_region.GetCountries().empty() ||
        !rhs_region.GetCountries().empty()) {
        return !lhs_region.GetCountries().empty();
      }
      if(!lhs_region.GetMarkets().empty() ||
        !rhs_region.GetMarkets().empty()) {
        return !lhs_region.GetMarkets().empty();
      }
      return true;
    };
    auto find_sorted_index = [=] (int index, int size) {
      if(index != 0 && comparator(index, index - 1)) {
        return *std::lower_bound(
          make_counting_iterator(0), make_counting_iterator(index), index,
          [&] (auto lhs, auto rhs) { return comparator(lhs, rhs); });
      } else if(index != size - 1 && comparator(index + 1, index)) {
        return *std::lower_bound(make_counting_iterator(index + 1),
          make_counting_iterator(size), index,
          [&] (auto lhs, auto rhs) { return comparator(lhs, rhs); }) - 1;
      }
      return index;
    };
    list->transact([&] {
      for(auto i = 0; i < list->get_size(); ++i) {
        auto index = find_sorted_index(i, i + 1);
        if(index != i) {
          list->move(i, index);
        }
      }
    });
  }
}

struct RegionBox::RegionQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit RegionQueryModel(std::shared_ptr<QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    return m_source->parse(query);
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([] (auto&& source_result) {
        auto& matches = [&] () -> std::vector<std::any>& {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            static auto empty_matches = std::vector<std::any>();
            return empty_matches;
          }
        }();
        auto result = std::vector<std::any>();
        auto regions = std::unordered_set<Region, RegionHash>();
        for(auto& value : matches) {
          auto& region = std::any_cast<Region&>(value);
          if(regions.insert(region).second) {
            result.push_back(region);
          }
        }
        return result;
    });
  }
};

RegionBox::RegionBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : RegionBox(query_model, std::make_shared<LocalValueModel<Region>>(),
      parent) {}

RegionBox::RegionBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_query_model(
        std::make_shared<RegionQueryModel>(std::move(query_model))),
      m_current(std::move(current)),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&RegionBox::on_current, this))) {
  auto current_model = std::make_shared<ArrayListModel<std::any>>();
  current_model->transact([&] {
    region_to_list_of_tags(m_current->get(), current_model);
    sort(current_model);
  });
  m_tag_combo_box = new TagComboBox(m_query_model, current_model,
    [] (const auto& list, auto index) {
      return new RegionListItem(std::any_cast<Region&&>(list->get(index)));
    });
  m_tag_combo_box->connect_submit_signal(
    std::bind_front(&RegionBox::on_submit, this));
  m_tag_operation_connection =
    m_tag_combo_box->get_current()->connect_operation_signal(
      std::bind_front(&RegionBox::on_tags_operation, this));
  m_tag_combo_box->installEventFilter(this);
  enclose(*this, *m_tag_combo_box);
  setFocusProxy(m_tag_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    RegionBox::get_query_model() const {
  return m_query_model->m_source;
}

const std::shared_ptr<RegionBox::CurrentModel>& RegionBox::get_current() const {
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

bool RegionBox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    update_min_max_size();
  }
  return QWidget::event(event);
}

void RegionBox::resizeEvent(QResizeEvent* event) {
  update_min_max_size();
  QWidget::resizeEvent(event);
}

void RegionBox::region_to_list_of_tags(const Region& region,
    const std::shared_ptr<AnyListModel>& list) {
  for(auto& country : region.GetCountries()) {
    list->push(m_query_model->parse(displayText(country)));
  }
  for(auto& market : region.GetMarkets()) {
    list->push(m_query_model->parse(displayText(MarketToken(market))));
  }
  for(auto& security : region.GetSecurities()) {
    list->push(m_query_model->parse(displayText(security)));
  }
}

void RegionBox::update_min_max_size() {
  if(m_tag_combo_box->minimumSize() != minimumSize()) {
    m_tag_combo_box->setMinimumSize(minimumSize());
  }
  if(m_tag_combo_box->maximumSize() != maximumSize()) {
    m_tag_combo_box->setMaximumSize(maximumSize());
  }
}

void RegionBox::on_current(const Region& region) {
  auto blocker = shared_connection_block(m_tag_operation_connection);
  auto& current = m_tag_combo_box->get_current();
  current->transact([&] {
    while(current->get_size() != 0) {
      current->remove(current->get_size() - 1);
    }
    region_to_list_of_tags(region, current);
  });
}

void RegionBox::on_submit(const std::shared_ptr<AnyListModel>& submission) {
  sort(m_tag_combo_box->get_current());
  sort(submission);
  auto region = Nexus::Region();
  for(auto i = 0; i < submission->get_size(); ++i) {
    region = region + std::any_cast<Region&&>(submission->get(i));
  }
  m_submit_signal(region);
}

void RegionBox::on_tags_operation(const AnyListModel::Operation& operation) {
  auto region = Nexus::Region();
  for(auto i = 0; i < m_tag_combo_box->get_current()->get_size(); ++i) {
    region =
      region + std::any_cast<Region&&>(m_tag_combo_box->get_current()->get(i));
  }
  auto blocker = shared_connection_block(m_current_connection);
  m_current->set(region);
}
