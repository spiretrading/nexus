#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TickerListItem.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

struct TickerBox::TickerQueryModel : QueryModel<Ticker> {
  std::shared_ptr<TickerInfoQueryModel> m_source;

  explicit TickerQueryModel(std::shared_ptr<TickerInfoQueryModel> source)
    : m_source(std::move(source)) {}

  optional<Ticker> parse(const QString& query) override {
    if(auto value = m_source->parse(query.toUpper())) {
      return value->m_ticker;
    }
    return none;
  }

  QtPromise<std::vector<Ticker>> submit(const QString& query) override {
    return m_source->submit(query.toUpper()).then(
      [] (std::vector<TickerInfo> matches) {
        auto tickers = std::unordered_set<Ticker>();
        for(auto& match : matches) {
          tickers.insert(match.m_ticker);
        }
        return std::vector<Ticker>(tickers.begin(), tickers.end());
      });
  }
};

TickerBox::TickerBox(
  std::shared_ptr<TickerInfoQueryModel> tickers, QWidget* parent)
  : TickerBox(
      std::move(tickers), std::make_shared<LocalTickerModel>(), parent) {}

TickerBox::TickerBox(std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_tickers(std::make_shared<TickerQueryModel>(std::move(tickers))),
      m_current(std::move(current)) {
  m_combo_box = new ComboBox(m_tickers, m_current,
    [=] (const auto& list, auto index) {
      return new TickerListItem(
        *m_tickers->m_source->parse(to_text(list->get(index))));
    });
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<TickerInfoQueryModel>& TickerBox::get_tickers() const {
  return m_tickers->m_source;
}

const std::shared_ptr<TickerBox::CurrentModel>& TickerBox::get_current() const {
  return m_current;
}

const Ticker& TickerBox::get_submission() const {
  return m_combo_box->get_submission();
}

void TickerBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool TickerBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void TickerBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection TickerBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_combo_box->connect_submit_signal(slot);
}
