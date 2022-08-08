#include "Spire/Ui/RegionBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QMoveEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/TagComboBox.hpp"

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
  m_tag_combo_box = new TagComboBox(m_query_model,
    std::make_shared<ArrayListModel<std::any>>(),
    [] (const auto& list, auto index) {
      return new RegionListItem(std::any_cast<Region&&>(list->get(index)));
    });
  m_tag_combo_box->connect_submit_signal(
    std::bind_front(&RegionBox::on_submit, this));
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

void RegionBox::update_min_max_size() {
  if(m_tag_combo_box->minimumSize() != minimumSize()) {
    m_tag_combo_box->setMinimumSize(minimumSize());
  }
  if(m_tag_combo_box->maximumSize() != maximumSize()) {
    m_tag_combo_box->setMaximumSize(maximumSize());
  }
}

void RegionBox::on_current(const Region& region) {
  auto& current = m_tag_combo_box->get_current();
  current->transact([&] {
    while(current->get_size() != 0) {
      current->remove(current->get_size() - 1);
    }
  });
  current->push(region);
}

void RegionBox::on_submit(const std::shared_ptr<AnyListModel>& submission) {
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
