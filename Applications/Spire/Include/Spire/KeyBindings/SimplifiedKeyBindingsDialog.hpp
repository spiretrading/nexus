#ifndef SPIRE_SIMPLIFIEDKEYBINDINGSDIALOG_HPP
#define SPIRE_SIMPLIFIEDKEYBINDINGSDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_SimplifiedKeyBindingsDialog;

namespace Spire {

  /*! \class SimplifiedKeyBindingsDialog
      \brief Displays a simplified key bindings dialog with built-in options.
   */
  class SimplifiedKeyBindingsDialog : public QDialog {
    public:

      //! Constructs a SimplifiedKeyBindingsDialog.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      SimplifiedKeyBindingsDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~SimplifiedKeyBindingsDialog();

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      std::unique_ptr<Ui_SimplifiedKeyBindingsDialog> m_ui;
      std::unordered_map<Nexus::MarketCode,
        std::vector<std::unique_ptr<const CanvasNode>>> m_orderTypes;
      std::unordered_map<int, QKeySequence> m_taskBindings;
      std::unordered_map<int, QKeySequence> m_cancelBindings;
      Nexus::MarketCode m_currentMarket;
      UserProfile* m_userProfile;
      KeyBindings m_keyBindings;

      void PopulateTaskTable();
      void PopulateCancelTable();
      bool FilterTaskBindingsTable(QEvent* event);
      bool FilterCancelBindingsTable(QEvent* event);
      void OnClearTasksButton();
      void OnClearCancelsButton();
      void OnOkButton();
      void OnApplyButton();
      void OnMarketChanged(int index);
  };
}

#endif
