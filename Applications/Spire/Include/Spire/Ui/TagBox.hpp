#ifndef SPIRE_TAG_BOX_HPP
#define SPIRE_TAG_BOX_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class TagBox : public QWidget {
    public:

      using DeleteSignal = Signal<void (int index)>;

      using SubmitSignal = Signal<void (const QString& text)>;

      explicit TagBox(std::shared_ptr<ListModel> model,
        QWidget* parent = nullptr);

      const std::shared_ptr<ListModel>& get_model() const;

    private:
      ListView* m_list_view;
      TextBox* m_text_box;
  };
}

#endif
