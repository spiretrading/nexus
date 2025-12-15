#ifndef SPIRE_REGION_INPUT_DIALOG_HPP
#define SPIRE_REGION_INPUT_DIALOG_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/UI/UserProfile.hpp"

class Ui_ValueListInputDialog;

namespace Spire {

  /** Displays an input editor dialog for a Region. */
  class RegionInputDialog : public QDialog {
    public:

      /**
       * Constructs a RegionInputDialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit RegionInputDialog(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a RegionInputDialog.
       * @param region The Region to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      RegionInputDialog(Nexus::Region region,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      ~RegionInputDialog() override;

      /** Returns the Region. */
      const Nexus::Region& GetRegion() const;

    private:
      std::unique_ptr<Ui_ValueListInputDialog> m_ui;
      UserProfile* m_userProfile;
      Nexus::Region m_region;

      void ActivateRow(int row, QKeyEvent* event);
      void Append(const QVariant& item);
      void OnAccept();
      void OnAddItemAction();
      void OnLoadFileAction();
      void OnDeleteItemAction();
      void OnCellActivated(int row, int column);
  };
}

#endif
