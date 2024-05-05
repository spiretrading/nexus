#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityListItem.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct SecurityBox::SecurityQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;

  explicit SecurityQueryModel(std::shared_ptr<QueryModel> source)
    : m_source(std::move(source)) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(value.has_value()) {
      return std::any_cast<SecurityInfo&>(value).m_security;
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
        auto matches = [&] {
          try {
            return source_result.Get();
          } catch(const std::exception&) {
            return std::vector<std::any>();
          }
        }();
        auto result = std::vector<std::any>();
        auto securities = std::unordered_set<Security>();
        for(auto& value : matches) {
          auto& security = std::any_cast<SecurityInfo&>(value).m_security;
          if(securities.insert(security).second) {
            result.push_back(security);
          }
        }
        return result;
    });
  }
};

SecurityBox::SecurityBox(std::shared_ptr<ComboBox::QueryModel> securities,
  QWidget* parent)
  : SecurityBox(
      std::move(securities), std::make_shared<LocalSecurityModel>(), parent) {}

SecurityBox::SecurityBox(std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_securities(
        std::make_shared<SecurityQueryModel>(std::move(securities))),
      m_current(std::move(current)) {
  auto combo_box_current = make_transform_value_model(m_current,
    [] (const Security& current) {
      return std::any(current);
    },
    [] (const std::any& current) {
      return std::any_cast<Security>(current);
    });
  m_combo_box = new ComboBox(m_securities, combo_box_current,
    [=] (const auto& list, auto index) {
      return new SecurityListItem(std::any_cast<SecurityInfo&&>(
        m_securities->m_source->parse(to_text(list->get(index)))));
    });
  m_combo_box->connect_submit_signal([=] (const auto& submission) {
    m_submit_signal(std::any_cast<const Security&>(submission));
  });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    SecurityBox::get_securities() const {
  return m_securities->m_source;
}

const std::shared_ptr<SecurityBox::CurrentModel>&
    SecurityBox::get_current() const {
  return m_current;
}

const Security& SecurityBox::get_submission() const {
  auto& submission = m_combo_box->get_submission();
  if(submission.has_value()) {
    return std::any_cast<const Security&>(submission);
  }
  static auto security = Security();
  return security;
}

void SecurityBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool SecurityBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void SecurityBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection SecurityBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
