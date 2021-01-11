#ifndef SPIRE_SECURITYINPUTDIALOG_HPP
#define SPIRE_SECURITYINPUTDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include <QTimer>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Utilities/Utilities.hpp"

class QLineEdit;
class QModelIndex;
class QTableView;
class Ui_SecurityInputDialog;

namespace Spire {

  /*! \class SecurityInputDialog
      \brief Prompts the user for a Security.
   */
  class SecurityInputDialog : public QDialog {
    public:

      //! Constructs a SecurityInputDialog.
      /*!
        \param userProfile The user's profile.
        \param initial The Security to initially display to the user.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      SecurityInputDialog(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& initial, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a SecurityInputDialog.
      /*!
        \param userProfile The user's profile.
        \param text The text that triggered this dialog.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      SecurityInputDialog(Beam::Ref<UserProfile> userProfile,
        const std::string& text, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~SecurityInputDialog();

      //! Returns the Security represented by this dialog.
      /*!
        \param supportWildCards Whether wild-cards should be supported.
      */
      Nexus::Security GetSecurity(bool supportWildCards = false) const;

      //! Returns the line edit widget being used by this dialog.
      QLineEdit& GetSymbolInput();

    private:
      std::unique_ptr<Ui_SecurityInputDialog> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<SecurityInfoModel> m_model;
      SecurityInfoCompleter* m_completer;
      QTableView* m_completerPopup;

      void Initialize();
      void AdjustCompleterSize();
      void OnInputEdited(const QString& text);
      void OnRowsAddedRemoved(const QModelIndex& parent, int start, int end);
      void OnDataChanged(const QModelIndex& topLeft,
        const QModelIndex& bottomRight);
  };
}

#endif
