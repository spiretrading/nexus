#ifndef SPIRE_WINDOWTITLEBAR_HPP
#define SPIRE_WINDOWTITLEBAR_HPP
#include <memory>
#include <QWidget>
#include <QPushButton>

class QLabel;
class Ui_WindowTitleBar;

namespace Spire {
  class WindowTitleBar : public QWidget{
    public: 
      WindowTitleBar(QWidget* parent = nullptr);
  private:
      std::unique_ptr<Ui_WindowTitleBar> m_ui;
  };
}

#endif