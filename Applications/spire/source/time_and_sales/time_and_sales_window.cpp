#include "spire/time_and_sales/time_and_sales_window.hpp"
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVBoxLayout>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include "spire/time_and_sales/time_and_sales_table_view.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/spire/export_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/security_widget.hpp"
#include "spire/ui/transition_widget.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesWindow::TimeAndSalesWindow(const TimeAndSalesProperties& properties,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : QWidget(parent),
      m_table(nullptr) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(180, 200));
  resize(scale_width(180), scale_height(450));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new Window(m_body);
  setWindowTitle(tr("Time and Sales"));
  window->set_svg_icon(":/icons/time-sale-black.svg",
    ":/icons/time-sale-grey.svg");
  setWindowIcon(QIcon(":icons/time-sales-icon-256x256.png"));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto padding_widget = new QWidget(this);
  padding_widget->setFixedHeight(scale_height(4));
  padding_widget->setStyleSheet("background-color: #F5F5F5;");
  layout->addWidget(padding_widget);
  m_table = new TimeAndSalesTableView(this);
  m_table->hide();
  m_security_widget = new SecurityWidget(input_model,
    SecurityWidget::Theme::LIGHT, this);
  layout->addWidget(m_security_widget);
  m_volume_label = new QLabel(tr("Volume:"), this);
  m_volume_label->setFocusPolicy(Qt::NoFocus);
  m_volume_label->setFixedHeight(scale_height(20));
  m_volume_label->setStyleSheet(QString(R"(
    background-color: #F5F5F5;
    color: #333333;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;
    padding-left: %2px;)").arg(scale_height(10)).arg(scale_width(8)));
  layout->addWidget(m_volume_label);
  set_model(std::make_shared<EmptyTimeAndSalesModel>(Security()));
  set_properties(properties);
  m_item_delegate = new CustomVariantItemDelegate(this);
}

void TimeAndSalesWindow::set_model(std::shared_ptr<TimeAndSalesModel> model) {
  m_volume_label->setText(tr("Volume:"));
  if(m_model.is_initialized()) {
    setWindowTitle(
      m_item_delegate->displayText(QVariant::fromValue(model->get_security()),
      QLocale()) + tr(" - Time and Sales"));
    m_security_widget->set_widget(m_table);
  }
  model->connect_volume_signal([=] (const Quantity& v) { on_volume(v); });
  m_model.emplace(std::move(model), m_properties);
  m_table->set_model(m_model.get_ptr());
}

const TimeAndSalesProperties& TimeAndSalesWindow::get_properties() const {
  return m_properties;
}

void TimeAndSalesWindow::set_properties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
  m_model->set_properties(m_properties);
  if(m_table == nullptr) {
    return;
  }
  m_table->set_properties(m_properties);
}

connection TimeAndSalesWindow::connect_change_security_signal(
    const ChangeSecuritySignal::slot_type& slot) const {
  return m_security_widget->connect_change_security_signal(slot);
}

connection TimeAndSalesWindow::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void TimeAndSalesWindow::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

void TimeAndSalesWindow::contextMenuEvent(QContextMenuEvent* event) {
  auto contents = m_body->layout()->itemAt(1)->widget();
  QRect widget_geometry(contents->mapToGlobal(contents->geometry().topLeft()),
    contents->mapToGlobal(contents->geometry().bottomRight()));
  if(widget_geometry.contains(event->globalPos())) {
    QMenu context_menu(this);
    QAction properties_action(tr("Properties"), &context_menu);
    connect(&properties_action, &QAction::triggered, this,
      &TimeAndSalesWindow::show_properties_dialog);
    context_menu.addAction(&properties_action);
    QAction export_action(tr("Export Table"), &context_menu);
    connect(&export_action, &QAction::triggered, this,
      &TimeAndSalesWindow::export_table);
    export_action.setEnabled(m_table->isVisible());
    context_menu.addAction(&export_action);
    context_menu.setFixedWidth(scale_width(140));
    context_menu.setWindowFlag(Qt::NoDropShadowWindowHint);
    DropShadow context_menu_shadow(true, true, &context_menu);
    context_menu.setStyleSheet(QString(R"(
      QMenu {
        background-color: #FFFFFF;
        border: %1px solid #A0A0A0 %2px solid #A0A0A0;
        color: #000000;
        font-family: Roboto;
        font-size: %3px;
        padding: %4px 0px;
      }

      QMenu::item {
        padding: %5px 0px %5px %6px;
      }

      QMenu::item:disabled,
      QMenu::item:disabled:selected,
      QMenu::item:disabled:hover {
        background-color: #FFFFFF;
        color: #C8C8C8;
      }

      QMenu::item:selected, QMenu::item:hover {
        background-color: #8D78EC;
        color: #FFFFFF;
      })")
      .arg(scale_height(1)).arg(scale_width(1))
      .arg(scale_height(12)).arg(scale_height(5))
      .arg(scale_height(3)).arg(scale_width(8)));
    context_menu.exec(event->globalPos());
  }
}

void TimeAndSalesWindow::keyPressEvent(QKeyEvent* event) {
  QApplication::sendEvent(m_security_widget, event);
}

void TimeAndSalesWindow::export_table() {
  m_security_widget->show_overlay_widget();
  auto filepath = QFileDialog::getSaveFileName(this, tr("Export As"),
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
    tr("/time_and_sales"), "CSV (*.csv)");
  if(!filepath.isNull()) {
    export_model_as_csv(m_model.get(), std::ofstream(filepath.toStdString()));
  }
  m_security_widget->hide_overlay_widget();
}

void TimeAndSalesWindow::show_properties_dialog() {
  TimeAndSalesPropertiesDialog dialog(m_properties, this);
  dialog.connect_apply_signal([=] (auto p) { set_properties(p); });
  m_security_widget->show_overlay_widget();
  if(dialog.exec() == QDialog::Accepted) {
    set_properties(dialog.get_properties());
  }
  m_security_widget->hide_overlay_widget();
}

void TimeAndSalesWindow::on_volume(const Quantity& volume) {
  m_volume_label->setText(tr("Volume:").append(" ") +
    m_item_delegate->displayText(QVariant::fromValue(volume), QLocale()));
}
