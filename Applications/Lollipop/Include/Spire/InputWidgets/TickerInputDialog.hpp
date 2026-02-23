#ifndef SPIRE_TICKER_INPUT_DIALOG_HPP
#define SPIRE_TICKER_INPUT_DIALOG_HPP
#include <functional>
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QDialog>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/Utilities/Utilities.hpp"

class QLineEdit;
class QModelIndex;
class QTableView;
class Ui_SecurityInputDialog;

namespace Spire {

  /** Prompts the user for a Ticker. */
  class TickerInputDialog : public QDialog {
    public:

      /**
       * Constructs a TickerInputDialog.
       * @param userProfile The user's profile.
       * @param initial The Ticker to initially display to the user.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      TickerInputDialog(Beam::Ref<UserProfile> userProfile,
        const Nexus::Ticker& initial, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /**
       * Constructs a TickerInputDialog.
       * @param userProfile The user's profile.
       * @param text The text that triggered this dialog.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      TickerInputDialog(Beam::Ref<UserProfile> userProfile,
        const std::string& text, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      ~TickerInputDialog() override;

      /** Returns the Ticker represented by this dialog. */
      Nexus::Ticker GetTicker() const;

      /** Returns the line edit widget being used by this dialog. */
      QLineEdit& GetSymbolInput();

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;

    private:
      std::unique_ptr<Ui_SecurityInputDialog> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<TickerInfoModel> m_model;
      TickerInfoCompleter* m_completer;
      QTableView* m_completerPopup;

      void Initialize();
      void AdjustCompleterSize();
      void OnInputEdited(const QString& text);
      void OnRowsAddedRemoved(const QModelIndex& parent, int start, int end);
      void OnDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);
  };

  /**
   * Pops up a TickerInputDialog.
   * @param userProfile The user's profile.
   * @param initialValue The initial value to display in the text field.
   * @param parent The parent widget.
   * @param resultSlot The slot to call when the dialog finishes.
   */
  void ShowTickerInputDialog(Beam::Ref<UserProfile> userProfile,
    const boost::variant<std::string, Nexus::Ticker>& initialValue,
    QWidget* parent,
    std::function<void (boost::optional<Nexus::Ticker>)> onResult);
}

#endif
