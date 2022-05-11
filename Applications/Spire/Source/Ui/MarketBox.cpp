#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_destination(const MarketDatabase::Entry& market) {
    auto destination_entry = DestinationDatabase::Entry();
    destination_entry.m_id = market.m_displayName;
    destination_entry.m_description = market.m_description;
    return destination_entry;
  }

  auto to_market(const std::shared_ptr<ComboBox::QueryModel>& model,
      const Destination& destination) {
    auto market_entry = model->parse(QString::fromStdString(destination));
    if(market_entry.has_value()) {
      return std::any_cast<MarketDatabase::Entry>(market_entry).m_code;
    }
    return MarketCode();
  }
}

struct MarketBox::MarketQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit MarketQueryModel(std::shared_ptr<ComboBox::QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(value.has_value()) {
      return to_destination(std::any_cast<MarketDatabase::Entry&>(value));
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
        auto& matches = [&] () -> std::vector<std::any>& {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            static auto empty_matches = std::vector<std::any>();
            return empty_matches;
          }
        }();
        auto result = std::vector<std::any>();
        for(auto& value : matches) {
          result.push_back(
            to_destination(std::any_cast<MarketDatabase::Entry&>(value)));
        }
        return result;
    });
  }
};

MarketBox::MarketBox(
  std::shared_ptr<ComboBox::QueryModel> query_model, QWidget* parent)
  : MarketBox(std::move(query_model),
      std::make_shared<LocalValueModel<MarketCode>>(), parent) {}

MarketBox::MarketBox(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_query_model(
        std::make_shared<MarketQueryModel>(std::move(query_model))),
      m_current(std::move(current)) {
  auto destination_box_current = make_transform_value_model(m_current,
    [=] (const MarketCode& current) {
      auto destination = m_query_model->parse(current.GetData());
      if(destination.has_value()) {
        return std::any_cast<DestinationDatabase::Entry>(destination).m_id;
      }
      return Destination();
    },
    [=] (const Destination& current) {
      return to_market(m_query_model->m_source, current);
    });
  m_destination_box =
    new DestinationBox(m_query_model, destination_box_current);
  m_destination_box->connect_submit_signal([=] (const auto& submission) {
    m_submission = to_market(m_query_model->m_source, submission);
    m_submit_signal(m_submission);
  });
  enclose(*this, *m_destination_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    MarketBox::get_query_model() const {
  return m_query_model->m_source;
}

const std::shared_ptr<MarketBox::CurrentModel>& MarketBox::get_current() const {
  return m_current;
}

const MarketCode& MarketBox::get_submission() const {
  return m_submission;
}

void MarketBox::set_placeholder(const QString& placeholder) {
  m_destination_box->set_placeholder(placeholder);
}

bool MarketBox::is_read_only() const {
  return m_destination_box->is_read_only();
}

void MarketBox::set_read_only(bool is_read_only) {
  m_destination_box->set_read_only(is_read_only);
}

connection MarketBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
