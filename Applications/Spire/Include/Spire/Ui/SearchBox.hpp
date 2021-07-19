#ifndef SPIRE_SEARCH_BOX_HPP
#define SPIRE_SEARCH_BOX_HPP
#include <QWidget>
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a text search input box. */
  class SearchBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QString& submission)>;

      /**
       * Constructs a SearchBox using a LocalTextModel.
       * @param parent The parent widget.
       */
      explicit SearchBox(QWidget* parent = nullptr);

      /**
       * Constructs a SearchBox.
       * @param model The current value's model.
       * @param parent The parent widget.
       */
      explicit SearchBox(std::shared_ptr<TextModel> model,
        QWidget* parent = nullptr);

      /** Returns the model associated with the TextBox. */
      const std::shared_ptr<TextModel>& get_model() const;

      /**
       * Sets the placeholder text.
       * @param text The placeholder text.
       */
      void set_placeholder(const QString& text);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      boost::signals2::scoped_connection m_current_connection;
      TextBox* m_text_box;
      Button* m_delete_button;

      void on_current(const QString& current);
      void on_delete_button();
  };
}

#endif
