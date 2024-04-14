#ifndef SPIRE_NEW_BLOTTER_FORM_HPP
#define SPIRE_NEW_BLOTTER_FORM_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a form to make a new blotter. */
  class NewBlotterForm : public QWidget {
    public:

      /**
       * Signals to create a new blotter.
       * @param name The new blotter's name.
       */
      using SubmitSignal = Signal<void (const QString& name)>;

      /**
       * Constructs a NewBlotterForm.
       * @param blotters The list of existing blotters.
       * @param parent The parent widget.
       */
      NewBlotterForm(
        std::shared_ptr<ListModel<BlotterModel*>> blotters, QWidget& parent);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection
        connect_submit_signal(const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      class TextModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ListModel<BlotterModel*>> m_blotters;
      std::shared_ptr<TextModel> m_current_name;
      boost::signals2::scoped_connection m_current_name_connection;
      TextBox* m_name;
      Button* m_create_button;
      OverlayPanel* m_panel;

      void create_name(const QString& name);
      void on_cancel();
      void on_create();
      void on_current_name(const QString& value);
  };
}

#endif
