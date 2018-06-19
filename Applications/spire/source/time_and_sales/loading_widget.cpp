#include "spire/time_and_sales/loading_widget.hpp"
#include <QMovie>
#include "spire/spire/dimensions.hpp"

using namespace spire;

loading_widget::loading_widget(const QString& file_path,
    QWidget* parent = nullptr)
    : QLabel(parent) {
  setAlignment(Qt::AlignCenter);
  auto logo = new QMovie(file_path, QByteArray(), this);
  logo->setScaledSize(scale(16, 16));
  setMovie(logo);
  logo->start();
}
