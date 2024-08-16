#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasNodeNotVisibleException::CanvasNodeNotVisibleException()
    : runtime_error("Canvas node has no visible nodes to display.") {}

CanvasNodeNotVisibleException::CanvasNodeNotVisibleException(
    const string& message)
    : runtime_error(message) {}

CanvasNodeNotVisibleException::~CanvasNodeNotVisibleException() throw() {}
