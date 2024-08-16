#ifndef SPIRE_TASKWIDGET_HPP
#define SPIRE_TASKWIDGET_HPP
#include <memory>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Canvas/Canvas.hpp"

class Ui_TaskWidget;

namespace Spire {

  /*! \class TaskWidget
      \brief Displays a read-only Task.
   */
  class TaskWidget : public QWidget {
    public:

      //! Constructs a TaskWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TaskWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TaskWidget();

      //! Resets the CanvasNode displayed.
      void ResetDisplayedNode();

      //! Sets the node to display.
      /*!
        \param displayedNode The node to display.
      */
      void SetDisplayedNode(const CanvasNode& displayedNode);

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      std::unique_ptr<Ui_TaskWidget> m_ui;
      std::unique_ptr<CanvasNode> m_displayedNode;
  };
}

#endif
