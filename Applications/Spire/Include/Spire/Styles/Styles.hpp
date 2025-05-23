#ifndef SPIRE_STYLES_HPP
#define SPIRE_STYLES_HPP

namespace Spire::Styles {
  class AncestorSelector;
  class AndSelector;
  class Any;
  template<typename T, typename G> class BasicProperty;
  class Block;
  class ChildSelector;
  class CombinatorSelector;
  template<typename... T> class CompositeProperty;
  template<typename T> class ConstantExpression;
  template<typename T> class CubicBezierExpression;
  class DescendantSelector;
  class EvaluatedBlock;
  class EvaluatedProperty;
  template<typename T> struct Evaluation;
  template<typename T> class Expression;
  class FlipSelector;
  template<typename I, typename T, typename... A>
    class FunctionDefinitionExpression;
  class IsASelector;
  template<typename T> class LinearExpression;
  class NotSelector;
  class OrSelector;
  class ParentSelector;
  template<typename T> class PeriodicEvaluator;
  class Property;
  class PropertyMatchSelector;
  class PseudoElement;
  template<typename T, typename G> class PseudoElementSelector;
  template<typename T>
  class RevertExpression;
  class Rule;
  class Selector;
  class SelectorRegistry;
  class SelectConnection;
  class SiblingSelector;
  template<typename T, typename G> class StateSelector;
  class StyleSheet;
  class StyleSheetMap;
  class Stylist;
}

#endif
