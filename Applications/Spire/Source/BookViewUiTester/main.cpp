#include <QApplication>
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Ui/MoneyBox.hpp"

using namespace Nexus;
using namespace Spire;

template<typename U>
MoneyBox* make_money_box(
    std::shared_ptr<SecurityTechnicalsModel> technicals, U field) {
  return new MoneyBox(
    std::make_shared<OptionalScalarValueModelDecorator<Money>>(
      std::make_shared<ScalarValueModelDecorator<Money>>(
        make_field_value_model(technicals, field))));
}

template<typename U>
QuantityBox* make_quantity_box(
    std::shared_ptr<SecurityTechnicalsModel> technicals, U field) {
  return new QuantityBox(
    std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
      std::make_shared<ScalarValueModelDecorator<Quantity>>(
        make_field_value_model(technicals, field))));
}

struct TechnicalsTestWindow : QWidget {
  TechnicalsTestWindow(
      std::shared_ptr<SecurityTechnicalsModel> technicals,
      std::shared_ptr<QuantityModel> default_bid_quantity,
      std::shared_ptr<QuantityModel> default_ask_quantity,
      QWidget* parent = nullptr)
      : QWidget(parent) {
    auto layout = make_grid_layout(this);
    layout->setContentsMargins(
      {scale_width(5), scale_height(5), scale_width(5), scale_height(5)});
    layout->setHorizontalSpacing(scale_width(30));
    layout->setVerticalSpacing(scale_height(5));
    layout->addWidget(make_label(tr("High:")), 0, 0);
    auto high_box = make_money_box(technicals, &SecurityTechnicals::m_high);
    layout->addWidget(high_box, 0, 1);
    layout->addWidget(make_label(tr("Low:")), 1, 0);
    auto low_box = make_money_box(technicals, &SecurityTechnicals::m_low);
    layout->addWidget(low_box, 1, 1);
    layout->addWidget(make_label(tr("Open:")), 2, 0);
    auto open_box = make_money_box(technicals, &SecurityTechnicals::m_open);
    layout->addWidget(open_box, 2, 1);
    layout->addWidget(make_label(tr("Close:")), 3, 0);
    auto close_box = make_money_box(technicals, &SecurityTechnicals::m_close);
    layout->addWidget(close_box, 3, 1);
    layout->addWidget(make_label(tr("Volume:")), 4, 0);
    auto volume_box =
      make_quantity_box(technicals, &SecurityTechnicals::m_volume);
    layout->addWidget(volume_box, 4, 1);
    layout->addWidget(make_label(tr("Default Bid Quantity:")), 5, 0);
    auto bid_quantity_box = new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        default_bid_quantity));
    layout->addWidget(bid_quantity_box, 5, 1);
    layout->addWidget(make_label(tr("Default Ask Quantity:")), 6, 0);
    auto ask_quantity_box = new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        default_ask_quantity));
    layout->addWidget(ask_quantity_box, 6, 1);
    setFixedSize(scale(350, 250));
  }

  bool eventFilter(QObject* object, QEvent* event) override {
    if(event->type() == QEvent::Close) {
      QApplication::quit();
    }
    return QWidget::eventFilter(object, event);
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("BookView Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto technicals = std::make_shared<LocalSecuirtyTechnicalsModel>(
    SecurityTechnicals{1100, Money(144.4), Money(142.11), Money(144.25),
      Money(144.99)});
  auto default_bid_quantity = std::make_shared<LocalQuantityModel>(100);
  auto default_ask_quantity = std::make_shared<LocalQuantityModel>(100);
  auto tester = TechnicalsTestWindow(technicals, default_bid_quantity,
    default_ask_quantity);
  auto widget = QWidget();
  auto layout = make_vbox_layout(&widget);
  auto panel = new TechnicalsPanel(technicals, default_bid_quantity,
    default_ask_quantity);
  layout->addWidget(panel);
  layout->addStretch(1);
  widget.installEventFilter(&tester);
  widget.show();
  tester.show();
  tester.move(
    tester.pos().x() + widget.frameGeometry().width() + scale_width(100),
    widget.pos().y());
  application.exec();
}
