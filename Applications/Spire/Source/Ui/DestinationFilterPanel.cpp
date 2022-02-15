#include "Spire/Ui/DestinationFilterPanel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct DestinationFilterQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;
  std::shared_ptr<AnyListModel> m_matches;
  std::unordered_set<QString> m_matches_set;
  std::vector<QString> m_matches_list;
  scoped_connection m_matches_connection;

  DestinationFilterQueryModel(std::shared_ptr<ComboBox::QueryModel> source,
      std::shared_ptr<AnyListModel> matches)
      : m_source(std::move(source)),
        m_matches(std::move(matches)),
        m_matches_connection(m_matches->connect_operation_signal(
          std::bind_front(&DestinationFilterQueryModel::on_operation, this))) {
    for(auto i = 0; i < m_matches->get_size(); ++i) {
      add_match(i);
    }
  }

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_matches_set.contains(displayTextAny(
        std::any_cast<DestinationDatabase::Entry&>(value).m_id))) {
      return std::any();
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
      auto result = [&] {
        try {
          return source_result.Get();
        } catch(const std::exception&) {
          return std::vector<std::any>();
        }
      }();
      std::erase_if(result, [&] (auto& value) {
        return m_matches_set.contains(displayTextAny(
          std::any_cast<DestinationDatabase::Entry&>(value).m_id));
      });
      return result;
    });
  }

  void on_operation(const AnyListModel::Operation& operation) {
    visit(operation,
      [&] (const AnyListModel::AddOperation& operation) {
        add_match(operation.m_index);
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        m_matches_set.erase(m_matches_list[operation.m_index]);
        m_matches_list.erase(m_matches_list.begin() + operation.m_index);
      });
  }

  void add_match(int index) {
    auto value = displayTextAny(m_matches->get(index));
    m_matches_set.insert(value);
    m_matches_list.insert(m_matches_list.begin() + index, value);
  }
};

AnyInputBox* destination_box_builder(std::shared_ptr<ComboBox::QueryModel> model,
    std::shared_ptr<AnyListModel> matches) {
  auto box = new DestinationBox(
    std::make_shared<DestinationFilterQueryModel>(model, matches));
  box->set_placeholder(QObject::tr("Search destinations"));
  auto input_box = new AnyInputBox(*box);
  input_box->connect_submit_signal([=] (const auto& submission) {
    input_box->get_current()->set(Destination());
    matches->push(any_cast<Destination>(submission));
  });
  return input_box;
}

DestinationFilterPanel* Spire::make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent) {
  return make_destination_filter_panel(std::move(query_model),
    std::make_shared<ArrayListModel<Destination>>(),
    std::make_shared<LocalValueModel<DestinationFilterPanel::Mode>>(
      DestinationFilterPanel::Mode::INCLUDE), parent);
}

DestinationFilterPanel* Spire::make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Destination>> matches,
    std::shared_ptr<ValueModel<DestinationFilterPanel::Mode>> mode,
    QWidget& parent) {
  return new DestinationFilterPanel(
    std::bind_front(destination_box_builder, std::move(query_model)),
    std::move(matches), std::move(mode), QObject::tr("Filter by Destination"),
    parent);
}
