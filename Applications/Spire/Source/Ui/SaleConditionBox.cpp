#include "Spire/Ui/SaleConditionBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SaleConditionListItem.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct SaleConditionBox::SaleConditionQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit SaleConditionQueryModel(std::shared_ptr<QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(value.has_value()) {
      return std::any_cast<SaleConditionInfo&>(value).m_condition;
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
        auto conditions = std::unordered_set<std::string>();
        for(auto& value : matches) {
          auto& condition =
            std::any_cast<SaleConditionInfo&>(value).m_condition;
          if(conditions.insert(condition.m_code).second) {
            result.push_back(condition);
          }
        }
        return result;
    });
  }
};

SaleConditionBox::SaleConditionBox(
  std::shared_ptr<ComboBox::QueryModel> query_model, QWidget* parent)
  : SaleConditionBox(std::move(query_model),
      std::make_shared<LocalValueModel<TimeAndSale::Condition>>(), parent) {}

SaleConditionBox::SaleConditionBox(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_query_model(
        std::make_shared<SaleConditionQueryModel>(std::move(query_model))),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_rejected(false),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&SaleConditionBox::on_current, this))) {
  auto combo_box_current = make_transform_value_model(m_current,
    [=] (const TimeAndSale::Condition& current) {
      if(m_query_model->parse(
          QString::fromStdString(current.m_code)).has_value()) {
        return std::any(current);
      }
      return std::any();
    },
    [] (const std::any& current) {
      if(current.has_value()) {
        return std::any_cast<TimeAndSale::Condition>(current);
      }
      return TimeAndSale::Condition();
    });
  m_combo_box = new ComboBox(m_query_model, combo_box_current,
    [=] (const auto& list, auto index) {
      return new SaleConditionListItem(
        std::any_cast<SaleConditionInfo&&>(m_query_model->m_source->parse(
          QString::fromStdString(std::any_cast<TimeAndSale::Condition&&>(
            list->get(index)).m_code))));
    });
  m_combo_box->connect_submit_signal(
    std::bind_front(&SaleConditionBox::on_submit, this));
  enclose(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
  proxy_style(*this, *m_combo_box);
  m_input_box =
    static_cast<AnyInputBox*>(m_combo_box->layout()->itemAt(0)->widget());
  m_input_box->connect_submit_signal(
    std::bind_front(&SaleConditionBox::on_input_submit, this));
  m_input_box->installEventFilter(this);
}

const std::shared_ptr<ComboBox::QueryModel>&
    SaleConditionBox::get_query_model() const {
  return m_query_model->m_source;
}

const std::shared_ptr<SaleConditionBox::CurrentModel>&
    SaleConditionBox::get_current() const {
  return m_current;
}

const TimeAndSale::Condition& SaleConditionBox::get_submission() const {
  if(auto& submission = m_combo_box->get_submission(); submission.has_value()) {
    return std::any_cast<const TimeAndSale::Condition&>(submission);
  }
  static auto condition = TimeAndSale::Condition();
  return condition;
}

void SaleConditionBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool SaleConditionBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void SaleConditionBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection SaleConditionBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool SaleConditionBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = static_cast<QKeyEvent&>(*event);
    if(key_event.key() == Qt::Key_Escape) {
      if(watched == m_input_box) {
        if(m_submission == TimeAndSale::Condition()) {
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

void SaleConditionBox::showEvent(QShowEvent* event) {
  find_focus_proxy(*m_combo_box)->installEventFilter(this);
  QWidget::showEvent(event);
}

void SaleConditionBox::on_current(const TimeAndSale::Condition& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_input_box->layout()->itemAt(0)->widget(), Rejected());
  }
}

void SaleConditionBox::on_input_submit(const AnyRef& submission) {
  auto query = any_cast<QString>(submission);
  if(query.isEmpty()) {
    if(m_current->get() != TimeAndSale::Condition()) {
      m_current->set(TimeAndSale::Condition());
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

void SaleConditionBox::on_submit(const std::any& submission) {
  if(submission.has_value()) {
    m_submission = std::any_cast<TimeAndSale::Condition>(submission);
  } else {
    m_submission = TimeAndSale::Condition();
  }
  m_submit_signal(m_submission);
}
