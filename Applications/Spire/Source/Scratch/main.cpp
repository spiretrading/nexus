#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QDial>
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ScalarWidget.hpp"
#include <QDoubleSpinBox>
#include "Spire/Ui/CustomQtVariants.hpp"

class TestWidget : public QWidget {
  public:

    using ChangeSignal = Signal<void (Nexus::Money)>;

    TestWidget(QWidget* parent = nullptr) : QWidget(parent) {
      auto layout = new QHBoxLayout(this);
      m_button = new QPushButton(this);
      connect(m_button, &QPushButton::clicked, this, &TestWidget::on_button);
      layout->addWidget(m_button);
      on_button();
    }

    void set_value(Nexus::Money value) {
      m_value = value;
      m_button->setText(CustomVariantItemDelegate().displayText(
        QVariant::fromValue(m_value), QLocale()));
    }

    boost::signals2::connection connect_signal(
        const ChangeSignal::slot_type& slot) const {
      return m_signal.connect(slot);
    }

  private:
    mutable ChangeSignal m_signal;
    Nexus::Money m_value;
    QPushButton* m_button;

    void on_button() {
      set_value(Nexus::Money(1.23));
      m_signal(m_value);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto layout = new QVBoxLayout(window);
  auto spire_widget = new TestWidget(window);
  auto spire_scalar_widget = new ScalarWidget(spire_widget,
    &TestWidget::connect_signal, &TestWidget::set_value);
  layout->addWidget(spire_scalar_widget);
  auto spire_button = new QPushButton(
    "4.56", window);
  QObject::connect(spire_button, &QPushButton::clicked,
    [=] { spire_scalar_widget->set_value(Scalar(4.56));
          spire_button->setText("4.56"); });
  spire_scalar_widget->connect_change_signal([=] (auto value) {
    spire_button->setText(QString::number(
      static_cast<double>(static_cast<Nexus::Quantity>(value))));
  });
  layout->addWidget(spire_button);
  window->resize(800, 600);
  window->show();
  application->exec();
}
