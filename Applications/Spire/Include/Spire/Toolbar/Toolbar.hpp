#ifndef SPIRE_TOOLBAR_HPP
#define SPIRE_TOOLBAR_HPP
#include <memory>
#include <QDialog>
#include <QPoint>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
//#include "Spire/UI/WindowTitleBar.hpp"
class Ui_Toolbar;

namespace Spire {
  class Toolbar : public QDialog {
    public:
      Toolbar(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
      virtual ~Toolbar();
    
    private:
      std::unique_ptr<Ui_Toolbar> m_ui;
      //UI::WindowTitleBar* m_titleBar;

  };
}

#endif
