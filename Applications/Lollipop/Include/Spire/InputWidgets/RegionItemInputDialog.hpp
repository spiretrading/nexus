#ifndef SPIRE_REGION_ITEM_INPUT_DIALOG_HPP
#define SPIRE_REGION_ITEM_INPUT_DIALOG_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QDialog>
#include <QVariant>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/Utilities/RegionCompleter.hpp"
#include "Spire/Utilities/RegionQueryModel.hpp"

class QLineEdit;
class QTableView;
class Ui_SecurityInputDialog;

namespace Spire {

  /**
   * Prompts the user for a single item within a Region. An item can be either a
   * CountryCode, Venue, Security, or a Region.
   */
  class RegionItemInputDialog : public QDialog {
    public:

      /**
       * Constructs a RegionItemInputDialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit RegionItemInputDialog(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a RegionItemInputDialog.
       * @param item The item to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      RegionItemInputDialog(QVariant item,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a RegionInputDialog.
       * @param text The text that triggered this dialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      RegionItemInputDialog(const std::string& text,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      ~RegionItemInputDialog() override;

      /** Returns the Item represented by this dialog. */
      QVariant GetItem() const;

      /** Returns the line edit widget being used by this dialog. */
      QLineEdit& GetSymbolInput();

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;

    private:
      std::unique_ptr<Ui_SecurityInputDialog> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<RegionQueryModel> m_model;
      RegionCompleter* m_completer;
      QTableView* m_completerPopup;

      void AdjustCompleterSize();
      void OnInputEdited(const QString& text);
      void OnRowsAddedRemoved(const QModelIndex& parent, int start, int end);
      void OnDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);
  };

  /**
   * Pops up a RegionItemInputDialog.
   * @param item The item to display in the text field.
   * @param userProfile The user's profile.
   * @param parent The parent widget.
   * @param resultSlot The slot to call when the dialog closes.
   */
  void ShowRegionItemInputDialog(
    const boost::variant<std::string, QVariant>& item,
    Beam::Ref<UserProfile> userProfile, QWidget* parent,
    std::function<void (boost::optional<QVariant> item)> onResult);
}

#endif
