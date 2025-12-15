#ifndef SPIRE_BOOKVIEWPROPERTIESDIALOG_HPP
#define SPIRE_BOOKVIEWPROPERTIESDIALOG_HPP
#include <QDialog>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/UI/UI.hpp"

class Ui_BookViewPropertiesDialog;

namespace Spire {

  /*! \class BookViewPropertiesDialog
      \brief Displays the properties for a BookViewWindow.
   */
  class BookViewPropertiesDialog : public QDialog {
    public:

      //! Constructs a BookViewPropertiesDialog.
      /*!
        \param userProfile The user's profile.
        \param security The Security to represent.
        \param properties The BookViewProperties to modify.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      BookViewPropertiesDialog(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security, const BookViewProperties& properties,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~BookViewPropertiesDialog();

      //! Returns the BookViewProperties represented by this dialog.
      const BookViewProperties& GetProperties() const;

    private:
      std::unique_ptr<Ui_BookViewPropertiesDialog> m_ui;
      UserProfile* m_userProfile;
      Nexus::Security m_security;
      BookViewProperties m_properties;

      void Redisplay();
      void OnLoadDefault();
      void OnSaveAsDefault();
      void OnResetDefault();
      void OnOk();
      void OnApply();
      void OnApplyToAll();
  };
}

#endif
