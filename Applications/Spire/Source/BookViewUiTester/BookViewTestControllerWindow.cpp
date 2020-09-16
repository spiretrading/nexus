#include "Spire/BookViewUiTester/BookViewTestControllerWindow.hpp"
#include <QGridLayout>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Spire/Definitions.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Nexus;
using namespace Spire;

BookViewTestControllerWindow::BookViewTestControllerWindow(
    BookViewWindow* window)
    : m_window(window) {
  setFixedSize(scale(250, 300));
  m_window->installEventFilter(this);
  m_window->connect_security_change_signal(
    [=] (const auto& s) { on_security_changed(s); });
  auto layout = new QGridLayout(this);
  auto loading_time_label = new QLabel("Loading Time (ms):", this);
  layout->addWidget(loading_time_label, 0, 0);
  m_load_time_spin_box = new QSpinBox(this);
  m_load_time_spin_box->setMaximum(100000);
  m_load_time_spin_box->setValue(1);
  layout->addWidget(m_load_time_spin_box, 0, 1);
  auto m_model_update_period_label = new QLabel("Model Update Period (ms):",
    this);
  layout->addWidget(m_model_update_period_label, 1, 0);
  m_model_update_period_spin_box = new QSpinBox(this);
  m_model_update_period_spin_box->setMaximum(100000);
  m_model_update_period_spin_box->setValue(1000);
  connect(m_model_update_period_spin_box, &QSpinBox::editingFinished,
    this, &BookViewTestControllerWindow::on_model_period_updated);
  layout->addWidget(m_model_update_period_spin_box, 1, 1);
  auto line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  layout->addWidget(line, 2, 1, 1, 2);
  auto market_combo_box_label = new QLabel("Market:", this);
  layout->addWidget(market_combo_box_label, 3, 0);
  m_market_combo_box = new QComboBox(this);
  for(auto& market : GetDefaultMarketDatabase().GetEntries()) {
    m_market_combo_box->addItem(QString::fromStdString(market.m_displayName));
  }
  layout->addWidget(m_market_combo_box, 3, 1);
  auto mpid_label = new QLabel("MPID:", this);
  layout->addWidget(mpid_label, 4, 0);
  m_mpid_line_edit = new QLineEdit(this);
  layout->addWidget(m_mpid_line_edit, 4, 1);
  auto price_label = new QLabel("Price:", this);
  layout->addWidget(price_label, 5, 0);
  m_price_spin_box = new QDoubleSpinBox(this);
  m_price_spin_box->setMaximum(1000000);
  m_price_spin_box->setValue(50.00);
  layout->addWidget(m_price_spin_box, 5, 1);
  auto quantity_label = new QLabel("Quantity", this);
  layout->addWidget(quantity_label, 6, 0);
  m_quantity_spin_box = new QSpinBox(this);
  m_quantity_spin_box->setMaximum(1000000);
  m_quantity_spin_box->setValue(100);
  layout->addWidget(m_quantity_spin_box, 6, 1);
  auto side_label = new QLabel("Side:", this);
  layout->addWidget(side_label, 7, 0);
  m_side_combo_box = new QComboBox(this);
  m_side_combo_box->addItem("Bid");
  m_side_combo_box->addItem("Ask");
  layout->addWidget(m_side_combo_box, 7, 1);
  m_submit_button = new QPushButton("Submit", this);
  m_submit_button->setStyleSheet("background-color: rgb(125, 255, 125);");
  layout->addWidget(m_submit_button, 8, 1);
  connect(m_submit_button, &QPushButton::clicked, this,
    &BookViewTestControllerWindow::on_submit);
}

bool BookViewTestControllerWindow::eventFilter(QObject* watched,
    QEvent* event) {
  if(event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(watched, event);
}

void BookViewTestControllerWindow::on_security_changed(
    const Security& security) {
  m_model = std::make_shared<RandomBookViewModel>(security,
    Definitions::GetDefaults(),
    boost::posix_time::millisec(m_load_time_spin_box->value()));
  m_model->set_period(boost::posix_time::milliseconds(
    m_model_update_period_spin_box->value()));
  m_window->set_model(m_model);
}

void BookViewTestControllerWindow::on_model_period_updated() {
  if(m_model != nullptr) {
    m_model->set_period(boost::posix_time::milliseconds(
      m_model_update_period_spin_box->value()));
  }
}

void BookViewTestControllerWindow::on_submit() {
  BookQuote quote;
  quote.m_market = m_market_combo_box->currentText().toStdString();
  quote.m_mpid = m_mpid_line_edit->text().toStdString();
  quote.m_quote.m_price = Money(Quantity(m_price_spin_box->value()));
  quote.m_quote.m_size = m_quantity_spin_box->value();
  if(m_side_combo_box->currentText() == "Bid") {
    quote.m_quote.m_side = Side::BID;
  } else {
    quote.m_quote.m_side = Side::ASK;
  }
  m_model->publish(quote);
}
