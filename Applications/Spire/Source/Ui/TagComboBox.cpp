#include "Spire/Ui/TagComboBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TagBox.hpp"

using namespace boost::signals2;
using namespace Spire;

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : TagComboBox(std::move(query_model), &ListView::default_view_builder,
      parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  ViewBuilder view_builder, QWidget* parent)
  : TagComboBox(std::move(query_model),
      std::make_shared<ArrayListModel<std::any>>(),
      std::make_shared<LocalValueModel<std::any>>(),
      std::move(view_builder), parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<AnyListModel> list, std::shared_ptr<CurrentModel> current,
    ViewBuilder view_builder, QWidget* parent)
    : QWidget(parent),
      m_tag_box(new TagBox(std::move(list),
        std::make_shared<LocalTextModel>())),
      m_combo_box(new ComboBox(std::move(query_model), std::move(current),
        new AnyInputBox(*m_tag_box), std::move(view_builder))) {
  m_combo_box->connect_submit_signal(
    std::bind_front(&TagComboBox::on_submit, this));
  m_tag_box->get_list()->connect_operation_signal(
    std::bind_front(&TagComboBox::on_operation, this));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    TagComboBox::get_query_model() const {
  return m_combo_box->get_query_model();
}

const std::shared_ptr<AnyListModel>& TagComboBox::get_list() const {
  return m_tag_box->get_list();
}

const std::shared_ptr<TagComboBox::CurrentModel>&
    TagComboBox::get_current() const {
  return m_combo_box->get_current();
}

void TagComboBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool TagComboBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void TagComboBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection TagComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void TagComboBox::on_submit(const std::any& submission) {
  auto& list = get_list();
  auto is_found = [&] {
    auto text = displayTextAny(submission);
    for(auto i = 0; i < list->get_size(); ++i) {
      if(displayTextAny(list->get(i)) == text) {
        return true;
      }
    }
    return false;
  }();
  if(!is_found) {
    list->push(submission);
  }
}

void TagComboBox::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      m_tag_box->setFocus();
      m_submit_signal(get_list());
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      m_submit_signal(get_list());
    });
}
