#ifndef SPIRE_TASKKEYBINDINGSDIALOG_HPP
#define SPIRE_TASKKEYBINDINGSDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

class QKeySequence;
class QTableWidgetItem;
class Ui_TaskKeyBindingsDialog;

namespace Spire {

  /*! \class TaskKeyBindingsDialog
      \brief Displays the user's Task key bindings.
   */
  class TaskKeyBindingsDialog : public QDialog {
    public:

      //! Constructs a TaskKeyBindingsDialogDialog.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TaskKeyBindingsDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      virtual ~TaskKeyBindingsDialog();

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      std::unique_ptr<Ui_TaskKeyBindingsDialog> m_ui;
      UserProfile* m_userProfile;
      KeyBindings m_keyBindings;
      std::vector<Nexus::Venue> m_venues;
      Nexus::Venue m_currentVenue;
      QKeySequence m_currentKeyBinding;
      int m_currentIndex;

      QKeySequence GetCurrentKeyBinding() const;
      void PopulateComboBox();
      void Commit();
      void OnVenueChanged(int index);
      void OnKeyChanged(int index);
      void OnTaskNameChanged(const QString& text);
      void OnOkButton();
      void OnApply();
  };
}

#endif
