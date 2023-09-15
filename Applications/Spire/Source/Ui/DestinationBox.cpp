#include "Spire/Ui/DestinationBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Ui/Layouts.hpp"

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
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_rejected(false),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&DestinationBox::on_current, this))) {
  auto combo_box_current = make_transform_value_model(m_current,
    [=] (const Destination& current) {
      if(m_query_model->parse(to_text(current)).has_value()) {
        return std::any(current);
      }
      return std::any();
    },
    [] (const std::any& current) {
      if(current.has_value()) {
        return std::any_cast<Destination>(current);
      }
      return Destination();
    });
  m_combo_box = new ComboBox(m_query_model, combo_box_current,
    [=] (const auto& list, auto index) {
      return new DestinationListItem(
        std::any_cast<DestinationDatabase::Entry&&>(
          m_query_model->m_source->parse(to_text(list->get(index)))));
    });
  m_combo_box->connect_submit_signal(
    std::bind_front(&DestinationBox::on_submit, this));
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
  m_input_box =
    static_cast<AnyInputBox*>(m_combo_box->layout()->itemAt(0)->widget());
  m_input_box->connect_submit_signal(
    std::bind_front(&DestinationBox::on_input_submit, this));
  m_input_box->installEventFilter(this);
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
  return m_submission;
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
      if(watched == m_input_box) {
        if(m_submission == Destination()) {
          if(!any_cast<QString>(m_input_box->get_current()->get()).isEmpty()) {
            m_current->set(m_submission);
          }
          event->ignore();
          return true;
        }
      } else if(!m_query_model->parse(
          any_cast<QString>(m_input_box->get_submission())).has_value()) {
        event->ignore();
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

void DestinationBox::on_current(const Destination& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_input_box->layout()->itemAt(0)->widget(), Rejected());
  }
}

void DestinationBox::on_input_submit(const AnyRef& submission) {
  auto query = any_cast<QString>(submission);
  if(query.isEmpty()) {
    if(m_current->get() != Destination()) {
      m_current->set(Destination());
    }
    on_submit(std::any());
  } else if(!m_query_model->parse(query).has_value()) {
    m_current->set(m_submission);
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*m_input_box->layout()->itemAt(0)->widget(), Rejected());
    }
  }
}

void DestinationBox::on_submit(const std::any& submission) {
  if(submission.has_value()) {
    m_submission = std::any_cast<Destination>(submission);
  } else {
    m_submission = Destination();
  }
  m_submit_signal(m_submission);
}
