#ifndef SPIRE_KEY_TAG_HPP
#define SPIRE_KEY_TAG_HPP
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class KeyTag : public QWidget {
    public:

      /** Constructs a KeyTag with the Qt::Unknown key. */
      explicit KeyTag(QWidget* parent = nullptr);
  
      explicit KeyTag(std::shared_ptr<KeyModel> model,
        QWidget* parent = nullptr);
  
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
