#ifndef SPIRE_TEXT_BOX_HPP
#define SPIRE_TEXT_BOX_HPP
#include <QLabel>
#include <QLineEdit>
#include <QTimeLine>
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the element's font. */
  using Font = BasicProperty<QFont, struct FontTag>;

  /** Sets the color of the text. */
  using TextColor = BasicProperty<QColor, struct TextColorTag>;

  /** Styles a widget's text. */
  using TextStyle = CompositeProperty<Font, TextColor>;

  /** Sets the text alignment. */
  using TextAlign = BasicProperty<Qt::Alignment, struct TextAlignTag>;

  /** Selects a read-only widget. */
  using ReadOnly = StateSelector<void, struct ReadOnlyTag>;

  /** Selects the placeholder. */
  using Placeholder = StateSelector<void, struct PlaceholderTag>;

  /** Styles a widget's text. */
  TextStyle text_style(QFont font, QColor color);
}

  //! Displays a one-line text box.
  class TextBox : public Styles::StyledWidget {
    public:

      //! Signals that the current text changed.
      using CurrentSignal = Signal<void (const QString& text)>;

      //! Signals that the text is submitted.
      using SubmitSignal = Signal<void (const QString& text)>;

      //! Constructs a TextBox with an empty current value.
      /*!
        \param parent The parent widget.
      */
      explicit TextBox(QWidget* parent = nullptr);

      //! Constructs a TextBox.
      /*!
        \param current The current value.
        \param parent The parent widget.
      */
      explicit TextBox(const QString& current, QWidget* parent = nullptr);

      //! Returns the current value.
      const QString& get_current() const;

      //! Sets the current value.
      void set_current(const QString& value);

      //! Returns the last submitted value.
      const QString& get_submission() const;

      //! Sets the placeholder value.
      void set_placeholder(const QString& value);

      //! Sets whether the box is read-only.
      void set_read_only(bool read_only);

      //! Returns <code>true</code> iff this box is read-only.
      bool is_read_only() const;

      //! Connects a slot to the CurrentSignal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;

      //! Connects a slot to the SubmitSignal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void style_updated() override;
      bool test_selector(const Styles::Selector& selector) const override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;
      void enterEvent(QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      QLineEdit* m_line_edit;
      QLabel* m_placeholder;
      QString m_current;
      QString m_submission;
      QString m_placeholder_text;

      void on_editing_finished();
      void on_text_edited(const QString& text);
      void on_text_changed(const QString& text);
      void elide_text();
      void update_display_text();
      void update_placeholder_text();
  };
}

#endif
