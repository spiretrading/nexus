#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVBoxLayout>
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/TimeAndSales/EmptyTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesDialog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ExportModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/SecurityWidget.hpp"
#include "Spire/Ui/TransitionWidget.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesWindow::TimeAndSalesWindow(const TimeAndSalesProperties& properties,
    Ref<SecurityInputModel> input_model,
    QWidget* parent)
    : Window(parent),
      m_table(nullptr) {
  setMinimumSize(scale(180, 200));
  resize_body(scale(180, 410));
  setStyleSheet("background-color: #FFFFFF;");
  setWindowTitle(tr("Time and Sales"));
  set_svg_icon(":/Icons/time-sale-black.svg");
  setWindowIcon(QIcon(":/Icons/time-sales-icon-256x256.png"));
  m_body = new QWidget(this);
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
  Window::layout()->addWidget(m_body);
}

void TimeAndSalesWindow::set_model(std::shared_ptr<TimeAndSalesModel> model) {
  m_volume_label->setText(tr("Volume:"));
  if(m_model.is_initialized()) {
    setWindowTitle(
      m_item_delegate->displayText(QVariant::fromValue(model->get_security()),
      QLocale()) + tr(" - Time and Sales"));
    m_security_widget->set_widget(m_table);
  }
  m_volume_connection = model->connect_volume_signal(
    [=] (const Quantity& v) { on_volume(v); });
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
    auto context_menu_shadow = new DropShadow(true, true, &context_menu);
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
  dialog.connect_apply_signal([&] {
    set_properties(dialog.get_properties()); });
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
