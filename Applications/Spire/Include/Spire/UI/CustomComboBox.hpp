#ifndef SPIRE_CUSTOMCOMBOBOX_HPP
#define SPIRE_CUSTOMCOMBOBOX_HPP

#include <memory>
#include <QWidget>
#include <QComboBox>

namespace Spire
{
  class CustomComboBox : public QComboBox {
  public:
    CustomComboBox(QWidget* parent = nullptr);
    virtual ~CustomComboBox();
  private:
    QComboBox* m_CustomComboBox;
  };
}

#endif