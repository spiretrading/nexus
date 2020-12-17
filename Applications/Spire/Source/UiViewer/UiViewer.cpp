#include "Spire/UiViewer/UiViewer.hpp"
#include "Spire/Spire/Dimensions.hpp"

QLabel* Spire::make_parameters_label(QWidget* parent) {
  auto label = new QLabel(QObject::tr("Parameters"), parent);
  label->setFixedHeight(scale_height(80));
  label->setStyleSheet(QString(R"(
    QLabel {
      background-color: transparent;
      border-bottom: 1px solid black;
      font-family: Roboto;
      font-size: %1px;
      margin-bottom: %2px;
    })").arg(scale_height(14)).arg(scale_height(12)));
  label->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  return label;
}
