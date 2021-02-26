#ifndef SPIRE_STYLES_BOX_STYLES_HPP
#define SPIRE_STYLES_BOX_STYLES_HPP
#include <QColor>
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
  using BackgroundColor = BasicProperty<QColor, struct BackgroundColorTag>;
  using BorderColor = BasicProperty<QColor, struct BorderColorTag>;
  using BorderTopSize = BasicProperty<int, struct BorderTopSizeTag>;
  using BorderRightSize = BasicProperty<int, struct BorderRightSizeTag>;
  using BorderBottomSize = BasicProperty<int, struct BorderBottomSizeTag>;
  using BorderLeftSize = BasicProperty<int, struct BorderLeftSizeTag>;
  using PaddingTopSize = BasicProperty<int, struct PaddingTopSizeTag>;
  using PaddingRightSize = BasicProperty<int, struct PaddingRightSizeTag>;
  using PaddingBottomSize = BasicProperty<int, struct PaddingBottomSizeTag>;
  using PaddingLeftSize = BasicProperty<int, struct PaddingLeftSizeTag>;
}

#endif
