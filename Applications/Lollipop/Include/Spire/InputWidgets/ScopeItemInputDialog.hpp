#ifndef SPIRE_SCOPE_ITEM_INPUT_DIALOG_HPP
#define SPIRE_SCOPE_ITEM_INPUT_DIALOG_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QDialog>
#include <QVariant>
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/Utilities/ScopeCompleter.hpp"
#include "Spire/Utilities/ScopeQueryModel.hpp"

class QLineEdit;
class QTableView;
class Ui_SecurityInputDialog;

namespace Spire {

  /**
   * Prompts the user for a single item within a Scope. An item can be either a
   * CountryCode, Venue, Ticker, or a Scope.
   */
  class ScopeItemInputDialog : public QDialog {
    public:

      /**
       * Constructs a ScopeItemInputDialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit ScopeItemInputDialog(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a ScopeItemInputDialog.
       * @param item The item to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      ScopeItemInputDialog(QVariant item,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a ScopeItemInputDialog.
       * @param text The text that triggered this dialog.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      ScopeItemInputDialog(const std::string& text,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      ~ScopeItemInputDialog() override;

      /** Returns the Item represented by this dialog. */
      QVariant GetItem() const;

      /** Returns the line edit widget being used by this dialog. */
      QLineEdit& GetSymbolInput();

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;

    private:
      std::unique_ptr<Ui_SecurityInputDialog> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<ScopeQueryModel> m_model;
      ScopeCompleter* m_completer;
      QTableView* m_completerPopup;

      void AdjustCompleterSize();
      void OnInputEdited(const QString& text);
      void OnRowsAddedRemoved(const QModelIndex& parent, int start, int end);
      void OnDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);
  };

  /**
   * Pops up a ScopeItemInputDialog.
   * @param item The item to display in the text field.
   * @param userProfile The user's profile.
   * @param parent The parent widget.
   * @param resultSlot The slot to call when the dialog closes.
   */
  void ShowScopeItemInputDialog(
    const boost::variant<std::string, QVariant>& item,
    Beam::Ref<UserProfile> userProfile, QWidget* parent,
    std::function<void (boost::optional<QVariant>)> onResult);
}

#endif
