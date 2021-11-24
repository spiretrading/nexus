#include "Spire/Ui/ComboBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model, QWidget* parent)
  : ComboBox(std::move(query_model), &ListView::default_view_builder, parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
  ViewBuilder view_builder, QWidget* parent)
  : ComboBox(std::move(query_model),
      std::make_shared<LocalValueModel<std::any>>(),
      std::move(view_builder), parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_query_model(std::move(query_model)),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_read_only(false),
      m_matches(std::make_shared<ArrayListModel>()) {
  update_style(*this, [] (auto& style) {
    style.get(FocusIn()).set(border_color(QColor(0x4B23A0)));
  });
  m_input_box = new TextBox();
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(m_input_box);
  m_input_connection = m_input_box->get_current()->connect_update_signal(
    std::bind_front(&ComboBox::on_input, this));
  m_list_view = new ListView(m_matches, std::move(view_builder));
  m_drop_down_list = new DropDownList(*m_list_view, *this);
  m_drop_down_list->installEventFilter(this);
  m_drop_down_list->get_list_view().connect_submit_signal(
    std::bind_front(&ComboBox::on_drop_down_submit, this));
}

const std::shared_ptr<ComboBox::QueryModel>& ComboBox::get_query_model() const {
  return m_query_model;
}

const std::shared_ptr<ComboBox::CurrentModel>& ComboBox::get_current() const {
  return m_current;
}

const std::any& ComboBox::get_submission() const {
  return m_submission;
}

bool ComboBox::is_read_only() const {
  return m_is_read_only;
}

void ComboBox::set_read_only(bool is_read_only) {
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  m_input_box->set_read_only(m_is_read_only);
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection ComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Show) {
    match(*this, PopUp());
  } else if(event->type() == QEvent::Hide) {
    unmatch(*this, PopUp());
  }
  return QWidget::eventFilter(watched, event);
}

void ComboBox::keyPressEvent(QKeyEvent* event) {
  QCoreApplication::sendEvent(&m_drop_down_list->get_list_view(), event);
}

void ComboBox::on_input(const QString& query) {
  if(query.isEmpty()) {
    on_query(std::vector<std::any>());
  } else {
    m_query_result = m_query_model->submit(query);
    m_query_result.then(std::bind_front(&ComboBox::on_query, this));
    auto value = m_query_model->parse(query);
    if(value.has_value()) {
      m_current->set(value);
    }
  }
}

void ComboBox::on_query(Expect<std::vector<std::any>>&& result) {
  auto selection = [&] {
    try {
      return result.Get();
    } catch(const std::exception&) {
      return std::vector<std::any>();
    }
  }();
  m_matches->transact([&] {
    while(m_matches->get_size() != 0) {
      m_matches->remove(m_matches->get_size() - 1);
    }
    for(auto& item : selection) {
      m_matches->push(item);
    }
  });
  if(selection.empty()) {
    m_drop_down_list->hide();
  } else if(!m_drop_down_list->isVisible()) {
    m_drop_down_list->show();
  }
}

void ComboBox::on_drop_down_submit(const std::any& submission) {
  m_current->set(submission);
  m_submission = submission;
  m_drop_down_list->hide();
  m_submit_signal(submission);
}

LocalComboBoxQueryModel::LocalComboBoxQueryModel()
  : m_values(QChar()) {}

void LocalComboBoxQueryModel::add(const std::any& value) {
  add(displayTextAny(value).toLower(), value);
}

void LocalComboBoxQueryModel::add(const QString& id, const std::any& value) {
  m_values[id.data()] = value;
}

std::any LocalComboBoxQueryModel::parse(const QString& query) {
  auto i = m_values.find(query.toLower().data());
  if(i == m_values.end()) {
    return {};
  }
  return *i->second;
}

QtPromise<std::vector<std::any>> LocalComboBoxQueryModel::submit(
    const QString& query) {
  auto matches = std::vector<std::any>();
  for(auto i = m_values.startsWith(query.data()); i != m_values.end(); ++i) {
    matches.push_back(*i->second);
  }
  return QtPromise(std::move(matches));
}
