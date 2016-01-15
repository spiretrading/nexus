#ifndef SPIRE_CHARTINTERACTIONS_HPP
#define SPIRE_CHARTINTERACTIONS_HPP

namespace Spire {

  /*! \enum ChartInteractionMode
      \brief Lists the ways that the mouse interacts with the ChartView.
    */
  enum class ChartInteractionMode {

    //! No interaction specified.
    NONE,

    //! Pan around the view.
    PAN,

    //! Zoom in or out of the view.
    ZOOM
  };
}

#endif
