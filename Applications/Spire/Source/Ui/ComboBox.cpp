#include "Spire/Ui/ComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model, QWidget* parent)
  : ComboBox(std::move(query_model), &ListView::default_view_builder, parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
  ViewBuilder view_builder, QWidget* parent)
  : ComboBox(std::move(query_model),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::move(view_builder), parent) {}

ComboBox::ComboBox(std::shared_ptr<QueryModel> query_model,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_query_model(std::move(query_model)),
      m_current(std::move(current)),
      m_selection(std::move(selection)),
      m_view_builder(std::move(view_builder)),
      m_is_read_only(false) {
  m_input_box = new TextBox();
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(m_input_box);
}

const std::shared_ptr<ComboBox::QueryModel>& ComboBox::get_query_model() const {
  return m_query_model;
}

const std::shared_ptr<ComboBox::CurrentModel>& ComboBox::get_current() const {
  return m_current;
}

const std::shared_ptr<ComboBox::SelectionModel>&
    ComboBox::get_selection() const {
  return m_selection;
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

void LocalComboBoxQueryModel::add(const std::any& value) {
  add(displayTextAny(value), value);
}

void LocalComboBoxQueryModel::add(const QString& id, const std::any& value) {}

QtPromise<std::vector<std::any>> LocalComboBoxQueryModel::submit(
    const QString& query) {
  return {};
}
