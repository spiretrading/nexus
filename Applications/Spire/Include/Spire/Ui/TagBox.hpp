#ifndef SPIRE_TAG_BOX_HPP
#define SPIRE_TAG_BOX_HPP
#include <QWidget>
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a list of tags within a box. */
  class TagBox : public QWidget {
    public:

      /**
       * Signals that the tag at the index is being deleted.
       * @param index The index of the deleted tag.
       */
      using DeleteSignal = Signal<void (int index)>;

      /**
       * Constructs a TagBox.
       * @param list_model The list model which holds a list of tags.
       * @param current_model The current text's model.
       * @param parent The parent widget.
       */
      TagBox(std::shared_ptr<ListModel> list_model,
        std::shared_ptr<TextModel> current_model, QWidget* parent = nullptr);

      /** Returns the list of tags. */
      const std::shared_ptr<ListModel>& get_list_model() const;

      /** Returns the current text model. */
      const std::shared_ptr<TextModel>& get_current_model() const;

      /** Returns <code>true</code> iff this TagBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the TagBox should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;

    private:
      mutable DeleteSignal m_delete_signal;
      std::shared_ptr<ListModel> m_model;
      ListView* m_list_view;
      TextBox* m_text_box;
      std::vector<Tag*> m_tags;

      QWidget* build_tag(const std::shared_ptr<ListModel>& model, int index);
      void on_operation(const ListModel::Operation& operation);
      void update_uneditable();
  };
}

#endif
