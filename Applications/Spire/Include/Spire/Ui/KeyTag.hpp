#ifndef SPIRE_KEY_TAG_HPP
#define SPIRE_KEY_TAG_HPP
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays text for the associated key. */
  class KeyTag : public QWidget {
    public:

      /**
       * Constructs a KeyTag with the Qt::Unknown key.
       * parent The parent widget.
       */
      explicit KeyTag(QWidget* parent = nullptr);
  
      /**
       * Constructs a KeyTag with a key provided by a model
       * model The current key's model.
       * parent The parent widget.
       */
      explicit KeyTag(std::shared_ptr<KeyModel> model,
        QWidget* parent = nullptr);
  
      /** Returns the model. */
      const std::shared_ptr<KeyModel>& get_model() const;

      QSize sizeHint() const override;

    private:
      std::shared_ptr<KeyModel> m_model;
      boost::signals2::scoped_connection m_current_connection;
      TextBox* m_text_box;

      void on_current_key(Qt::Key key);
  };
}

#endif
