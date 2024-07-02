#include "Spire/Ui/ComboBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct ComboBoxTextModel : TextModel {
    std::shared_ptr<ComboBox::CurrentModel> m_current;
    scoped_connection m_connection;
    LocalTextModel m_value;

    ComboBoxTextModel(std::shared_ptr<ComboBox::CurrentModel> current)
        : m_current(current),
          m_value(to_text(m_current->get())) {
      m_connection = m_current->connect_update_signal(
        std::bind_front(&ComboBoxTextModel::on_update, this));
    }

    const Type& get() const {
      return m_value.get();
    }

    QValidator::State test(const Type& value) const {
      return m_value.test(value);
    }

    QValidator::State set(const Type& value) {
      return m_value.set(value);
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
      return m_value.connect_update_signal(slot);
    }

    void on_update(const std::any& current) {
      auto text = to_text(current);
      if(m_value.get() != text) {
        m_value.set(text);
      }
    }
  };
}

ComboBox::DeferredData::DeferredData(ComboBox& box)
  : m_submission(box.m_current->get()),
    m_submission_text(to_text(m_submission)),
    m_input_focus_proxy(nullptr),
    m_list_view(nullptr),
    m_focus_observer(box),
    m_key_observer(*box.m_input_box),
    m_matches(std::make_shared<ArrayListModel<std::any>>()),
    m_drop_down_list(nullptr),
    m_completion_tag(0),
    m_has_autocomplete_selection(false),
    m_current_connection(box.m_current->connect_update_signal(
      std::bind_front(&ComboBox::on_current, &box))) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model, QWidget* parent)
  : ComboBox(std::move(query_model), &ListView::default_item_builder, parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
  ListViewItemBuilder<> item_builder, QWidget* parent)
  : ComboBox(std::move(query_model),
      std::make_shared<LocalValueModel<std::any>>(), std::move(item_builder),
      parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
  std::shared_ptr<CurrentModel> current, ListViewItemBuilder<> item_builder,
  QWidget* parent)
  : ComboBox(std::move(query_model), current, new AnyInputBox(
      *(new TextBox(std::make_shared<ComboBoxTextModel>(current)))),
      std::move(item_builder), parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, AnyInputBox* input_box,
    ListViewItemBuilder<> item_builder, QWidget* parent)
    : QWidget(parent),
      m_query_model(std::move(query_model)),
      m_current(std::move(current)),
      m_input_box(input_box),
      m_item_builder(std::move(item_builder)) {
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  enclose(*this, *m_input_box);
}

const std::shared_ptr<ComboBox::QueryModel>& ComboBox::get_query_model() const {
  return m_query_model;
}

const std::shared_ptr<ComboBox::CurrentModel>& ComboBox::get_current() const {
  return m_current;
}

const std::any& ComboBox::get_submission() const {
  initialize_deferred_data();
  return m_data->m_submission;
}

void ComboBox::set_placeholder(const QString& placeholder) {
  m_input_box->set_placeholder(placeholder);
}

bool ComboBox::is_read_only() const {
  return m_input_box->is_read_only();
}

void ComboBox::set_read_only(bool is_read_only) {
  if(!is_read_only) {
    initialize_deferred_data();
  }
  m_input_box->set_read_only(is_read_only);
}

connection ComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  initialize_deferred_data();
  return m_data->m_submit_signal.connect(slot);
}

