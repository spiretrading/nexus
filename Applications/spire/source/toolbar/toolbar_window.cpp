#include "spire/toolbar/toolbar_window.hpp"
#include <QAction>
#include <QEvent>
#include <QGraphicsDropshadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QString>
#include <QStyledItemDelegate>
#include <QtSvg/QSvgRenderer>
#include <QVBoxlayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_window::toolbar_window(QWidget* parent)
    : QWidget(parent) {
  setWindowFlag(Qt::FramelessWindowHint);
  setContentsMargins(0, 0, 0, 0);
  setFixedSize(scale(308, 98));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  layout->addLayout(title_bar_layout);
  auto renderer = new QSvgRenderer(QString(":/icons/spire-icon.svg"), this);
  auto spire_icon = QImage(scale(26, 26), QImage::Format_ARGB32);
  spire_icon.fill(QColor(0, 0, 0, 0));
  QPainter p1(&spire_icon);
  auto icon_rect = QRectF(scale_width(8), scale_height(8), scale_width(10),
    scale_height(10));
  renderer->render(&p1, icon_rect);
  auto title_bar_logo = new QLabel(this);
  title_bar_logo->setFixedSize(scale(26, 26));
  title_bar_logo->setPixmap(QPixmap::fromImage(spire_icon));
  title_bar_layout->addWidget(title_bar_logo);
  auto username_label = new QLabel(tr("Spire - Signed in as Username"));
  username_label->setStyleSheet(QString(
    R"(font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  username_label->setFixedSize(scale(218, 26));
  title_bar_layout->addWidget(username_label);
  m_minimize_button = new icon_button(":/icons/minimize-grey.svg",
    ":/icons/minimize-black.svg", scale_width(32), scale_height(26),
    QRectF(scale_width(11), scale_height(12), scale_width(10), scale_height(2)),
    this);
  title_bar_layout->addWidget(m_minimize_button);
  m_close_button = new icon_button(":/icons/close-grey.svg",
    ":/icons/close-red.svg", scale_width(32), scale_height(26),
    QRectF(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  title_bar_layout->addWidget(m_close_button);
  auto input_layout = new QVBoxLayout();
  input_layout->setMargin(0);
  input_layout->setSpacing(0);
  layout->addLayout(input_layout);
  auto combobox_layout = new QHBoxLayout();
  combobox_layout->setContentsMargins(scale_width(8), scale_height(8),
    scale_width(8), scale_height(5));
  combobox_layout->setSpacing(0);
  input_layout->addLayout(combobox_layout);
  m_window_manager_button = new toolbar_menu(tr("Window Manager"), this);
  m_window_manager_button->setFixedSize(scale(138, 26));
  m_window_manager_button->add_item(tr("Minimize All"));
  combobox_layout->addWidget(m_window_manager_button);
  combobox_layout->addStretch(1);
  m_recently_closed_button = new toolbar_menu(tr("Recently Closed"), this);
  m_recently_closed_button->setFixedSize(scale(138, 26));
  m_recently_closed_button->add_item("Book View", ":/icons/bookview-black.svg");
  m_recently_closed_button->add_item("Book View", ":/icons/bookview-black.svg");
  m_recently_closed_button->add_item("Book View", ":/icons/bookview-black.svg");
  m_recently_closed_button->add_item("Book View", ":/icons/bookview-black.svg");
  m_recently_closed_button->add_item("Book View", ":/icons/bookview-black.svg");
  combobox_layout->addWidget(m_recently_closed_button);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(8), scale_height(5),
    scale_width(8), scale_height(8));
  button_layout->setSpacing(scale_width(14));
  input_layout->addLayout(button_layout);
  m_account_button = new icon_button(":/icons/account-light-purple.svg",
    ":/icons/account-purple.svg", scale_width(20), scale_height(20), this);
  m_account_button->setToolTip(tr("Account"));
  button_layout->addWidget(m_account_button);
  m_key_bindings_button = new icon_button(":/icons/key-bindings-light-purple.svg",
    ":/icons/key-bindings-purple.svg", scale_width(20), scale_height(20), this);
  m_key_bindings_button->setToolTip(tr("Key Bindings"));
  button_layout->addWidget(m_key_bindings_button);
  m_canvas_button = new icon_button(":/icons/canvas-light-purple.svg",
    ":/icons/canvas-purple.svg", scale_width(20), scale_height(20), this);
  m_canvas_button->setToolTip(tr("Canvas"));
  button_layout->addWidget(m_canvas_button);
  m_book_view_button = new icon_button(":/icons/bookview-light-purple.svg",
    ":/icons/bookview-purple.svg", scale_width(20), scale_height(20), this);
  m_book_view_button->setToolTip(tr("Bookview"));
  button_layout->addWidget(m_book_view_button);
  m_time_sale_button = new icon_button(":/icons/time-sale-light-purple.svg",
    ":/icons/time-sale-purple.svg", scale_width(20), scale_height(20), this);
  m_time_sale_button->setToolTip(tr("Time and Sale"));
  button_layout->addWidget(m_time_sale_button);
  m_chart_button = new icon_button(":/icons/chart-light-purple.svg",
    ":/icons/chart-purple.svg", scale_width(20), scale_height(20), this);
  m_chart_button->setToolTip(tr("Chart"));
  button_layout->addWidget(m_chart_button);
  m_dashboard_button = new icon_button(":/icons/dashboard-light-purple.svg",
    ":/icons/dashboard-purple.svg", scale_width(20), scale_height(20), this);
  m_dashboard_button->setToolTip(tr("Dashboard"));
  button_layout->addWidget(m_dashboard_button);
  m_order_imbalances_button = new icon_button(
    ":/icons/order-imbalances-light-purple.svg",
    ":/icons/order-imbalances-purple.svg", scale_width(20), scale_height(20),
    this);
  m_order_imbalances_button->setToolTip(tr("Order Imbalances"));
  button_layout->addWidget(m_order_imbalances_button);
  m_blotter_button = new icon_button(":/icons/blotter-light-purple.svg",
    ":/icons/blotter-purple.svg", scale_width(20), scale_height(20), this);
  m_blotter_button->setToolTip(tr("Blotter"));
  button_layout->addWidget(m_blotter_button);
}

connection toolbar_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

connection toolbar_window::connect_reopen_signal(
    const reopen_signal::slot_type& slot) const {
  return m_reopen_signal.connect(slot);
}

void toolbar_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool toolbar_window::eventFilter(QObject* watched, QEvent* event) {
  return QWidget::eventFilter(watched, event);
}
