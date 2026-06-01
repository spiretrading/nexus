#include "Spire/Ui/KeyListBox.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_key_input_box(std::shared_ptr<KeySequenceListModel> keys,
      std::shared_ptr<KeySequenceValueModel> current) {
    auto validated = make_validated_value_model(
      [keys = std::move(keys)] (const QKeySequence& value) {
        if(value.isEmpty()) {
          return QValidator::Acceptable;
        }
        for(auto i = 0; i < keys->get_size(); ++i) {
          if(keys->get(i) == value) {
            return QValidator::Invalid;
          }
        }
        return QValidator::Acceptable;
      }, std::move(current));
    auto key_input_box = new KeyInputBox(std::move(validated));
    update_style(*key_input_box, [] (auto& style) {
      style.get(Any() || Hover() || Focus() || Disabled()).
        set(BackgroundColor(QColor(Qt::transparent))).
        set(border_size(0)).
        set(padding(0));
    });
    return new AnyInputBox(*key_input_box);
  }
}

KeyListBox::KeyListBox(std::shared_ptr<KeySequenceListModel> keys,
    std::shared_ptr<KeySequenceValueModel> current, QWidget* parent)
    : AnyTagBox(keys, make_key_input_box(keys, std::move(current)), parent),
      m_key_submit_connection(get_input_box().connect_submit_signal(
        std::bind_front(&KeyListBox::on_submit, this))),
      m_keys_connection(get_keys()->connect_operation_signal(
        std::bind_front(&KeyListBox::on_keys_operation, this))) {
  update_prompt_visibility();
}

std::shared_ptr<KeySequenceListModel> KeyListBox::get_keys() const {
  return std::static_pointer_cast<KeySequenceListModel>(AnyTagBox::get_tags());
}

std::shared_ptr<KeySequenceValueModel> KeyListBox::get_current() const {
  return std::static_pointer_cast<KeySequenceValueModel>(
    get_input_box().get_current());
}

bool KeyListBox::is_input_empty() const {
  return get_current()->get().isEmpty();
}

void KeyListBox::on_submit(const AnyRef& submission) {
  auto key = any_cast<QKeySequence>(submission);
  if(key.isEmpty()) {
    return;
  }
  get_keys()->push(key);
  get_current()->set(QKeySequence());
}

void KeyListBox::on_keys_operation(
    const KeySequenceListModel::Operation& operation) {
  visit(operation,
    [&] (const KeySequenceListModel::AddOperation&) {
      update_prompt_visibility();
    },
    [&] (const KeySequenceListModel::RemoveOperation&) {
      update_prompt_visibility();
    });
}

void KeyListBox::update_prompt_visibility() {
  auto visibility = [&] {
    if(get_keys()->get_size() == 0) {
      return Visibility::VISIBLE;
    }
    return Visibility::INVISIBLE;
  }();
  update_style(get_input_box(), [=] (auto& style) {
    style.get(Any() > is_a<KeyInputBox>() > Prompt()).set(visibility);
  });
}
