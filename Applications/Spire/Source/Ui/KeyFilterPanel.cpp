#include "Spire/Ui/KeyFilterPanel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/KeyInputBox.hpp"

using namespace boost::signals2;
using namespace Spire;

struct KeyInputBoxValueModel : ValueModel<QKeySequence> {
  mutable UpdateSignal m_update_signal;
  std::shared_ptr<KeySequenceValueModel> m_source;
  std::shared_ptr<AnyListModel> m_matches;
  std::unordered_set<QString> m_matches_set;
  std::vector<QString> m_matches_list;
  scoped_connection m_source_connection;
  scoped_connection m_matches_connection;

  KeyInputBoxValueModel(std::shared_ptr<KeySequenceValueModel> source,
      std::shared_ptr<AnyListModel> matches)
      : m_source(std::move(source)),
        m_matches(std::move(matches)),
        m_source_connection(m_source->connect_update_signal(m_update_signal)),
        m_matches_connection(m_matches->connect_operation_signal(
          std::bind_front(&KeyInputBoxValueModel::on_operation, this))) {
    for(auto i = 0; i < m_matches->get_size(); ++i) {
      add_match(i);
    }
  }

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const Type& get() const override {
    return m_source->get();
  }

  QValidator::State test(const Type& value) const {
    return m_source->test(value);
  }

  QValidator::State set(const Type& value) {
    if(m_matches_set.contains(value.toString())) {
      return QValidator::State::Invalid;;
    }
    return m_source->set(value);;
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_update_signal.connect(slot);
  }

  void on_operation(const AnyListModel::Operation& operation) {
    visit(operation,
      [&] (const AnyListModel::AddOperation& operation) {
        add_match(operation.m_index);
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        m_matches_set.erase(m_matches_list[operation.m_index]);
        m_matches_list.erase(m_matches_list.begin() + operation.m_index);
      });
  }

  void add_match(int index) {
    auto value = to_text(m_matches->get(index));
    m_matches_set.insert(value);
    m_matches_list.insert(m_matches_list.begin() + index, value);
  }
};

AnyInputBox* key_input_box_builder(
    std::shared_ptr<KeySequenceValueModel> current,
    std::shared_ptr<AnyListModel> matches) {
  auto input_box = new AnyInputBox(*(new KeyInputBox(
    std::make_shared<KeyInputBoxValueModel>(std::move(current), matches))));
  input_box->connect_submit_signal([=] (const auto& submission) {
    auto& sequence = any_cast<QKeySequence>(submission);
    if(!sequence.isEmpty()) {
      input_box->get_current()->set(QKeySequence());
      matches->push(sequence);
    }
  });
  return input_box;
}

KeyFilterPanel* Spire::make_key_filter_panel(QWidget& parent) {
  return make_key_filter_panel(std::make_shared<LocalKeySequenceValueModel>(),
    parent);
}

KeyFilterPanel* Spire::make_key_filter_panel(
    std::shared_ptr<KeySequenceValueModel> current, QWidget& parent) {
  return make_key_filter_panel(std::move(current),
    std::make_shared<ArrayListModel<QKeySequence>>(),
    std::make_shared<LocalValueModel<KeyFilterPanel::Mode>>(
      KeyFilterPanel::Mode::INCLUDE), parent);
}

KeyFilterPanel* Spire::make_key_filter_panel(
  std::shared_ptr<KeySequenceValueModel> current,
  std::shared_ptr<ListModel<QKeySequence>> matches,
  std::shared_ptr<ValueModel<KeyFilterPanel::Mode>> mode, QWidget& parent) {
  return new KeyFilterPanel(
    std::bind_front(key_input_box_builder, std::move(current)),
    std::move(matches), std::move(mode), QObject::tr("Filter by Key"), parent);
}
