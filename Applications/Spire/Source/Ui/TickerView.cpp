#include "Spire/Ui/TickerView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TickerDialog.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

TickerView::TickerView(std::shared_ptr<TickerInfoQueryModel> tickers,
  QWidget& body, QWidget* parent)
  : TickerView(std::move(tickers), std::make_shared<LocalTickerModel>(), body,
      parent) {}

TickerView::TickerView(std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<CurrentModel> current, QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_ticker_dialog(std::move(tickers), this),
      m_current(std::move(current)),
      m_body(&body) {
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
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&TickerView::on_current, this));
  m_ticker_dialog.connect_submit_signal(
    std::bind_front(&TickerView::on_submit, this));
  if(m_current->get()) {
    on_current(m_current->get());
  }
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

TickerView::State TickerView::save_state() const {
  return State(m_tickers);
}

void TickerView::restore(const State& state) {
  m_tickers = state.m_tickers;
  if(auto top = m_tickers.get_top()) {
    if(*top != m_current->get()) {
      auto blocker = shared_connection_block(m_current_connection);
      m_current->set(*top);
      if(m_layers->currentWidget() != m_body) {
        m_layers->setCurrentWidget(m_body);
      }
    }
  }
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
  } else if(event->key() == Qt::Key_PageUp) {
    if(auto ticker = m_tickers.rotate_top()) {
      m_current->set(*ticker);
    }
  } else if(event->key() == Qt::Key_PageDown) {
    if(auto ticker = m_tickers.rotate_bottom()) {
      m_current->set(*ticker);
    }
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TickerView::on_current(const Ticker& ticker) {
  if(!ticker) {
    return;
  }
  m_ticker_dialog.hide();
  m_tickers.add(ticker);
  if(m_layers->currentWidget() != m_body) {
    m_layers->setCurrentWidget(m_body);
  }
}

void TickerView::on_submit(const Ticker& ticker) {
  m_current->set(ticker);
}
