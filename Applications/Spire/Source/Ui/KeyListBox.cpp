#include "Spire/Ui/KeyListBox.hpp"
#include <unordered_set>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/HashQtTypes.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_key_input_box(std::shared_ptr<KeySequenceValueModel> current) {
    auto key_input_box = new KeyInputBox(std::move(current));
    update_style(*key_input_box, [] (auto& style) {
      style.get(Any() || Hover() || Focus() || Disabled()).
        set(BackgroundColor(QColor(Qt::transparent))).
        set(border_size(0)).
        set(padding(0));
    });
    return new AnyInputBox(*key_input_box);
  }

  struct UniqueKeySequenceListModel : KeySequenceListModel {
    std::shared_ptr<KeySequenceListModel> m_source;
    std::unordered_set<QKeySequence> m_exclusion_set;
    ListModelTransactionLog<QKeySequence> m_transaction;
    scoped_connection m_connection;

    explicit UniqueKeySequenceListModel(
        std::shared_ptr<KeySequenceListModel> source)
        : m_source(std::move(source)),
          m_connection(m_source->connect_operation_signal(
            std::bind_front(&UniqueKeySequenceListModel::on_operation, this))) {
      for(auto i = 0; i < m_source->get_size(); ++i) {
        m_exclusion_set.insert(m_source->get(i));
      }
    }

    int get_size() const override {
      return m_source->get_size();
    }

    const QKeySequence& get(int index) const override {
      return m_source->get(index);
    }

    QValidator::State set(int index, const QKeySequence& value) override {
      if(index < 0 || index >= get_size()) {
        return QValidator::State::Invalid;
      }
      if(m_exclusion_set.contains(value) && m_source->get(index) != value) {
        return QValidator::Invalid;
      }
      return m_source->set(index, value);
    }

    QValidator::State insert(const QKeySequence& value, int index) override {
      if(m_exclusion_set.contains(value)) {
        return QValidator::Invalid;
      }
      return m_source->insert(value, index);
    }

    QValidator::State move(int source, int destination) override {
      return m_source->move(source, destination);
    }

    QValidator::State remove(int index) override {
      return m_source->remove(index);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void transact(const std::function<void ()>& transaction) override {
      m_transaction.transact(transaction);
    }

    void on_operation(const Operation& operation) {
      visit(operation,
        [&] (const AddOperation& operation) {
          m_exclusion_set.insert(m_source->get(operation.m_index));
        },
        [&] (const PreRemoveOperation& operation) {
          m_exclusion_set.erase(m_source->get(operation.m_index));
        },
        [&] (const UpdateOperation& operation) {
          m_exclusion_set.erase(operation.get_previous());
          m_exclusion_set.insert(operation.get_value());
        });
      m_transaction.push(operation);
    }
  };
}

KeyListBox::KeyListBox(std::shared_ptr<KeySequenceListModel> keys,
    std::shared_ptr<KeySequenceValueModel> current, QWidget* parent)
    : AnyTagBox(
        std::make_shared<UniqueKeySequenceListModel>(std::move(keys)),
        make_key_input_box(std::move(current)), parent),
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
  auto visibility = get_keys()->get_size() == 0 ?
    Visibility::VISIBLE : Visibility::INVISIBLE;
  update_style(get_input_box(), [visibility] (auto& style) {
    style.get(Any() > is_a<KeyInputBox>() > Prompt()).set(visibility);
  });
}
