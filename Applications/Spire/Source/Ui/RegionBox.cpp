#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/TagComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  struct hash {
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
        auto matches = [&] {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            return std::vector<std::any>();
          }
        }();
        auto result = std::vector<std::any>();
        auto regions = std::unordered_set<Region, hash>();
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
      m_current(std::move(current)) {
  auto tag_combo_box_current = make_transform_value_model(m_current,
    [] (const Region& current) {
      return std::any(current);
    },
    [] (const std::any& current) {
      return std::any_cast<Region>(current);
    });
  m_tag_combo_box = new TagComboBox(m_query_model,
    std::make_shared<ArrayListModel<std::any>>(),
    tag_combo_box_current, [] (const auto& list, auto index) {
      return new RegionListItem(std::any_cast<Region&&>(list->get(index)));
    });
  m_tag_combo_box->connect_submit_signal(
    std::bind_front(&RegionBox::on_submit, this));
  enclose(*this, *m_tag_combo_box);
  setFocusProxy(m_tag_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>& RegionBox::get_query_model() const {
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

void RegionBox::on_submit(const std::shared_ptr<AnyListModel>& submission) {
  if(submission->get_size() == 1) {
    m_submit_signal(std::any_cast<Region&&>(submission->get(0)));
    return;
  }
  auto region = Nexus::Region();
  for(auto i = 0; i < submission->get_size(); ++i) {
    region = region + std::any_cast<Region&&>(submission->get(i));
  }
  m_submit_signal(region);
}
