#include "Spire/Canvas/Operations/CanvasOperationException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasOperationException::CanvasOperationException()
    : runtime_error("Invalid canvas operation performed.") {}

CanvasOperationException::CanvasOperationException(string message)
    : runtime_error(std::move(message)) {}

CanvasOperationException::~CanvasOperationException() throw() {}
