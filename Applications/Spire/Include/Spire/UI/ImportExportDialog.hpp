#ifndef SPIRE_IMPORTEXPORTDIALOG_HPP
#define SPIRE_IMPORTEXPORTDIALOG_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

class Ui_ImportExportDialog;

namespace Spire {
namespace UI {

  /*! \class ImportExportDialog
      \brief Allows the user to import or export their settings.
   */
  class ImportExportDialog : public QDialog {
    public:

      //! Constructs the ImportExportDialog.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ImportExportDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ImportExportDialog();

      //! Commits the settings represented by this dialog.
      void CommitSettings();

    private:
      std::unique_ptr<Ui_ImportExportDialog> m_ui;
      UserProfile* m_userProfile;

      void ExportSettings();
      void ImportSettings();
      void SetOpenMode(bool toggled);
      void SetSaveMode(bool toggled);
  };
}
}

#endif
