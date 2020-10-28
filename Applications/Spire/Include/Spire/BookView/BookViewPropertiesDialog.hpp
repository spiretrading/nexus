#ifndef SPIRE_BOOK_VIEW_PROPERTIES_DIALOG_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_DIALOG_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Ui/Dialog.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/TabWidget.hpp"

namespace Spire {

  //! Displays the properties for a book view window.
  class BookViewPropertiesDialog : public Dialog {
    public:

      //! Signals that properties should be applied to the parent window.
      using ApplySignal = Signal<void ()>;

      //! Signals that properties should be applied to all windows.
      using ApplyAllSignal = Signal<void ()>;

      //! Signals that properties should be saved as the default.
      using SaveDefaultSignal = Signal<void ()>;

      //! Constructs a book view properties dialog.
      /*!
        \param properties The properties to modify.
        \param security The security used for modifying interactions.
        \param parent The parent widget.
      */
      BookViewPropertiesDialog(const BookViewProperties& properties,
        const Nexus::Security& security, QWidget* parent = nullptr);

      //! Returns the properties represented by this dialog.
      BookViewProperties get_properties() const;

      //! Connects a slot to the apply signal.
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

      //! Connects a slot to the apply all signal.
      boost::signals2::connection connect_apply_all_signal(
        const ApplyAllSignal::slot_type& slot) const;

      //! Connects a slot to the save default signal.
      boost::signals2::connection connect_save_default_signal(
        const SaveDefaultSignal::slot_type& slot) const;

    private:
      mutable ApplySignal m_apply_signal;
      mutable ApplyAllSignal m_apply_all_signal;
      mutable SaveDefaultSignal m_save_default_signal;
      TabWidget* m_tab_widget;
      BookViewLevelPropertiesWidget* m_levels_tab_widget;
      BookViewHighlightPropertiesWidget* m_highlights_tab_widget;
  };
}

#endif
