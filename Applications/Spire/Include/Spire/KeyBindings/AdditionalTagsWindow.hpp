#ifndef SPIRE_ADDITIONAL_TAGS_WINDOW_HPP
#define SPIRE_ADDITIONAL_TAGS_WINDOW_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays a table of additional tags. */
  class AdditionalTagsWindow : public Window {
    public:

      /**
       * Constructs an AdditionalTagsWindow.
       * @param parent The parent widget.
       */
      AdditionalTagsWindow(QWidget* parent = nullptr);

    private:
      std::shared_ptr<TextModel> m_error_message;
  };
}

#endif
