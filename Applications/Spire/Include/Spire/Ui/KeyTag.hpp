#ifndef SPIRE_KEY_TAG_HPP
#define SPIRE_KEY_TAG_HPP
#include <memory>
#include <QWidget>
#include <boost/signals2/connection.hpp>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Styles the KeyTag when it's displaying a modifier key. */
  using ModifierKeyState = StateSelector<void, struct ModifierKeyStateTag>;

  /** Styles the KeyTag when it's displaying the ESC key. */
  using EscapeKeyState = StateSelector<void, struct EscapeKeyStateTag>;
}

  /** Displays text for the associated key. */
  class KeyTag : public QWidget {
    public:

      /**
       * Constructs a KeyTag using a local model initialized to a
       * Qt::Unknown key.
       * @param parent The parent widget.
       */
      explicit KeyTag(QWidget* parent = nullptr);
  
      explicit KeyTag(Qt::Key current, QWidget* parent = nullptr);

      /**
       * Constructs a KeyTag.
       * @param model The current key's model.
       * @param parent The parent widget.
       */
      explicit KeyTag(std::shared_ptr<KeyModel> model,
        QWidget* parent = nullptr);
  
      /** Returns the model. */
      const std::shared_ptr<KeyModel>& get_model() const;

    private:
      enum class State {
        DEFAULT,
        MODIFIER,
        ESCAPE
      };
      std::shared_ptr<KeyModel> m_model;
      boost::signals2::scoped_connection m_current_connection;
      TextBox* m_text_box;
      State m_state;

      void on_current_key(Qt::Key key);
  };
}

#endif
