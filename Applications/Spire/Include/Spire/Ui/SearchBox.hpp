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
       * Constructs a SearchBox.
       * @param parent The parent widget.
       */
      explicit SearchBox(QWidget* parent = nullptr);

      /** Returns the model associated with the TextBox. */
      const std::shared_ptr<TextModel>& get_model() const;

    private:
      TextBox* m_text_box;
      Button* m_delete_button;

      void on_current(const QString& value);
      void on_delete_button();
  };
}

#endif
