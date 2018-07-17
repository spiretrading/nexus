#include "spire/book_view/book_view_highlight_properties_widget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/check_box.hpp"

using namespace spire;

book_view_highlight_properties_widget::book_view_highlight_properties_widget(
    const book_view_properties& properties, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(10), scale_width(8),
    scale_height(20));
  auto markets_layout = new QVBoxLayout();
  markets_layout->setContentsMargins({});
  markets_layout->setSpacing(0);
  auto markets_label = new QLabel(tr("Markets"), this);
  markets_layout->addWidget(markets_label, 14);
  markets_layout->addStretch(10);
  auto markets_list_widget = new QListWidget(this);
  markets_layout->addWidget(markets_list_widget, 222);
  layout->addLayout(markets_layout, 140);
  layout->addStretch(18);
  auto market_highlight_layout = new QVBoxLayout();
  market_highlight_layout->setContentsMargins({});
  market_highlight_layout->addStretch(24);
  auto highlight_none_check_box = new check_box(tr("Highlight None"), this);
  market_highlight_layout->addWidget(highlight_none_check_box, 16);
  layout->addLayout(market_highlight_layout, 130);
  layout->addStretch(18);
  auto vertical_rule = new QWidget(this);
  vertical_rule->setStyleSheet("background-color: #C8C8C8;");
  layout->addWidget(vertical_rule, 1);
  layout->addStretch(18);
  auto orders_layout = new QVBoxLayout();
  orders_layout->setContentsMargins({});
  auto orders_label = new QLabel(tr("Orders"), this);
  orders_layout->addWidget(orders_label, 14);
  layout->addLayout(orders_layout, 151);
}

void book_view_highlight_properties_widget::apply(
    book_view_properties& properties) const {
}