bool ComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_data->m_drop_down_list) {
    if(event->type() == QEvent::Show) {
      match(*this, PopUp());
    } else if(event->type() == QEvent::Hide) {
      unmatch(*this, PopUp());
    }
  } else if(watched == m_input_box) {
    if(event->type() == QEvent::Show) {
      update_focus_proxy();
    }
  } else if(watched == m_data->m_input_focus_proxy) {
    if(event->type() == QEvent::FocusOut &&
        m_data->m_drop_down_list->isVisible()) {
      return true;
    }
  } else if(watched == m_data->m_drop_down_list->window()) {
    if(event->type() == QEvent::KeyPress) {
      return QCoreApplication::sendEvent(m_data->m_input_focus_proxy, event);
    } else if(event->type() == QEvent::ContextMenu) {
      auto& menu_event = *static_cast<QContextMenuEvent*>(event);
      if(rect().contains(mapFromGlobal(menu_event.globalPos()))) {
        auto new_event = QContextMenuEvent(menu_event.reason(),
          m_data->m_input_focus_proxy->mapFromGlobal(menu_event.globalPos()),
          menu_event.globalPos(), menu_event.modifiers());
        return QCoreApplication::sendEvent(
          m_data->m_input_focus_proxy, &new_event);
      }
    } else if(event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseMove) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        auto new_event = QMouseEvent(mouse_event.type(),
          m_data->m_input_focus_proxy->mapFromGlobal(mouse_event.globalPos()),
          mouse_event.windowPos(), mouse_event.screenPos(),
          mouse_event.button(), mouse_event.buttons(), mouse_event.modifiers(),
          mouse_event.source());
        return QCoreApplication::sendEvent(
          m_data->m_input_focus_proxy, &new_event);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ComboBox::keyPressEvent(QKeyEvent* event) {
  if(is_read_only()) {
    return QWidget::keyPressEvent(event);
  }
  if(m_data->m_drop_down_list->isVisible() &&
      (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
      event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)) {
    auto is_top_current =
      m_data->m_drop_down_list->get_list_view().get_current()->get() == 0;
    QCoreApplication::sendEvent(
      &m_data->m_drop_down_list->get_list_view(), event);
    if(event->key() == Qt::Key_Up && is_top_current &&
        m_data->m_drop_down_list->get_list_view().get_current()->get() == 0) {
      revert_current();
    }
  }
  if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
    m_data->m_drop_down_list->setVisible(
      m_data->m_drop_down_list->get_list_view().get_list()->get_size() != 0);
    event->accept();
    return;
  } else if(event->key() == Qt::Key_Escape) {
    if(m_data->m_drop_down_list->get_list_view().get_selection().get() == 0) {
      m_data->m_drop_down_list->hide();
      return;
    }
  }
  return QWidget::keyPressEvent(event);
}

void ComboBox::showEvent(QShowEvent* event) {
  if(!is_read_only()) {
    initialize_deferred_data();
  }
  QWidget::showEvent(event);
}

void ComboBox::update_focus_proxy() {
  auto proxy = find_focus_proxy(*m_input_box);
  if(proxy != m_data->m_input_focus_proxy) {
    if(m_data->m_input_focus_proxy) {
      m_data->m_input_focus_proxy->removeEventFilter(this);
    }
    m_data->m_input_focus_proxy = proxy;
    if(proxy) {
      proxy->installEventFilter(this);
    }
  }
}

void ComboBox::initialize_deferred_data() const {
  if(m_data) {
    return;
  }
  auto self = const_cast<ComboBox*>(this);
  self->m_data = std::make_unique<DeferredData>(*self);
  m_input_box->installEventFilter(self);
  m_input_box->connect_submit_signal(
    std::bind_front(&ComboBox::on_submit, self));
  m_data->m_input_connection =
    m_input_box->get_current()->connect_update_signal(
      std::bind_front(&ComboBox::on_input, self));
  m_data->m_highlight_connection =
    m_input_box->get_highlight()->connect_update_signal(
      std::bind_front(&ComboBox::on_highlight, self));
  m_data->m_list_view =
    new ListView(std::static_pointer_cast<AnyListModel>(m_data->m_matches),
      std::move(m_item_builder));
  m_data->m_list_view->setFocusPolicy(Qt::NoFocus);
  m_data->m_drop_down_list = new DropDownList(*m_data->m_list_view, *self);
  m_data->m_drop_down_list->setFocusPolicy(Qt::NoFocus);
  auto panel = m_data->m_drop_down_list->window();
  panel->setFocusPolicy(Qt::NoFocus);
  panel->setWindowFlags(Qt::Popup | (panel->windowFlags() & ~Qt::Tool));
  panel->installEventFilter(self);
  m_data->m_drop_down_list->installEventFilter(self);
  self->update_focus_proxy();
  m_data->m_drop_down_current_connection = m_data->m_drop_down_list->
    get_list_view().get_current()->connect_update_signal(
      std::bind_front(&ComboBox::on_drop_down_current, self));
  m_data->m_drop_down_list->get_list_view().connect_submit_signal(
    std::bind_front(&ComboBox::on_drop_down_submit, self));
  m_data->m_focus_observer.connect_state_signal(
    std::bind_front(&ComboBox::on_focus, self));
  m_data->m_key_observer.connect_filtered_key_press_signal(
    std::bind_front(&ComboBox::on_input_key_press, self));
}

