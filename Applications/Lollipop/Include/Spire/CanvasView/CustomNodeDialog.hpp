#ifndef SPIRE_CUSTOMNODEDIALOG_HPP
#define SPIRE_CUSTOMNODEDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include <QMetaObject>
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/Spire/Spire.hpp"

class QAbstractButton;
class QComboBox;
class QTableWidgetItem;
class Ui_CustomNodeDialog;

namespace Spire {

  /*! \class CustomNodeDialog
      \brief Edits a CustomNode.
   */
  class CustomNodeDialog : public QDialog {
    public:

      //! Constructs a CustomNodeDialog for a new CustomNode.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CustomNodeDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a CustomNodeDialog based on an existing CustomNode.
      /*!
        \param userProfile The user's profile.
        \param customNode The CustomNode to edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CustomNodeDialog(const CustomNode& customNode,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~CustomNodeDialog();

      //! Returns the new CustomNode.
      std::unique_ptr<CustomNode> GetNode();

    private:
      friend class Ui_CustomNodeDialog;
      friend class CustomNodeChildrenTable;
      std::unique_ptr<Ui_CustomNodeDialog> m_ui;
      UserProfile* m_userProfile;
      std::vector<CustomNode::Child> m_children;
      std::vector<std::unique_ptr<CanvasNode>> m_nodes;
      QMetaObject::Connection m_itemChangedConnection;

      void Setup(const CustomNode& customNode);
      void AddItem(const CustomNode::Child& child, int row);
      void DeleteItem(QTableWidgetItem* item);
      void AddNewChildItem();
      void OnButtonClicked(QAbstractButton* button);
      void OnItemChanged(QTableWidgetItem* item);
      void OnItemActivated(QTableWidgetItem* item);
      void OnCurrentItemChanged(QTableWidgetItem* current,
        QTableWidgetItem* previous);
  };
}

#endif
