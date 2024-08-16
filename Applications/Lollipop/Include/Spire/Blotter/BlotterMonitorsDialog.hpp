#ifndef SPIRE_BLOTTER_MONITORS_DIALOG_HPP
#define SPIRE_BLOTTER_MONITORS_DIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_BlotterMonitorsDialog;

namespace Spire {

  /** Displays the monitors installed for a Blotter. */
  class BlotterMonitorsDialog : public QDialog {
    public:

      /**
       * Constructs a BlotterMonitorsDialog.
       * @param userProfile The user's profile.
       * @param properties The BlotterTaskProperties to display.
       * @param model The BlotterModel to apply the changes to.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      BlotterMonitorsDialog(Beam::Ref<UserProfile> userProfile,
        const BlotterTaskProperties& properties,
        Beam::Ref<BlotterModel> model, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~BlotterMonitorsDialog();

      /** Returns the BlotterTaskProperties represented by this dialog. */
      const BlotterTaskProperties& GetProperties() const;

    private:
      std::unique_ptr<Ui_BlotterMonitorsDialog> m_ui;
      UserProfile* m_userProfile;
      BlotterTaskProperties m_propeties;
      BlotterModel* m_model;
      int m_currentRow;

      void Commit();
      void OnMonitorItemDataChanged(int row, int column);
      void OnMonitorItemSelectionChanged(int currentRow, int currentColumn,
        int previousRow, int previousColumn);
      void OnLoadDefault();
      void OnSaveAsDefault();
      void OnResetDefault();
      void OnAddMonitor();
      void OnDeleteMonitor();
      void OnAccept();
      void OnApply();
      void OnApplyToAll();
  };
}

#endif
