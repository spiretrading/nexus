#ifndef SPIRE_DASHBOARDRENDERER_HPP
#define SPIRE_DASHBOARDRENDERER_HPP
#include <functional>
#include <memory>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/signal.hpp>
#include <QPaintDevice>
#include <QRect>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardRowRenderer.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

namespace Spire {

  /*! \struct DashboardRendererSettings
      \brief Stores the internal state of a DashboardRenderer for serialization
             purposes.
   */
  struct DashboardRendererSettings;

  /*! \class DashboardRenderer
      \brief Draws a DashboardModel.
   */
  class DashboardRenderer : private boost::noncopyable {
    public:

      //! Signals that this renderer has a draw operation available.
      using DrawSignal = boost::signals2::signal<void ()>;

      //! Function signature used to build a DashboardRowRenderer.
      /*!
        \param row The DashboardRow to render.
      */
      using DashboardRowRendererBuilder = std::function<
        std::unique_ptr<DashboardRowRenderer> (const DashboardRow& row)>;

      //! Constructs a DashboardRenderer.
      /*!
        \param model The DashboardModel to render.
        \param selectionModel The DashboardSelectionModel used to highlight
               selected rows.
        \param rowRendererBuilder Constructs the DashboardRowRenderers used by
               each row in the <i>model</i>.
        \param userProfile The user's profile.
      */
      DashboardRenderer(Beam::Ref<const DashboardModel> model,
        Beam::Ref<const DashboardSelectionModel> selectionModel,
        const DashboardRowRendererBuilder& rowRendererBuilder,
        Beam::Ref<UserProfile> userProfile);

      //! Returns the model being rendered.
      const DashboardModel& GetModel() const;

      //! Returns the number of rows being rendered.
      int GetSize() const;

      //! Returns the renderer used for the header.
      const DashboardRowRenderer& GetHeaderRenderer() const;

      //! Returns the renderer used for the header.
      DashboardRowRenderer& GetHeaderRenderer();

      //! Returns the maximum row height.
      int GetMaxRowHeight() const;

      //! Sets the maximum row height.
      /*!
        \param height The maximum height in pixels that a row can be.
      */
      void SetMaxRowHeight(int height);

      //! Returns the minimum column width.
      int GetMinimunColumnWidth() const;

      //! Sets the minimum column width.
      /*!
        \param width The minimum width in pixels that a column can be.
      */
      void SetMinimumColumnWidth(int width);

      //! Returns the default width of a column.
      int GetDefaultColumnWidth() const;

      //! Sets the default width of a column.
      /*!
        \param value The default column width.
      */
      void SetDefaultColumnWidth(int value);

      //! Returns the width of a specified column.
      /*!
        \param index The index of the column.
        \return The width of the column at the specified <i>index</i>.  If no
                such column exists then a value of <code>0</code> is returned.
      */
      int GetColumnWidth(int index) const;

      //! Sets the width of a column.
      /*!
        \param index The index of the column.
        \param width The width in pixels of the column.
      */
      void SetColumnWidth(int index, int width);

      //! Returns the model index of a column from its renderer index.
      /*!
        \param index The column's renderer index.
        \return The column's model index.
      */
      int GetColumnModelIndex(int index) const;

      //! Returns the row at a specified index.
      /*!
        \param index The index of the row to get.
        \return The row at the specified <i>index</i>.
      */
      boost::optional<const DashboardRow&> GetRow(int index) const;

      //! Returns the display index of a row.
      /*!
        \param row The row to lookup.
        \return The display index of the <i>row</i>.
      */
      int GetRowDisplayIndex(const DashboardRow& row) const;

      //! Moves a column from one index to another.
      /*!
        \param sourceIndex The index of the column to move.
        \param destinationIndex The index to move the column to.
      */
      void MoveColumn(int sourceIndex, int destinationIndex);

      //! Moves a row from one index to another.
      /*!
        \param sourceIndex The index of the row to move.
        \param destinationIndex The index to move the row to.
      */
      void MoveRow(int sourceIndex, int destinationIndex);

      //! Reorders the rows.
      /*!
        \param indicies The list of rows in the order they should be displayed.
      */
      void ReorderRows(const std::vector<int>& indicies);

      //! Inserts an empty row.
      /*!
        \param index The index of the empty row to add.
      */
      void InsertEmptyRow(int index);

      //! Performs a draw operation.
      /*!
        \param device The device to draw to.
        \param region The region within the <i>device</i> to draw to.
      */
      void Draw(QPaintDevice& device, const QRect& region);

      //! Returns this renderer's current settings.
      DashboardRendererSettings GetSettings() const;

      //! Applies settings to this renderer.
      /*!
        \param settings The settings to apply to this renderer.
      */
      void Apply(const DashboardRendererSettings& settings);

      //! Connects a slot to the DrawSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the DrawSignal.
      */
      boost::signals2::connection ConnectDrawSignal(
        const DrawSignal::slot_function_type& slot) const;

    private:
      friend struct DashboardRendererSettings;
      template<typename, typename> friend struct Beam::Shuttle;
      struct RowEntry {
        const DashboardRow* m_row;
        std::unique_ptr<DashboardRowRenderer> m_renderer;
        boost::signals2::scoped_connection m_drawConnection;
      };
      struct ColumnEntry {
        int m_width;
        int m_index;
      };
      const DashboardModel* m_model;
      const DashboardSelectionModel* m_selectionModel;
      DashboardRowRendererBuilder m_rowRendererBuilder;
      UserProfile* m_userProfile;
      std::unique_ptr<DashboardRow> m_header;
      std::unique_ptr<RowEntry> m_headerEntry;
      std::vector<std::unique_ptr<RowEntry>> m_rows;
      std::vector<ColumnEntry> m_columns;
      int m_defaultColumnWidth;
      int m_minimumColumnWidth;
      int m_maxRowHeight;
      boost::signals2::scoped_connection m_rowAddedConnection;
      boost::signals2::scoped_connection m_rowRemovedConnection;
      mutable DrawSignal m_drawSignal;

      void SetupHeader();
      void DrawBackground(QPainter& painter, QPaintDevice& device,
        const QRect& region, int index);
      void DrawForeground(QPainter& painter, QPaintDevice& device,
        const QRect& region, int index);
      void OnRowAddedSignal(const DashboardRow& row);
      void OnRowRemovedSignal(const DashboardRow& row);
      void OnRowDrawSignal();
  };

  struct DashboardRendererSettings {
    std::vector<DashboardRenderer::ColumnEntry> m_columns;
    std::vector<int> m_emptyRows;
    int m_defaultColumnWidth;
    int m_minimumColumnWidth;
    int m_maxRowHeight;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::DashboardRenderer::ColumnEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::DashboardRenderer::ColumnEntry& value,
        unsigned int version) const {
      shuttle.shuttle("width", value.m_width);
      shuttle.shuttle("index", value.m_index);
    }
  };

  template<>
  struct Shuttle<Spire::DashboardRendererSettings> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::DashboardRendererSettings& value,
        unsigned int version) const {
      shuttle.shuttle("columns", value.m_columns);
      shuttle.shuttle("empty_rows", value.m_emptyRows);
      shuttle.shuttle("default_column_width", value.m_defaultColumnWidth);
      shuttle.shuttle("minimum_column_width", value.m_minimumColumnWidth);
      shuttle.shuttle("max_row_height", value.m_maxRowHeight);
    }
  };
}

#endif
