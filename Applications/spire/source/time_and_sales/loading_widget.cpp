#include "spire/time_and_sales/loading_widget.hpp"
#include <QMovie>
#include <QTimer>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace spire;

loading_widget::loading_widget(QWidget* parent)
    : QWidget(parent) {
  setStyleSheet("background-color: #FFFFFF;");
  auto layout = new QVBoxLayout(this);
  m_logo_widget = new QLabel(this);
  auto logo = new QMovie(":/icons/pre-loader.gif", QByteArray(), this);
  logo->setScaledSize(scale(32, 32));
  m_logo_widget->setMovie(logo);
  m_logo_widget->setStyleSheet(
    QString("padding-top: %1px;").arg(scale_height(50)));
  m_logo_widget->setAlignment(Qt::AlignHCenter);
  layout->addWidget(m_logo_widget);
}

void loading_widget::hideEvent(QHideEvent* event) {
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void loading_widget::showEvent(QShowEvent* event) {
  QTimer::singleShot(2000, [=] { m_logo_widget->movie()->start(); });
}
