#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasTypeCompatibilityException::CanvasTypeCompatibilityException()
    : CanvasOperationException("Incompatible types") {}

CanvasTypeCompatibilityException::CanvasTypeCompatibilityException(
    string message)
    : CanvasOperationException(std::move(message)) {}

CanvasTypeCompatibilityException::~CanvasTypeCompatibilityException() {}
