#include "Spire/UiViewer/UiViewer.hpp"
#include "Spire/Spire/Dimensions.hpp"

QLabel* Spire::make_parameters_label(QWidget* parent) {
  auto label = new QLabel(QObject::tr("Parameters"), parent);
  label->setFixedHeight(scale_height(80));
  label->setStyleSheet(QString(R"(
    QLabel {
      background-color: transparent;
      border-bottom: 1px solid #E0E0E0;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
      margin-bottom: %2px;
      padding-bottom: %3px;
    })").arg(scale_height(14)).arg(scale_height(12)).arg(scale_height(6)));
  label->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  return label;
}
