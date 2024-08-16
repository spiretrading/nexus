#ifndef SPIRE_BREADTHFIRSTCANVASNODEITERATOR_HPP
#define SPIRE_BREADTHFIRSTCANVASNODEITERATOR_HPP
#include <deque>
#include <Beam/Collections/View.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class BreadthFirstCanvasNodeIterator
      \brief A CanvasNode iterator traversing in breadth first order.
   */
  class BreadthFirstCanvasNodeIterator : public boost::iterator_facade<
      BreadthFirstCanvasNodeIterator, const CanvasNode,
      boost::iterators::bidirectional_traversal_tag> {
    public:

      //! Constructs a BreadthFirstCanvasNodeIterator.
      BreadthFirstCanvasNodeIterator();

      //! Constructs a BreadthFirstCanvasNodeIterator.
      /*!
        \param node The node to begin the iteration at.
      */
      BreadthFirstCanvasNodeIterator(const CanvasNode& node);

      //! Constructs a BreadthFirstCanvasNodeIterator.
      /*!
        \param root The node to use as the root of the tree.
        \param current The node to use as the current node.
      */
      BreadthFirstCanvasNodeIterator(const CanvasNode& root,
        const CanvasNode& current);

    private:
      friend class boost::iterator_core_access;
      std::deque<const CanvasNode*> m_forwardQueue;
      std::deque<const CanvasNode*> m_backwardQueue;

      void decrement();
      void increment();
      bool equal(const BreadthFirstCanvasNodeIterator& rhs) const;
      void advance(difference_type n);
      const CanvasNode& dereference() const;
  };

  //! Returns a View over a CanvasNode and its children in breadth first order.
  /*!
    \param node The CanvasNode to create the view over.
  */
  Beam::View<const CanvasNode> BreadthFirstView(const CanvasNode& node);
}

#endif
