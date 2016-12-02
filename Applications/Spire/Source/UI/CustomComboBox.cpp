#include <memory>
#include <QWidget>
#include "Spire/UI/CustomComboBox.hpp"

using namespace Spire;
using namespace std;

CustomComboBox::CustomComboBox(QWidget* parent) : QComboBox(parent)
{
  setStyleSheet("background-color:rgb(22,0,46)");
}

CustomComboBox::~CustomComboBox()
{

}