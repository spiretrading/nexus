#include "Spire/UI/ReadOnlyCheckBox.hpp"

using namespace Spire;
using namespace Spire::UI;

ReadOnlyCheckBox::ReadOnlyCheckBox(QWidget* parent)
    : QCheckBox(parent) {}

ReadOnlyCheckBox::ReadOnlyCheckBox(const QString& text, QWidget* parent)
    : QCheckBox(text, parent) {}

ReadOnlyCheckBox::~ReadOnlyCheckBox() {}

void ReadOnlyCheckBox::mousePressEvent(QMouseEvent* event) {}

void ReadOnlyCheckBox::mouseReleaseEvent(QMouseEvent* event) {}
