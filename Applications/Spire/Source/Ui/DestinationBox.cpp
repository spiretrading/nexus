#include "Spire/Ui/DestinationBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct DestinationBox::DestinationQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit DestinationQueryModel(std::shared_ptr<QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(value.has_value()) {
      return std::any_cast<DestinationDatabase::Entry&>(value).m_id;
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
        auto matches = [&] {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            return std::vector<std::any>();
          }
        }();
        auto result = std::vector<std::any>();
        for(auto& value : matches) {
          result.push_back(
            std::any_cast<DestinationDatabase::Entry&>(value).m_id);
        }
        return result;
    });
  }
};

DestinationBox::DestinationBox(
  std::shared_ptr<ComboBox::QueryModel> query_model, QWidget* parent)
  : DestinationBox(std::move(query_model),
      std::make_shared<LocalValueModel<Destination>>(), parent) {}

DestinationBox::DestinationBox(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_query_model(
        std::make_shared<DestinationQueryModel>(std::move(query_model))),
      m_current(std::move(current)) {
  auto combo_box_current = make_transform_value_model(m_current,
    [] (const Destination& current) {
      return std::any(current);
    },
    [] (const std::any& current) {
      return std::any_cast<Destination>(current);
    });
  m_combo_box = new ComboBox(m_query_model, combo_box_current,
    [=] (const auto& list, auto index) {
      return new DestinationListItem(
        std::any_cast<DestinationDatabase::Entry&&>(
          m_query_model->m_source->parse(displayText(list->get(index)))));
    });
  m_combo_box->connect_submit_signal([=] (const auto& submission) {
    m_submit_signal(std::any_cast<const Destination&>(submission));
  });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    DestinationBox::get_query_model() const {
  return m_query_model->m_source;
}

const std::shared_ptr<DestinationBox::CurrentModel>&
    DestinationBox::get_current() const {
  return m_current;
}

const Destination& DestinationBox::get_submission() const {
  auto& submission = m_combo_box->get_submission();
  if(submission.has_value()) {
    return std::any_cast<const Destination&>(submission);
  }
  static auto destination = Destination();
  return destination;
}

void DestinationBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool DestinationBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void DestinationBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection DestinationBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DestinationBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = static_cast<QKeyEvent&>(*event);
    if(key_event.key() == Qt::Key_Escape) {
      auto input_box =
        static_cast<AnyInputBox*>(m_combo_box->layout()->itemAt(0)->widget());
      if(!m_query_model->parse(
          any_cast<QString>(input_box->get_submission())).has_value()) {
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DestinationBox::showEvent(QShowEvent* event) {
  find_focus_proxy(*m_combo_box)->installEventFilter(this);
  QWidget::showEvent(event);
}
