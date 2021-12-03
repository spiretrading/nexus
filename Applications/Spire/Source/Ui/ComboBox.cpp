#include "Spire/Ui/ComboBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
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
      m_submission_text(displayTextAny(m_submission)),
      m_is_read_only(false),
      m_focus_observer(*this),
      m_matches(std::make_shared<ArrayListModel>()),
      m_completion_tag(0),
      m_has_autocomplete_selection(false) {
  m_input_box = new TextBox();
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  m_input_box->installEventFilter(this);
  m_input_box->connect_submit_signal(
    std::bind_front(&ComboBox::on_submit, this));
  m_input_connection = m_input_box->get_current()->connect_update_signal(
    std::bind_front(&ComboBox::on_input, this));
  m_highlight_connection = m_input_box->get_highlight()->connect_update_signal(
    std::bind_front(&ComboBox::on_highlight, this));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(m_input_box);
  m_list_view = new ListView(m_matches, std::move(view_builder));
  m_drop_down_list = new DropDownList(*m_list_view, *this);
  m_drop_down_list->installEventFilter(this);
  m_current_connection =
    m_drop_down_list->get_list_view().get_current()->connect_update_signal(
      std::bind_front(&ComboBox::on_drop_down_current, this));
  m_drop_down_list->get_list_view().connect_submit_signal(
    std::bind_front(&ComboBox::on_drop_down_submit, this));
  m_focus_observer.connect_state_signal(
    std::bind_front(&ComboBox::on_focus, this));
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
  if(watched == m_drop_down_list) {
    if(event->type() == QEvent::Show) {
      match(*this, PopUp());
    } else if(event->type() == QEvent::Hide) {
      unmatch(*this, PopUp());
    }
  } else if(watched == m_input_box) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = static_cast<QKeyEvent&>(*event);
      if(key_event.key() == Qt::Key_Escape) {
        if(m_drop_down_list->isVisible()) {
          m_drop_down_list->hide();
          revert_current();
        } else {
          revert_to(m_submission_text, false);
        }
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ComboBox::keyPressEvent(QKeyEvent* event) {
  if(m_drop_down_list->isVisible() &&
      (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
      event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)) {
    auto is_top_current =
      m_drop_down_list->get_list_view().get_current()->get() == 0;
    QCoreApplication::sendEvent(&m_drop_down_list->get_list_view(), event);
    if(event->key() == Qt::Key_Up && is_top_current &&
        m_drop_down_list->get_list_view().get_current()->get() == 0) {
      revert_current();
    }
  }
  if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
    m_drop_down_list->setVisible(
      m_drop_down_list->get_list_view().get_list()->get_size() != 0);
    event->accept();
    return;
  } else if(event->key() == Qt::Key_Escape) {
    if(m_drop_down_list->get_list_view().get_selection().get() == 0) {
      m_drop_down_list->hide();
      return;
    }
  }
  return QWidget::keyPressEvent(event);
}

void ComboBox::update_completion() {
  if(m_matches->get_size() != 0) {
    auto& highlight = *m_input_box->get_highlight();
    auto& query = m_input_box->get_current()->get();
    if(highlight.get().m_end != query.size()) {
      m_prefix.clear();
      m_completion.clear();
      return;
    }
    auto top_match = displayTextAny(m_matches->at(0));
    if(!top_match.toLower().startsWith(query.toLower())) {
      m_prefix = query;
      m_completion.clear();
      return;
    }
    auto prefix = top_match.mid(0, query.size()).toLower();
    auto completion = top_match.mid(query.size());
    if(m_last_completion.size() < query.size()) {
      m_last_completion = query;
      auto selection_start = query.size();
      {
        auto blocker = shared_connection_block(m_input_connection);
        m_input_box->get_current()->set(query + completion);
      }
      m_has_autocomplete_selection = false;
      highlight.set({selection_start + completion.size(), selection_start});
      m_has_autocomplete_selection = true;
    } else {
      m_last_completion = query;
    }
    m_prefix = std::move(prefix);
    m_completion = std::move(completion);
  } else {
    m_last_completion.clear();
    m_prefix = m_input_box->get_current()->get();
    m_completion.clear();
  }
}

void ComboBox::revert_to(const QString& query, bool autocomplete) {
  auto blocker = shared_connection_block(m_input_connection);
  m_input_box->get_current()->set(query);
  m_has_autocomplete_selection = false;
  m_last_completion.clear();
  m_prefix = query;
  m_completion.clear();
  if(autocomplete) {
    update_completion();
  }
}

void ComboBox::revert_current() {
  auto& list_view = m_drop_down_list->get_list_view();
  list_view.get_current()->set(none);
  list_view.get_selection()->set(none);
  if(m_user_query) {
    revert_to(*m_user_query, true);
  }
}

void ComboBox::submit(const QString& query, bool is_passive) {
  auto value = m_query_model->parse(query);
  if(!value.has_value()) {
    return;
  }
  if(is_passive && displayTextAny(value) == displayTextAny(m_submission)) {
    return;
  }
  if(!m_completion.isEmpty()) {
    auto blocker = shared_connection_block(m_input_connection);
    m_input_box->get_current()->set(query);
    m_has_autocomplete_selection = false;
    m_current->set(value);
  }
  m_last_completion = query;
  m_prefix = query;
  m_completion.clear();
  m_submission = value;
  m_submission_text = query;
  m_input_box->get_highlight()->set(Highlight(query.size()));
  m_drop_down_list->hide();
  m_query_result = m_query_model->submit(query);
  m_query_result.then(
    std::bind_front(&ComboBox::on_query, this, ++m_completion_tag, false));
  m_submit_signal(value);
}

void ComboBox::on_input(const QString& query) {
  m_user_query = query;
  m_has_autocomplete_selection = false;
  if(query.isEmpty()) {
    on_query(++m_completion_tag, true, std::vector<std::any>());
  } else {
    m_query_result = m_query_model->submit(query);
    m_query_result.then(
      std::bind_front(&ComboBox::on_query, this, ++m_completion_tag, true));
    auto value = m_query_model->parse(query);
    if(value.has_value()) {
      m_current->set(value);
    }
  }
}

void ComboBox::on_highlight(const Highlight& highlight) {
  if(!m_has_autocomplete_selection) {
    return;
  }
  m_has_autocomplete_selection = false;
  auto& query = m_input_box->get_current()->get();
  auto value = m_query_model->parse(query);
  if(!value.has_value()) {
    return;
  }
  m_prefix = query;
  m_completion.clear();
  m_current->set(value);
}

void ComboBox::on_submit(const QString& query) {
  if(find_focus_state(*m_input_box) == FocusObserver::State::NONE) {
    return;
  }
  m_user_query = query;
  submit(query);
}

void ComboBox::on_query(
    std::uint32_t tag, bool show, Expect<std::vector<std::any>>&& result) {
  if(m_completion_tag != tag) {
    return;
  }
  auto selection = [&] {
    try {
      return result.Get();
    } catch(const std::exception&) {
      return std::vector<std::any>();
    }
  }();
  {
    auto blocker = shared_connection_block(m_current_connection);
    while(m_matches->get_size() != 0) {
      m_matches->remove(m_matches->get_size() - 1);
    }
    for(auto& item : selection) {
      m_matches->push(item);
    }
  }
  update_completion();
  if(show) {
    if(selection.empty()) {
      m_drop_down_list->hide();
    } else if(m_focus_observer.get_state() != FocusObserver::State::NONE &&
        !m_drop_down_list->isVisible()) {
      auto blocker = shared_connection_block(m_current_connection);
      m_drop_down_list->get_list_view().get_current()->set(none);
      m_drop_down_list->get_list_view().get_selection()->set(none);
      m_drop_down_list->show();
    }
  }
}

void ComboBox::on_drop_down_current(optional<int> index) {
  if(index) {
    auto& value = m_drop_down_list->get_list_view().get_list()->at(*index);
    auto text = displayTextAny(value);
    {
      auto input_blocker = shared_connection_block(m_input_connection);
      auto highlight_blocker = shared_connection_block(m_highlight_connection);
      m_input_box->get_current()->set(text);
    }
    m_last_completion = text;
    m_completion.clear();
    m_prefix.clear();
    m_has_autocomplete_selection = false;
    m_current->set(value);
  }
}

void ComboBox::on_drop_down_submit(const std::any& submission) {
  auto text = displayTextAny(submission);
  {
    auto input_blocker = shared_connection_block(m_input_connection);
    auto highlight_blocker = shared_connection_block(m_highlight_connection);
    m_input_box->get_current()->set(text);
    m_has_autocomplete_selection = false;
    m_input_box->get_highlight()->set(Highlight(text.size()));
  }
  m_submission = submission;
  m_submission_text = text;
  m_drop_down_list->hide();
  m_submit_signal(submission);
}

void ComboBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    m_drop_down_list->hide();
    submit(m_input_box->get_current()->get(), true);
  }
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
  for(auto i = m_values.startsWith(query.toLower().data());
      i != m_values.end(); ++i) {
    matches.push_back(*i->second);
  }
  return QtPromise(std::move(matches));
}