void ComboBox::update_completion() {
  if(m_data->m_matches->get_size() != 0) {
    auto& highlight = *m_input_box->get_highlight();
    auto& query = any_cast<QString>(m_input_box->get_current()->get());
    if(highlight.get().m_end != query.size()) {
      m_data->m_prefix.clear();
      m_data->m_completion.clear();
      return;
    }
    auto top_match = to_text(m_data->m_matches->get(0));
    if(!top_match.toLower().startsWith(query.toLower())) {
      m_data->m_prefix = query;
      m_data->m_completion.clear();
      return;
    }
    auto prefix = top_match.mid(0, query.size()).toLower();
    auto completion = top_match.mid(query.size());
    if(m_data->m_last_completion.size() < query.size()) {
      m_data->m_last_completion = query;
      auto selection_start = query.size();
      {
        auto blocker = shared_connection_block(m_data->m_input_connection);
        m_input_box->get_current()->set(query + completion);
      }
      m_data->m_has_autocomplete_selection = false;
      highlight.set({selection_start + completion.size(), selection_start});
      m_data->m_has_autocomplete_selection = true;
    } else {
      m_data->m_last_completion = query;
    }
    m_data->m_prefix = std::move(prefix);
    m_data->m_completion = std::move(completion);
  } else {
    m_data->m_last_completion.clear();
    m_data->m_prefix = any_cast<QString>(m_input_box->get_current()->get());
    m_data->m_completion.clear();
  }
}

void ComboBox::revert_to(const QString& query, bool autocomplete) {
  auto blocker = shared_connection_block(m_data->m_input_connection);
  m_input_box->get_current()->set(query);
  m_data->m_has_autocomplete_selection = false;
  m_data->m_last_completion.clear();
  m_data->m_prefix = query;
  m_data->m_completion.clear();
  if(autocomplete) {
    update_completion();
  }
}

void ComboBox::revert_current() {
  auto& list_view = m_data->m_drop_down_list->get_list_view();
  list_view.get_current()->set(none);
  clear(*list_view.get_selection());
  if(m_data->m_user_query) {
    revert_to(*m_data->m_user_query, true);
  }
}

void ComboBox::submit(const QString& query, bool is_passive) {
  auto value = m_query_model->parse(query);
  if(!value.has_value()) {
    return;
  }
  if(is_passive && to_text(value) == to_text(m_data->m_submission)) {
    return;
  }
  if(!m_data->m_completion.isEmpty()) {
    auto blocker = shared_connection_block(m_data->m_input_connection);
    m_input_box->get_current()->set(query);
    m_data->m_has_autocomplete_selection = false;
    auto current_blocker =
      shared_connection_block(m_data->m_current_connection);
    m_current->set(value);
  }
  m_data->m_last_completion = query;
  m_data->m_prefix = query;
  m_data->m_completion.clear();
  m_data->m_submission = value;
  m_data->m_submission_text = query;
  m_input_box->get_highlight()->set(Highlight(query.size()));
  m_data->m_drop_down_list->hide();
  m_data->m_query_result = m_query_model->submit(query).then(
    std::bind_front(
      &ComboBox::on_query, this, ++m_data->m_completion_tag, false));
  m_data->m_submit_signal(value);
}

void ComboBox::on_current(const std::any& current) {
  auto input = any_cast<QString>(m_input_box->get_current()->get());
  if(!is_equal(current, m_query_model->parse(input))) {
    auto text = to_text(current);
    if(input != text) {
      m_input_box->get_current()->set(text);
    }
  }
}

void ComboBox::on_input(const AnyRef& current) {
  if(is_read_only()) {
    return;
  }
  auto& query = any_cast<QString>(current);
  if(!m_data->m_last_completion.toLower().startsWith(query.toLower())) {
    m_data->m_last_completion.clear();
  }
  m_data->m_user_query = query;
  m_data->m_has_autocomplete_selection = false;
  if(query.isEmpty()) {
    on_query(++m_data->m_completion_tag, true, std::vector<std::any>());
  } else {
    m_data->m_query_result = m_query_model->submit(query).then(std::bind_front(
      &ComboBox::on_query, this, ++m_data->m_completion_tag, true));
    auto value = m_query_model->parse(query);
    if(value.has_value()) {
      auto blocker = std::array{
        shared_connection_block(m_data->m_input_connection),
        shared_connection_block(m_data->m_current_connection)};
      m_current->set(value);
    }
  }
}

