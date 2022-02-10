#include "Spire/Ui/SecurityFilterPanel.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct SecurityFilterQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;
  std::shared_ptr<AnyListModel> m_matches;
  std::unordered_set<QString> m_matches_set;
  std::vector<QString> m_matches_list;
  scoped_connection m_matches_connection;

  SecurityFilterQueryModel(std::shared_ptr<ComboBox::QueryModel> source,
      std::shared_ptr<AnyListModel> matches)
      : m_source(std::move(source)),
        m_matches(std::move(matches)),
        m_matches_connection(m_matches->connect_operation_signal(
          std::bind_front(&SecurityFilterQueryModel::on_operation, this))) {
    for(auto i = 0; i < m_matches->get_size(); ++i) {
      add_match(i);
    }
  }

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_matches_set.contains(
        displayTextAny(std::any_cast<SecurityInfo&>(value).m_security))) {
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
        return m_matches_set.contains(
          displayTextAny(std::any_cast<SecurityInfo&>(value).m_security));
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

AnyInputBox* security_box_builder(std::shared_ptr<ComboBox::QueryModel> model,
    std::shared_ptr<AnyListModel> matches) {
  auto box = new SecurityBox(
    std::make_shared<SecurityFilterQueryModel>(model, matches));
  box->set_placeholder(QObject::tr("Search securities"));
  auto input_box = new AnyInputBox(*box);
  input_box->connect_submit_signal([=] (const auto& submission) {
    input_box->get_current()->set(Security());
    matches->push(any_cast<Security>(submission));
  });
  return input_box;
}

SecurityFilterPanel::SecurityFilterPanel(
  std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent)
  : SecurityFilterPanel(std::move(query_model),
    std::make_shared<ArrayListModel<Security>>(),
    std::make_shared<LocalValueModel<Mode>>(Mode::INCLUDE), parent) {}

SecurityFilterPanel::SecurityFilterPanel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Security>> matches,
    std::shared_ptr<ValueModel<Mode>> mode, QWidget& parent)
    : QWidget(&parent),
      m_query_model(std::move(query_model)),
      m_matches(std::move(matches)) {
  m_panel = new OpenFilterPanel(
    std::bind_front(security_box_builder, m_query_model), m_matches,
    std::move(mode), QObject::tr("Filter by Security"), parent);
  m_panel->connect_submit_signal([=] (const auto& matches, auto mode) {
    m_submit_signal(m_matches, mode);
  });
  m_panel->window()->installEventFilter(this);
  hide();
}

const std::shared_ptr<ComboBox::QueryModel>&
    SecurityFilterPanel::get_query_model() const {
  return m_query_model;
}

const std::shared_ptr<ListModel<Nexus::Security>>&
    SecurityFilterPanel::get_matches() const {
  return m_matches;
}

const std::shared_ptr<ValueModel<SecurityFilterPanel::Mode>>&
    SecurityFilterPanel::get_mode() const {
  return m_panel->get_mode();
}

connection SecurityFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool SecurityFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    m_panel->hide();
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool SecurityFilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::Show) {
    m_panel->show();
  } else if(event->type() == QEvent::Hide) {
    m_panel->hide();
  }
  return QWidget::event(event);
}
