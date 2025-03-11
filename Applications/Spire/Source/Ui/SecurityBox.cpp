#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityListItem.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct SecurityBox::SecurityQueryModel : QueryModel<Security> {
  std::shared_ptr<SecurityInfoQueryModel> m_source;

  explicit SecurityQueryModel(std::shared_ptr<SecurityInfoQueryModel> source)
    : m_source(std::move(source)) {}

  optional<Security> parse(const QString& query) override {
    if(auto value = m_source->parse(query)) {
      return value->m_security;
    }
    return none;
  }

  QtPromise<std::vector<Security>> submit(const QString& query) override {
    return m_source->submit(query).then([] (std::vector<SecurityInfo> matches) {
      auto securities = std::unordered_set<Security>();
      for(auto& match : matches) {
        securities.insert(match.m_security);
      }
      return std::vector<Security>(securities.begin(), securities.end());
    });
  }
};

SecurityBox::SecurityBox(std::shared_ptr<SecurityInfoQueryModel> securities,
  QWidget* parent)
  : SecurityBox(
      std::move(securities), std::make_shared<LocalSecurityModel>(), parent) {}

SecurityBox::SecurityBox(std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_securities(
        std::make_shared<SecurityQueryModel>(std::move(securities))),
      m_current(std::move(current)) {
  m_combo_box = new ComboBox(m_securities, m_current,
    [=] (const auto& list, auto index) {
      return new SecurityListItem(
        *m_securities->m_source->parse(to_text(list->get(index))));
    });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<SecurityInfoQueryModel>&
    SecurityBox::get_securities() const {
  return m_securities->m_source;
}

const std::shared_ptr<SecurityBox::CurrentModel>&
    SecurityBox::get_current() const {
  return m_current;
}

const Security& SecurityBox::get_submission() const {
  return m_combo_box->get_submission();
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
  return m_combo_box->connect_submit_signal(slot);
}
