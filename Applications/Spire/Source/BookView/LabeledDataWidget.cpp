#include "Spire/BookView/LabeledDataWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

LabeledDataWidget::LabeledDataWidget(const QString& label_text,
  QWidget* parent)
  : LabeledDataWidget(label_text, "", parent) {}

LabeledDataWidget::LabeledDataWidget(const QString& label_text,
    const QString& data_text, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_label = new QLabel(label_text, this);
  m_label->setFixedWidth(scale_width(10));
  m_label->setStyleSheet(QString(R"(
    QLabel {
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(10)));
  layout->addWidget(m_label);
  m_data_label = new QLabel(data_text, this);
  m_data_label->setStyleSheet(QString(R"(
    QLabel {
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
    })").arg(scale_height(10)));
  layout->addWidget(m_data_label);
  layout->addStretch(1);
}

void LabeledDataWidget::set_label_text(const QString& text) {
  m_label->setText(text);
}

void LabeledDataWidget::set_data_text(const QString& text) {
  m_data_label->setText(text);
}

void LabeledDataWidget::set_label_width(int width) {
  m_label->setFixedWidth(width);
}
