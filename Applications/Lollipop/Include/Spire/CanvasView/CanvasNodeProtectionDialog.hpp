#ifndef SPIRE_CANVASNODEPROTECTIONDIALOG_HPP
#define SPIRE_CANVASNODEPROTECTIONDIALOG_HPP
#include <QDialog>
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

class QAbstractButton;
class Ui_CanvasNodeProtectionDialog;

namespace Spire {

  /*! \class CanvasNodeProtectionDialog
      \brief Edits a CanvasNode's protection attributes.
   */
  class CanvasNodeProtectionDialog : public QDialog {
    public:

      //! Constructs a CanvasNodeProtectionDialog.
      /*!
        \param node The CanvasNode to edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CanvasNodeProtectionDialog(const CanvasNode& node,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~CanvasNodeProtectionDialog();

      //! Returns the edited CanvasNode.
      std::unique_ptr<CanvasNode> GetNode();

    private:
      std::unique_ptr<Ui_CanvasNodeProtectionDialog> m_ui;
      std::unique_ptr<CanvasNode> m_node;
      CanvasNodeBuilder m_builder;

      void OnButtonClicked(QAbstractButton* button);
  };
}

#endif
