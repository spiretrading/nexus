#ifndef SPIRE_CHROMA_HASH_WIDGET_HPP
#define SPIRE_CHROMA_HASH_WIDGET_HPP
#include <array>
#include <QColor>
#include <QString>
#include <QWidget>

namespace Spire {

  /** Displays chroma hash. */
  class ChromaHashWidget : public QWidget {
    public:

      /** Constructs a chroma hash for an empty text. */
      explicit ChromaHashWidget(QWidget* parent = nullptr);

      /** Sets the text to hash. */
      void set_text(const QString& text);

    private:
      static constexpr auto COMPONENTS = 3;
      std::array<QColor, COMPONENTS> m_colors;
      std::array<QWidget*, COMPONENTS> m_color_widgets;
  };
}

#endif
