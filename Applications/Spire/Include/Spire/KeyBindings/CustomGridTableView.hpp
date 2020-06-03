#ifndef SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#define SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#include <boost/optional.hpp>
#include <QTableView>

namespace Spire {

  //! Represents a table view a cutom grid line and item highlight style.
  class CustomGridTableView : public QTableView {
    public:

      //! Constructs a CustomGridTableView.
      /*
        \param parent The parent widget.
      */
      explicit CustomGridTableView(QWidget* parent = nullptr);

      //! Returns the selected index.
      const QModelIndex& get_selected_index() const;

      //! Sets the selected index.
      /*
        \param index The selected index.
      */
      void set_selected_index(const QModelIndex& index);

      //! Sets the selected index to invalid.
      void reset_selected_index();

    protected:
      void leaveEvent(QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QModelIndex m_selected_index;
      boost::optional<QPoint> m_last_mouse_pos;

      void draw_border(const QModelIndex& index, QPainter* painter) const;
  };
}

#endif
