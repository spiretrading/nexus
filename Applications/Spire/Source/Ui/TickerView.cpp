#include "Spire/Ui/TickerView.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TickerDialog.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

TickerView::TickerView(std::shared_ptr<TickerInfoQueryModel> tickers,
  QWidget& body, QWidget* parent)
  : TickerView(
      std::move(tickers), std::make_shared<LocalTickerModel>(), body, parent) {}

TickerView::TickerView(std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<CurrentModel> current, QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_ticker_dialog(std::move(tickers), this),
      m_current(std::move(current)),
      m_body(&body),
      m_current_index(-1) {
  setFocusPolicy(Qt::StrongFocus);
  auto prompt = make_label(tr("Enter a ticker symbol."));
  update_style(*prompt, [] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(TextAlign(Qt::AlignCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_layers = new QStackedWidget();
  m_layers->addWidget(prompt);
  m_layers->addWidget(m_body);
  enclose(*this, *m_layers);
  m_ticker_dialog.connect_submit_signal(
    std::bind_front(&TickerView::on_submit, this));
}

const std::shared_ptr<TickerInfoQueryModel>& TickerView::get_tickers() const {
  return m_ticker_dialog.get_tickers();
}

const std::shared_ptr<TickerView::CurrentModel>&
    TickerView::get_current() const {
  return m_current;
}

const QWidget& TickerView::get_body() const {
  return *m_body;
}

QWidget& TickerView::get_body() {
  return *m_body;
}

void TickerView::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() &
      (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(auto text = event->text();
      text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_')) {
    m_ticker_dialog.show();
    QApplication::sendEvent(find_focus_proxy(m_ticker_dialog), event);
  } else if(event->key() == Qt::Key_PageUp && !m_tickers.empty()) {
    m_current_index =
      (m_tickers.size() + m_current_index - 1) % m_tickers.size();
    m_current->set(m_tickers[m_current_index]);
  } else if(event->key() == Qt::Key_PageDown && !m_tickers.empty()) {
    m_current_index = (m_current_index + 1) % m_tickers.size();
    m_current->set(m_tickers[m_current_index]);
  }
  QWidget::keyPressEvent(event);
}

void TickerView::on_submit(const Ticker& ticker) {
  if(auto i = std::find(m_tickers.begin(), m_tickers.end(), ticker);
      i != m_tickers.end()) {
    if(std::distance(m_tickers.begin(), i) <= m_current_index) {
      --m_current_index;
    }
    m_tickers.erase(i);
  }
  ++m_current_index;
  m_tickers.insert(m_tickers.begin() + m_current_index, ticker);
  m_current->set(ticker);
  m_ticker_dialog.hide();
  if(!m_tickers.empty() && m_layers->currentWidget() != m_body) {
    m_layers->setCurrentWidget(m_body);
  }
}
