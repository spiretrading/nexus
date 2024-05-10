#include "Spire/KeyBindings/TagNameBox.hpp"
#include "Spire/KeyBindings/OrderFieldAdditionalTag.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

struct TagNameBox::TagNameQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit TagNameQueryModel(std::shared_ptr<QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(value.has_value()) {
      return std::any_cast<std::shared_ptr<AdditionalTag>&>(value)->get_name();
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([] (auto&& source_result) {
        auto matches = [&] {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            return std::vector<std::any>();
          }
        }();
        auto result = std::vector<std::any>();
        auto names = std::unordered_set<QString>();
        for(auto& value : matches) {
          auto& name =
            std::any_cast<std::shared_ptr<AdditionalTag>&>(value)->get_name();
          if(names.insert(name).second) {
            result.push_back(name);
          }
        }
        return result;
    });
  }
};

TagNameBox::TagNameBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : TagNameBox(std::move(query_model),
      std::make_shared<LocalValueModel<std::shared_ptr<AdditionalTag>>>(
        std::make_shared<OrderFieldAdditionalTag>(Nexus::Tag(),
          OrderFieldInfoTip::Model())), parent) {}

TagNameBox::TagNameBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> current,
    QWidget* parent)
    : QWidget(parent),
      m_tags(std::make_shared<TagNameQueryModel>(std::move(query_model))),
      m_current(std::move(current)) {
  auto combo_box_current = make_transform_value_model(m_current,
    [=] (const std::shared_ptr<AdditionalTag>& current) {
      return std::any(current->get_name());
    },
    [=] (const std::any& current) {
      return std::any_cast<std::shared_ptr<AdditionalTag>>(
        get_query_model()->parse(std::any_cast<QString>(current)));
    });
  m_combo_box =
    new ComboBox(m_tags, combo_box_current, ListView::default_view_builder);
  m_combo_box->connect_submit_signal([=] (const auto& submission) {
    m_submission = std::any_cast<std::shared_ptr<AdditionalTag>>(
      get_query_model()->parse(std::any_cast<QString>(submission)));
    m_submit_signal(m_submission);
  });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
  m_connection = m_current->connect_update_signal(
    std::bind_front(&TagNameBox::on_current, this));
}

const std::shared_ptr<ComboBox::QueryModel>&
    TagNameBox::get_query_model() const {
  return m_tags->m_source;
}

const std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>>&
    TagNameBox::get_current() const {
  return m_current;
}

const std::shared_ptr<AdditionalTag>& TagNameBox::get_submission() const {
  return m_submission;
}

void TagNameBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool TagNameBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void TagNameBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
  if(is_read_only) {
    if(auto order_field_tag =
        std::dynamic_pointer_cast<OrderFieldAdditionalTag>(m_current->get());
        order_field_tag && !order_field_tag->get_name().isEmpty()) {
      m_info_tip = new OrderFieldInfoTip(order_field_tag->get_model(), this);
    }
  } else if(m_info_tip) {
    m_info_tip->window()->deleteLater();
  }
}

connection TagNameBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void TagNameBox::on_current(const std::shared_ptr<AdditionalTag>& current) {
  if(is_read_only()) {
    if(auto tag = std::dynamic_pointer_cast<OrderFieldAdditionalTag>(current);
        tag && !tag->get_name().isEmpty()) {
      if(m_info_tip) {
        m_info_tip->window()->deleteLater();
      }
      m_info_tip = new OrderFieldInfoTip(tag->get_model(), this);
    }
  }
}
