#ifndef SPIRE_LAYOUTS_HPP
#define SPIRE_LAYOUTS_HPP
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace Spire {

  /**
   * Returns a new QVBoxLayout with no margins or spacing.
   * @param parent The optional parent to the layout.
   */
  QVBoxLayout* make_vbox_layout(QWidget* parent = nullptr);

  /**
   * Returns a new QHBoxLayout with no margins or spacing.
   * @param parent The optional parent to the layout.
   */
  QHBoxLayout* make_hbox_layout(QWidget* parent = nullptr);

  /**
   * Returns a new QGridLayout with no margins or spacing.
   * @param parent The optional parent to the layout.
   */
  QGridLayout* make_grid_layout(QWidget* parent = nullptr);

  /**
   * Sets the layout of some parent widget to a new BoxLayout with no margins
   * used to wrap a body.
   * @param parent The parent widget to provide the layout to.
   * @param body The widget to enclose within the layout.
   */
  void enclose(QWidget& parent, QWidget& body);

  /**
   * Sets the layout of some parent widget to a new BoxLayout with no margins
   * used to wrap a body.
   * @param parent The parent widget to provide the layout to.
   * @param body The widget to enclose within the layout.
   * @param alignment How to align the <i>body</i> within the <i>parent</i>.
   */
  void enclose(QWidget& parent, QWidget& body, Qt::Alignment alignment);

  /**
   * Clears out the contents of a QLayout, deleting all items and their
   * contents.
   */
  void clear(QLayout& layout);
}

#endif
