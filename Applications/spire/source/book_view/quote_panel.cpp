#include "spire/book_view/quote_panel.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace Spire;

QuotePanel::QuotePanel(const std::shared_ptr<BookViewModel>& model, Side side,
    QWidget* parent)
    : QWidget(parent) {
  m_model = std::move(model);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_indicator_widget = new QWidget(this);
  m_indicator_widget->setFixedHeight(scale_height(2));
  set_indicator_color("#C8C8C8");
  layout->addWidget(m_indicator_widget);
  auto label_layout = new QHBoxLayout();
  m_price_label = new QLabel(tr("N/A"), this);
  m_price_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12)));
  label_layout->addWidget(m_price_label);
  m_size_label = new QLabel(this);
  m_size_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(10)));
  label_layout->addWidget(m_size_label);
  layout->addLayout(label_layout);
}

void QuotePanel::set_indicator_color(const QColor& color) {
  auto palette = m_indicator_widget->palette();
  palette.setColor(QPalette::Window, color);
  m_indicator_widget->setPalette(palette);
}
