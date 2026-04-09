#include "Spire/Ui/TickerDialog.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

TickerDialog::TickerDialog(
  std::shared_ptr<TickerInfoQueryModel> tickers, QWidget* parent)
  : TickerDialog(
      std::move(tickers), std::make_shared<LocalTickerModel>(), parent) {}

TickerDialog::TickerDialog(std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<TickerModel> current, QWidget* parent)
    : QWidget(parent) {
  auto header = make_label(tr("Ticker"));
  update_style(*header, [] (auto& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly() && Disabled()).
      set(text_style(font, QColor(0x808080))).
      set(PaddingBottom(scale_height(8)));
  });
  m_ticker_box = new TickerBox(std::move(tickers));
  update_style(*m_ticker_box, [] (auto& style) {
    style.get(Any() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(vertical_padding(scale_height(7)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(header);
  layout->addWidget(m_ticker_box);
  setFocusProxy(m_ticker_box);
  setFixedWidth(scale_width(180));
  m_panel = new OverlayPanel(*this, *parent);
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->installEventFilter(this);
  m_input_box = find_focus_proxy(*m_ticker_box);
  m_input_box->installEventFilter(this);
}

const std::shared_ptr<TickerInfoQueryModel>& TickerDialog::get_tickers() const {
  return m_ticker_box->get_tickers();
}

const std::shared_ptr<TickerModel>& TickerDialog::get_current() const {
  return m_ticker_box->get_current();
}

connection TickerDialog::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_ticker_box->connect_submit_signal(slot);
}

bool TickerDialog::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_panel && event->type() == QEvent::Close) {
    close();
  } else if(watched == m_input_box && event->type() == QEvent::KeyPress) {
    auto& key_event = static_cast<QKeyEvent&>(*event);
    if(key_event.key() == Qt::Key_Escape) {
      QApplication::sendEvent(m_panel, event);
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool TickerDialog::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    auto parent_size = m_panel->parentWidget()->size();
    auto x = (parent_size.width() - m_panel->width()) / 2;
    auto y = (parent_size.height() - m_panel->height()) / 2;
    m_panel->move(m_panel->parentWidget()->mapToGlobal({x, y}));
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_ticker_box->get_current()->set(Ticker());
    setFocus();
    m_panel->close();
  }
  return QWidget::event(event);
}
