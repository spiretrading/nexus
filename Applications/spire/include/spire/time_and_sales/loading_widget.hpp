#ifndef LOADING_WIDGET_HPP
#define LOADING_WIDGET_HPP
#include <QLabel>
#include "spire/time_and_sales/time_and_sales.hpp"

namespace spire {

  class loading_widget : public QLabel{
    public:

      loading_widget(const QString& file_path, QWidget* parent = nullptr)
          : QLabel(parent) {
        setAlignment(Qt::AlignCenter);
        auto logo = new QMovie(file_path, QByteArray(), this);
        logo->setScaledSize(scale(16, 16));
        setMovie(logo);
        logo->start();
      }
  };
}

#endif
