#ifndef SPIRE_LUASCRIPTDIALOG_HPP
#define SPIRE_LUASCRIPTDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include <QMetaObject>
#include "Spire/Canvas/LuaNodes/LuaScriptNode.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

class QAbstractButton;
class QComboBox;
class QTableWidgetItem;
class Ui_LuaScriptDialog;

namespace Spire {

  /*! \class LuaScriptDialog
      \brief Edits a LuaScriptNode.
   */
  class LuaScriptDialog : public QDialog {
    public:

      //! Constructs a LuaScriptDialog for a new LuaScriptNode.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      LuaScriptDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a LuaScriptDialog based on an existing LuaScriptNode.
      /*!
        \param userProfile The user's profile.
        \param node The LuaScriptNode to edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      LuaScriptDialog(const LuaScriptNode& node,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~LuaScriptDialog();

      //! Returns the new LuaScriptNode.
      std::unique_ptr<LuaScriptNode> GetNode();

    private:
      friend class Ui_LuaScriptDialog;
      friend class LuaScriptParametersTable;
      std::unique_ptr<Ui_LuaScriptDialog> m_ui;
      UserProfile* m_userProfile;
      std::vector<LuaScriptNode::Parameter> m_parameters;
      std::vector<std::unique_ptr<CanvasNode>> m_nodes;
      QMetaObject::Connection m_itemChangedConnection;

      void Setup(const LuaScriptNode& node);
      void AddParameter(const LuaScriptNode::Parameter& parameters, int row);
      void DeleteParameter(QTableWidgetItem* item);
      void AddNewParameter();
      void OnButtonClicked(QAbstractButton* button);
      void OnItemChanged(QTableWidgetItem* item);
      void OnItemActivated(QTableWidgetItem* item);
      void OnCurrentItemChanged(QTableWidgetItem* current,
        QTableWidgetItem* previous);
  };
}

#endif
