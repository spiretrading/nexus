#ifndef SPIRE_RESPONSIVE_LABEL_HPP
#define SPIRE_RESPONSIVE_LABEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a TextBox used as a label whose current value is the longest value
   * among a list of values that will fit within it's body without overflowing.
   */
  class ResponsiveLabel : public QWidget {
    public:

      /**
       * Constructs a ResponsiveLabel.
       * @param list The list of labels to display.
       * @param parent The parent widget.
       */
      explicit ResponsiveLabel(std::shared_ptr<ListModel<QString>> labels,
        QWidget* parent = nullptr);

      /** Returns the list of labels to display. */
      const std::shared_ptr<ListModel<QString>>& get_labels() const;

      /** Returns the model of the current's displayed value. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;
  };
}

#endif
