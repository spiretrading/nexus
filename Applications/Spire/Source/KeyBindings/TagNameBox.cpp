#include "Spire/KeyBindings/TagNameBox.hpp"
#include "Spire/KeyBindings/OrderFieldAdditionalTag.hpp"
#include "Spire/Spire/ListIndexValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct AdditionalTagNameListModel : ListModel<QString> {
    std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> m_source;
    ListModelTransactionLog<QString> m_transaction;
    scoped_connection m_connection;

    explicit AdditionalTagNameListModel(
      std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> source)
      : m_source(std::move(source)),
        m_connection(m_source->connect_operation_signal(
          std::bind_front(&AdditionalTagNameListModel::on_operation, this))) {}

    int get_size() const override {
      return m_source->get_size();
    }

    const QString& get(int index) const override {
      return m_source->get(index)->get_name();
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void transact(const std::function<void ()>& transaction) override {
      m_transaction.transact(transaction);
    }

    void on_operation(
        const ListModel<std::shared_ptr<AdditionalTag>>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<std::shared_ptr<AdditionalTag>>::AddOperation&
            operation) {
          m_transaction.push(AddOperation(operation.m_index,
            operation.get_value()->get_name()));
        },
        [&] (const ListModel<std::shared_ptr<AdditionalTag>>::MoveOperation&
            operation) {
          m_transaction.push(MoveOperation(operation.m_source,
            operation.m_destination));
        },
        [&] (const ListModel<std::shared_ptr<AdditionalTag>>::RemoveOperation&
            operation) {
          m_transaction.push(RemoveOperation(operation.m_index,
            operation.get_value()->get_name()));
        },
        [&] (const ListModel<std::shared_ptr<AdditionalTag>>::UpdateOperation&
            operation) {
          m_transaction.push(UpdateOperation(operation.m_index,
            operation.get_previous()->get_name(),
            operation.get_value()->get_name()));
        });
    }
  };

  struct AdditionalTagNameValueModel : ValueModel<QString> {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> m_list;
    std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> m_source;
    scoped_connection m_connection;

    AdditionalTagNameValueModel(
      std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> list,
      std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> source)
      : m_list(std::move(list)),
        m_source(std::move(source)),
        m_connection(m_source->connect_update_signal(
          std::bind_front(&AdditionalTagNameValueModel::on_update, this))) {}

    QValidator::State get_state() const override {
      return m_source->get_state();
    }

    const QString& get() const override {
      return m_source->get()->get_name();
    }

    QValidator::State test(const QString& value) const override {
      if(auto i = find_name(value); i != m_list->end()) {
        return QValidator::Acceptable;
      }
      return QValidator::Invalid;
    }

    QValidator::State set(const QString& value) override {
      if(auto i = find_name(value); i != m_list->end()) {
        return m_source->set(*i);
      }
      return QValidator::Invalid;
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_update_signal.connect(slot);
    }

    auto find_name(const QString& name) const {
      return std::find_if(m_list->begin(), m_list->end(),
        [&] (const std::shared_ptr<AdditionalTag>& tag) {
          return name == tag->get_name();
        });
    }

    void on_update(const std::shared_ptr<AdditionalTag>& tag) {
      m_update_signal(tag->get_name());
    }
  };
}

TagNameBox::TagNameBox(
  std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> tags,
  QWidget* parent)
  : TagNameBox(std::move(tags),
      std::make_shared<LocalValueModel<std::shared_ptr<AdditionalTag>>>(
        std::make_shared<OrderFieldAdditionalTag>(Nexus::Tag(),
          OrderFieldInfoTip::Model())), parent) {}

TagNameBox::TagNameBox(
    std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> tags,
    std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> current,
    QWidget* parent)
    : QWidget(parent),
      m_tags(std::move(tags)),
      m_current(std::move(current)) {
  auto name_list_model = std::make_shared<AdditionalTagNameListModel>(m_tags);
  m_drop_down_box = new DropDownBox(name_list_model,
    std::make_shared<ListIndexValueModel<QString>>(name_list_model,
      std::make_shared<AdditionalTagNameValueModel>(m_tags, m_current)),
    ListView::default_view_builder);
  enclose(*this, *m_drop_down_box);
  proxy_style(*this, *m_drop_down_box);
  setFocusProxy(m_drop_down_box);
  m_connection = m_current->connect_update_signal(
    std::bind_front(&TagNameBox::on_current, this));
}

const std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>>&
    TagNameBox::get_tags() const {
  return m_tags;
}

const std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>>&
    TagNameBox::get_current() const {
  return m_current;
}

bool TagNameBox::is_read_only() const {
  return m_drop_down_box->is_read_only();
}

void TagNameBox::set_read_only(bool is_read_only) {
  m_drop_down_box->set_read_only(is_read_only);
  if(is_read_only) {
    create_info_tip(m_current->get());
  } else if(m_info_tip) {
    m_info_tip->window()->deleteLater();
  }
}

connection TagNameBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_drop_down_box->connect_submit_signal([=] (const auto&) {
    slot(m_current->get());
  });
}

void TagNameBox::create_info_tip(
    const std::shared_ptr<AdditionalTag>& current) {
  if(auto tag = std::dynamic_pointer_cast<OrderFieldAdditionalTag>(current);
      tag && !tag->get_name().isEmpty()) {
    if(m_info_tip) {
      m_info_tip->window()->deleteLater();
    }
    m_info_tip = new OrderFieldInfoTip(tag->get_model(), this);
  }
}

void TagNameBox::on_current(const std::shared_ptr<AdditionalTag>& current) {
  if(is_read_only()) {
    create_info_tip(current);
  }
}