void ComboBox::on_highlight(const Highlight& highlight) {
  if(!m_data->m_has_autocomplete_selection) {
    return;
  }
  m_data->m_has_autocomplete_selection = false;
  auto& query = any_cast<QString>(m_input_box->get_current()->get());
  auto value = m_query_model->parse(query);
  if(!value.has_value()) {
    return;
  }
  m_data->m_prefix = query;
  m_data->m_completion.clear();
  auto current_blocker = shared_connection_block(m_data->m_current_connection);
  m_current->set(value);
}

void ComboBox::on_submit(const AnyRef& query) {
  if(find_focus_state(*m_input_box) == FocusObserver::State::NONE) {
    return;
  }
  m_data->m_user_query = any_cast<QString>(query);
  submit(*m_data->m_user_query);
}

void ComboBox::on_query(
    std::uint32_t tag, bool show, Expect<std::vector<std::any>>&& result) {
  if(m_data->m_completion_tag != tag) {
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
    auto blocker =
      shared_connection_block(m_data->m_drop_down_current_connection);
    while(m_data->m_matches->get_size() != 0) {
      m_data->m_matches->remove(m_data->m_matches->get_size() - 1);
    }
    for(auto& item : selection) {
      m_data->m_matches->push(item);
      auto list_item = m_data->m_list_view->get_list_item(
        m_data->m_list_view->get_list()->get_size() - 1);
      list_item->setFocusPolicy(Qt::NoFocus);
      if(list_item->is_mounted()) {
        list_item->layout()->itemAt(0)->widget()->setFocusPolicy(Qt::NoFocus);
      }
    }
  }
  update_completion();
  if(show) {
    if(selection.empty()) {
      m_data->m_drop_down_list->hide();
    } else if(m_data->m_focus_observer.get_state() !=
        FocusObserver::State::NONE && !m_data->m_drop_down_list->isVisible()) {
      m_data->m_drop_down_list->show();
    }
  }
}

void ComboBox::on_drop_down_current(optional<int> index) {
  if(index) {
    auto value =
      m_data->m_drop_down_list->get_list_view().get_list()->get(*index);
    auto text = to_text(value);
    {
      auto input_blocker = shared_connection_block(m_data->m_input_connection);
      auto highlight_blocker =
        shared_connection_block(m_data->m_highlight_connection);
      m_input_box->get_current()->set(text);
    }
    m_data->m_last_completion = text;
    m_data->m_completion.clear();
    m_data->m_prefix.clear();
    m_data->m_has_autocomplete_selection = false;
    auto current_blocker =
      shared_connection_block(m_data->m_current_connection);
    m_current->set(value);
  }
}

void ComboBox::on_drop_down_submit(const std::any& submission) {
  auto text = to_text(submission);
  {
    auto input_blocker = shared_connection_block(m_data->m_input_connection);
    auto highlight_blocker =
      shared_connection_block(m_data->m_highlight_connection);
    m_input_box->get_current()->set(text);
    m_data->m_has_autocomplete_selection = false;
    m_input_box->get_highlight()->set(Highlight(text.size()));
  }
  m_data->m_submission = submission;
  m_data->m_submission_text = text;
  m_data->m_drop_down_list->hide();
  m_data->m_submit_signal(submission);
}

void ComboBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && m_data) {
    m_data->m_drop_down_list->hide();
    if(m_data->m_input_focus_proxy) {
      auto focus_out_event = QFocusEvent(QEvent::FocusOut);
      QCoreApplication::sendEvent(
        m_data->m_input_focus_proxy, &focus_out_event);
    }
    submit(any_cast<QString>(m_input_box->get_current()->get()), true);
  }
}

bool ComboBox::on_input_key_press(QWidget& target, QKeyEvent& event) {
  if(is_read_only()) {
    return false;
  } else if(event.key() == Qt::Key_Escape) {
    if(m_data->m_drop_down_list->isVisible()) {
      m_data->m_drop_down_list->hide();
      revert_current();
    } else if(any_cast<QString>(m_input_box->get_current()->get()) !=
        m_data->m_submission_text &&
        m_query_model->parse(m_data->m_submission_text).has_value()) {
      revert_to(m_data->m_submission_text, false);
    } else {
      event.ignore();
    }
    return true;
  }
  return false;
}

LocalComboBoxQueryModel::LocalComboBoxQueryModel()
  : m_values(QChar()) {}

void LocalComboBoxQueryModel::add(const std::any& value) {
  add(to_text(value).toLower(), value);
}

void LocalComboBoxQueryModel::add(const QString& id, const std::any& value) {
  m_values[id.toLower().data()] = value;
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
