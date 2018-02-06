import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import HeaderInteractionState from './header-interaction-states';
import uuid from 'uuid';

const BUFFER_CELL_NUM = 4;
const DEFAULT_COLUMN_WIDTH = 200;
const CELL_SIDE_PADDING = 90;
const COLUMN_ADJUST_MOUSE_PADDING = 2;
const MINIMUM_COLUMN_WIDTH = 100;

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, {});
    this.componentModel = componentModel;
    this.componentId = uuid.v4();
    this.isCntrlPressed = false;
    this.isShiftPressed = false;
    this.activeRowIndex = null;
    this.headerInteractionState = HeaderInteractionState.IDLE;
    this.hoveredColumnIndex = null;
    this.contextMenu = componentModel.contextMenu;

    this.isInitialized = false;
    this.isViewingRegionDimensionsSet = false;  // due to bug in Safari you need to manually set
    this.onViewingRegionScroll = this.onViewingRegionScroll.bind(this);
    this.render = this.render.bind(this);
    this.onColumnHeaderClick = this.onColumnHeaderClick.bind(this);
    this.onTableRowClick = this.onTableRowClick.bind(this);
    this.onKeyDownCheckSelectionModifiers = this.onKeyDownCheckSelectionModifiers.bind(this);
    this.onKeyUpCheckSelectionModifiers = this.onKeyUpCheckSelectionModifiers.bind(this);
    this.onColumnHeaderMouseMove = this.onColumnHeaderMouseMove.bind(this);
    this.onColumnHeaderMouseDown = this.onColumnHeaderMouseDown.bind(this);
    this.onColumnHeaderMouseUp = this.onColumnHeaderMouseUp.bind(this);
    this.onColumnHeaderMouseLeave = this.onColumnHeaderMouseLeave.bind(this);
    this.onMouseUp = this.onMouseUp.bind(this);
  }

  setTableModel(tableModel) {
    this.tableModel = tableModel;
  }

  updateColumnChange() {
    this.resetColumnWidthsPx();
  }

  getViewingRegionWidthHeight() {
    let $viewingRegion = $('#' + this.componentId + ' .viewing-region');
    return {
      width: $viewingRegion.outerWidth(),
      height: $viewingRegion.height()
    };
  }

  initialize() {
    $('#' + this.componentId + ' .viewing-region').scroll(this.onViewingRegionScroll);
    $(document).keydown(this.onKeyDownCheckSelectionModifiers)
      .keyup(this.onKeyUpCheckSelectionModifiers);

    this.scrolledPx = {
      top: 0,
      left: 0
    };

    this.viewingCoords = {
      topLeftX: 0,
      topLeftY: 0,
      bottomRightX: 0,
      bottomRightY: 0
    };

    if (this.contextMenu != null) {
      this.contextMenu.setTableModel(this.tableModel);
      $.contextMenu({
        selector: '#' + this.componentId + ' .viewing-region',
        appendTo: '#' + this.componentId,
        items: this.contextMenu.getMenuItems(),
        callback: this.contextMenu.onMenuItemClick.bind(this.contextMenu)
      });
    }

    this.resetColumnWidthsPx();
    this.updateColumnWidthsAndEllipsis();

    $('#' + this.componentId + ' .column-headers tr').mousemove(this.onColumnHeaderMouseMove)
      .mousedown(this.onColumnHeaderMouseDown)
      .mouseup(this.onColumnHeaderMouseUp)
      .mouseleave(this.onColumnHeaderMouseLeave);
    
    $(document).mouseup(this.onMouseUp);
  }

  /** @private */
  onMouseUp(e) {
    let $container = $('#' + this.componentId);
    let wasTableClicked = $.contains($container[0], e.target);
    if (!wasTableClicked) {
      this.activeRowIndex = null;
      this.tableModel.clearSelectedRows();
      this.update();
    }
  }

  /** @private */
  onColumnHeaderMouseMove(e) {
    let headerPageOffsetX = $(e.currentTarget).offset().left;
    let eventPageOffsetX = e.pageX;
    let relativeToHeaderOffsetX = eventPageOffsetX - headerPageOffsetX;
    if (this.headerInteractionState != HeaderInteractionState.DRAGGING_COLUMN_BORDER) {
      let hoveredColumnIndex = this.gethoveredColumnIndex(relativeToHeaderOffsetX);
      if (hoveredColumnIndex != -1) {
        if (this.headerInteractionState == HeaderInteractionState.IDLE) {
          this.headerInteractionState = HeaderInteractionState.HOVER_COLUMN_BORDER;
          this.onHeaderHoverColumnBorderEnter(hoveredColumnIndex);
        }
      } else {
        this.headerInteractionState = HeaderInteractionState.IDLE;
        this.onHeaderIdleEnter();
      }
    } else {
      // column border is being dragged, findout what the delta is
      let currentBorderOffsetX = this.getBorderOffsetX(this.hoveredColumnIndex);
      let draggedDelta = currentBorderOffsetX - relativeToHeaderOffsetX;
      this.updateColumnWidth(this.hoveredColumnIndex, draggedDelta);
    }
  }

  /** @private */
  // border indices are based from 0 and the edges of the table do not count
  // delta: positive is moving to left, negative is moving to right
  updateColumnWidth(borderIndex, delta) {
    // update column widths
    let leftColumnIndex = borderIndex;
    let rightColumnIndex = borderIndex + 1;
    let leftColumnOriginalWidth = this.columnWidthsPx[leftColumnIndex];
    let rightColumnOriginalWidth = this.columnWidthsPx[rightColumnIndex];
    this.columnWidthsPx[leftColumnIndex] -= delta;
    this.columnWidthsPx[rightColumnIndex] += delta;

    // check if one of the columns has hit the minimum width
    let isLeftReachedMinimum = this.columnWidthsPx[leftColumnIndex] < MINIMUM_COLUMN_WIDTH;
    let isRightReachedMinimum = this.columnWidthsPx[rightColumnIndex] < MINIMUM_COLUMN_WIDTH;
    if (isLeftReachedMinimum || isRightReachedMinimum) {
      if (isLeftReachedMinimum) {
        this.columnWidthsPx[leftColumnIndex] = MINIMUM_COLUMN_WIDTH;
        let leftColumnDelta = leftColumnOriginalWidth - this.columnWidthsPx[leftColumnIndex];
        this.columnWidthsPx[rightColumnIndex] = rightColumnOriginalWidth + leftColumnDelta;
      } else {
        this.columnWidthsPx[rightColumnIndex] = MINIMUM_COLUMN_WIDTH;
        let rightColumnDelta = rightColumnOriginalWidth - this.columnWidthsPx[rightColumnIndex];
        this.columnWidthsPx[leftColumnIndex] = leftColumnOriginalWidth + rightColumnDelta;
      }
    }

    this.update();
    this.updateColumnWidthsAndEllipsis();
  }

  /** @private */
  getBorderOffsetX(columnIndex) {
    let offset = 0;
    for (let i=0; i<=columnIndex; i++) {
      offset += this.columnWidthsPx[i];
    }
    return offset;
  }

  /** @private */
  onColumnHeaderMouseDown(e) {
    if (this.headerInteractionState == HeaderInteractionState.HOVER_COLUMN_BORDER) {
      this.headerInteractionState = HeaderInteractionState.DRAGGING_COLUMN_BORDER;
    }
  }

  /** @private */
  onColumnHeaderMouseUp(e) {
    let headerPageOffsetX = $(e.currentTarget).offset().left;
    let eventPageOffsetX = e.pageX;
    let relativeToHeaderOffsetX = eventPageOffsetX - headerPageOffsetX;
    let hoveredColumnIndex = this.gethoveredColumnIndex(relativeToHeaderOffsetX);
    if (hoveredColumnIndex != -1) {
      this.headerInteractionState = HeaderInteractionState.HOVER_COLUMN_BORDER;
      this.onHeaderHoverColumnBorderEnter(hoveredColumnIndex);
    } else {
      this.headerInteractionState = HeaderInteractionState.IDLE;
      this.onHeaderIdleEnter();
    }
  }

  /** @private */
  onColumnHeaderMouseLeave(e) {
    if (this.headerInteractionState == HeaderInteractionState.DRAGGING_COLUMN_BORDER) {
      this.headerInteractionState = HeaderInteractionState.IDLE;
      this.onHeaderIdleEnter();
    }
  }

  /** @private */
  onHeaderHoverColumnBorderEnter(hoveredColumnIndex) {
    this.hoveredColumnIndex = hoveredColumnIndex;
    $('#' + this.componentId + ' .column-headers tr')[0].style.cursor = 'ew-resize';
  }

  /** @private */
  onHeaderIdleEnter() {
    this.hoveredColumnIndex = null;
    $('#' + this.componentId + ' .column-headers tr')[0].style.cursor = 'pointer';
  }

  /** @private */
  gethoveredColumnIndex(position) {
    let cumulativeWidth = 0;
    for (let i=0; i<this.columnWidthsPx.length-1; i++) {
      cumulativeWidth += this.columnWidthsPx[i];
      let leftBoundary = cumulativeWidth - COLUMN_ADJUST_MOUSE_PADDING;
      let rightBoundary = cumulativeWidth + COLUMN_ADJUST_MOUSE_PADDING;
      if (position >= leftBoundary && position <= rightBoundary) {
        return i;
      }
    }

    return -1;
  }

  /** @private */
  resetColumnWidthsPx() {
    this.columnWidthsPx = [];
    for (let i=0; i<this.tableModel.getColumnCount(); i++) {
      this.columnWidthsPx.push(DEFAULT_COLUMN_WIDTH);
    }
  }

  /** @private */
  updateColumnWidthsAndEllipsis() {
    let $cells = $('#' + this.componentId + ' .column-headers td div.header-label .text-label');
    for (let i=0; i<$cells.size(); i++) {
      let $cell = $($cells[i]);
      let $arrow = $cell.find('span');
      let text = this.tableModel.getColumnHeader(i);
      let textWidthPx = this.getTextWidth(text, this.fontFamily);
      if (textWidthPx > $cell.width()) {
        for (let j=0; j<text.length; j++) {
          let substring = text.substring(0, j);
          substring += '...';
          let substringWidthPx = this.getTextWidth(substring, this.fontFamily);
          if (substringWidthPx > this.columnWidthsPx[i]) {
            text = text.substring(0, j-1) + '...';
            break;
          }
        }
      }
      $cell.text(text);
    }
  }

  /** @private */
  calculatePaddings() {
    // horizontal padding
    this.leftPadding = 0;
    this.rightPadding = 0;
    let coords = this.getXCoordinatesIncludingBuffers();
    let leftX = coords.leftX;
    let rightX = coords.rightX;
    let columnCount = this.tableModel.getColumnCount();
    for (let i=0; i<columnCount; i++) {
      if (leftX > i) {
        this.leftPadding += this.columnWidthsPx[i];
      } else if (i > rightX) {
        this.rightPadding += this.columnWidthsPx[i];
      }
    }

    // vertical padding
    this.topPadding = 0;
    this.bottomPadding = 0;
    coords = this.getYCoordinatesIncludingBuffers();
    let topY = coords.topY;
    let bottomY = coords.bottomY;
    let numRecords = this.tableModel.getRowCount();
    let singleRowHeight = Number(this.componentModel.lineHeight);
    for (let i=0; i<topY; i++) {
      this.topPadding += singleRowHeight;
    }
    for (let i=bottomY; i<numRecords; i++) {
      this.bottomPadding += singleRowHeight;
    }

    let viewingRegionWidth = this.getViewingRegionWidthHeight().width;
    $('#' + this.componentId + ' .big-table-wrapper').css({
      paddingTop: this.topPadding || 0,
      paddingBottom: this.bottomPadding || 0,
      paddingLeft: this.leftPadding || 0,
      paddingRight: this.rightPadding || 0
    });
  }

  /** @private */
  getTextWidth(text, font) {
    // re-use canvas object for better performance
    var canvas = this.canvas || (this.canvas = document.createElement("canvas"));
    var context = canvas.getContext("2d");
    context.font = font;
    var metrics = context.measureText(text);
    return Math.ceil(metrics.width) + CELL_SIDE_PADDING;
  }

  /** @private */
  onColumnHeaderClick(e) {
    if (this.headerInteractionState == HeaderInteractionState.IDLE) {
      let columnIndex = $(e.currentTarget).attr('data-index');
      this.controller.onSortColumnSelected(Number(columnIndex));
    }
  }

  setViewingRegionDimensions() {
    let $container = $('#' + this.componentId);
    let width = $container.width();
    let height = $container.height();
    $('#' + this.componentId + ' .viewing-region').css({
      width: width,
      height: height
    });
  }

  dispose() {
    $('#' + this.componentId + ' .viewing-region').unbind('scroll');
    $(document).unbind('keydown', this.onKeyDownCheckSelectionModifiers)
      .unbind('keyup', this.onKeyUpCheckSelectionModifiers);

    if (this.contextMenu != null) {
      $.contextMenu('destroy');
    }

    $('#' + this.componentId + ' .context-menu-list').remove();
    $('#' + this.componentId + ' .column-headers tr').unbind('mousemove');
    $(document).unbind('mouseup', this.onMouseUp);
  }

  /** @private */
  getXCoordinatesIncludingBuffers() {
    let columnCount = this.tableModel.getColumnCount();
    let leftX = this.viewingCoords.topLeftX - BUFFER_CELL_NUM;
    if (leftX < 0) {
      leftX = 0;
    }
    let rightX = this.viewingCoords.bottomRightX + BUFFER_CELL_NUM;
    if (rightX >= columnCount) {
      rightX = columnCount - 1;
    }

    return {
      leftX: leftX,
      rightX: rightX
    };
  }

  /** @private */
  getYCoordinatesIncludingBuffers() {
    let numRecords = this.tableModel.getRowCount();
    let topY = this.viewingCoords.topLeftY  - BUFFER_CELL_NUM;
    if (topY < 0) {
      topY = 0;
    }
    let bottomY = this.viewingCoords.bottomRightY + BUFFER_CELL_NUM + 1;
    if (bottomY >= numRecords) {
      bottomY = numRecords - 1;
    }

    return {
      topY: topY,
      bottomY: bottomY
    };
  }

  /** @private */
  onViewingRegionScroll() {
    this.updateViewingRegiongCoordinates();
    this.updateColumnHeaderPosition();
    this.update();
  }

  /** @private */
  updateColumnHeaderPosition() {
    let left = this.scrolledPx.left * -1;
    $('#' + this.componentId + ' .column-headers').css('left', left);
  }

  /** @private */
  updateViewingRegiongCoordinates() {
    let $bigTableWrapper = $('#' + this.componentId + ' .viewing-region');
    this.scrolledPx.top = parseInt($bigTableWrapper.scrollTop());
    this.scrolledPx.left = parseInt($bigTableWrapper.scrollLeft());

    let $columnHeaders = $('#' + this.componentId + ' .column-headers td');
    this.columnWidthsPx = this.columnWidthsPx || [];

    // update viewing coordinates
    let totalWidths = 0;
    let leftX = 0;
    let rightX = 0;
    let viewingRegionRightPx = this.scrolledPx.left + this.getViewingRegionWidthHeight().width;
    for (let i=0; i<this.tableModel.getColumnCount(); i++) {
      totalWidths += this.columnWidthsPx[i];
      if (this.scrolledPx.left > totalWidths) {
        leftX++;
      }
      if (viewingRegionRightPx > totalWidths) {
        rightX++;
      }
    }
    this.viewingCoords.topLeftX = leftX;
    this.viewingCoords.bottomRightX = rightX;

    let totalHeight = 0;
    let topY = 0;
    let bottomY = 0;
    let viewingRegionBottomPx = this.scrolledPx.top + this.getViewingRegionWidthHeight().height;
    let numRecords = this.tableModel.getRowCount();
    for (let i=0; i<numRecords; i++) {
      totalHeight += Number(this.componentModel.lineHeight);
      if (this.scrolledPx.top > totalHeight) {
        topY++;
      }
      if (viewingRegionBottomPx > totalHeight) {
        bottomY++;
      }
      this.viewingCoords.topLeftY = topY;
      this.viewingCoords.bottomRightY = bottomY;
    }
  }

  renderTable() {
    let rows = [];
    let columns = [];
    let xCoords = this.getXCoordinatesIncludingBuffers();
    let yCoords = this.getYCoordinatesIncludingBuffers();
    let bottomY = Math.min(yCoords.bottomY, this.tableModel.getRowCount() - 1);
    for (let y=yCoords.topY; y<=bottomY; y++) {
      // loop for rows
      for (let x=xCoords.leftX; x<=xCoords.rightX; x++) {
        // loop for columns
        let columnWidth = this.columnWidthsPx[x];
        let cellValue = this.tableModel.getValueAt(x, y);
        let className = '';
        let cellStyle = {
          width: columnWidth,
          height: Number(this.componentModel.lineHeight),
          color: cellValue.style.fontColor,
          backgroundColor: cellValue.style.backgroundColor
        };

        let cellClassName = '';
        if (this.tableModel.isRowSelected(y)) {
          cellClassName += 'selected';
        }

        columns.push(
          <td key={x + '-' + y} style={cellStyle} className={cellClassName}>
            {cellValue.display}
          </td>
        );
      }

      rows.push(
        <tr key={y} className="no-select" data-row-index={y} onClick={this.onTableRowClick}>
          {columns}
        </tr>
      );
      columns = [];
    }

    this.table =  <table cellPadding="0" cellSpacing="0">
                    <tbody>
                      {rows}
                    </tbody>
                  </table>;
  }

  /** @private */
  onTableRowClick(e) {
   let $row = $(e.currentTarget);
   let clickedRowIndex = Number($row.attr('data-row-index'));

   if (!this.isCntrlPressed && !this.isShiftPressed) {
     // when no modifiers are pressed
     this.activeRowIndex = clickedRowIndex;
     this.tableModel.clearSelectedRows();
     this.tableModel.setSelectedRow(clickedRowIndex, false);
   } else if (this.isCntrlPressed && !this.isShiftPressed) {
     // when only the control modifier is pressed
     this.activeRowIndex = clickedRowIndex;
     if (this.tableModel.isRowSelectedControlModified(clickedRowIndex)) {
      this.tableModel.removeRowSelection(clickedRowIndex);
     } else {
       this.tableModel.setSelectedRow(clickedRowIndex, true);

       // set all existing selected rows with control modifier applied
       this.tableModel.setAllSelectedRowsControlModified();
     }
   } else {
     // when shift modifier is pressed
     let $allRows = $row.parent().children();
     let rowCounter = this.tableModel.getRowCount();
     let pivotRowsMetCounter = 0;

     // unselect the selected rows not chosen by control modifier
     this.tableModel.removeNonControlModifiedSelectedRows();

     for (let i=0; i<rowCounter; i++) {
       if (this.activeRowIndex == null) {
         this.activeRowIndex = clickedRowIndex;
       }

       let $currentRow = $(this);
       let currentRowIndex = i;
       if (currentRowIndex === this.activeRowIndex || currentRowIndex === clickedRowIndex) {
         pivotRowsMetCounter++;
       }

       if (pivotRowsMetCounter > 0 && pivotRowsMetCounter <= 2) {
         this.tableModel.setSelectedRow(currentRowIndex, false);

         if (pivotRowsMetCounter == 2) {
           pivotRowsMetCounter++;
         }
       }
     }
   }
   this.update();
  }

  componentWillUpdate() {
    if (!this.isInitialized) {
      if (!this.isViewingRegionDimensionsSet) {
        this.setViewingRegionDimensions();
        this.isViewingRegionDimensionsSet = true;
      }
      this.isInitialized = true;
    }

    if (this.isInitialized) {
      this.updateViewingRegiongCoordinates();
      this.renderTable();
      this.calculatePaddings();
    }
  }

  /** @private */
  renderColumnHeaders() {
    let cells = [];
    let columnSortOrders = this.controller.getColumnSortOrders();
    let columnCount = this.tableModel.getColumnCount();
    for (let i=0; i<columnCount; i++) {
      let arrowIconClass = "hidden";
      if (columnSortOrders.length > 0) {
        if (columnSortOrders[0].index == i) {
          if (columnSortOrders[0].isAsc) {
            // ascending
            arrowIconClass = 'arrow-icon icon-arrow-icon-down';
          } else {
            // descending
            arrowIconClass = 'arrow-icon icon-arrow-icon-up';
          }
        }
      }
      let cellWidth;
      if (this.columnWidthsPx != null && this.columnWidthsPx[i] != null) {
        cellWidth = this.columnWidthsPx[i];
      } else {
        cellWidth = DEFAULT_COLUMN_WIDTH;
      }
      let style = {
        width: cellWidth
      };
      cells.push(
        <td key={i} onClick={this.onColumnHeaderClick} data-index={i}>
          <div style={style}>
            <div className="header-label">
              <span className="text-label">{this.tableModel.getColumnHeader(i)}</span>
              <span className={arrowIconClass}></span>
            </div>
          </div>
        </td>
      );
    }
    this.columnHeaders =  <div className="column-headers no-select">
                            <table>
                              <thead>
                                <tr>
                                  {cells}
                                </tr>
                              </thead>
                            </table>
                          </div>;
  }

  /** @private */
  onKeyDownCheckSelectionModifiers(event) {
    let isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;
    if((!isMac && event.which == '17') || (isMac && event.which == '91')) {
      this.isCntrlPressed = true;
    } else if (event.which == '16') {
      this.isShiftPressed = true;
    }
  }

  /** @private */
  onKeyUpCheckSelectionModifiers(event) {
    let isMac = navigator.platform.toUpperCase().indexOf('MAC')>=0;
    if((!isMac && event.which == '17') || (isMac && event.which == '91')) {
      this.isCntrlPressed = false;
    } else if (event.which == '16') {
      this.isShiftPressed = false;
    }
  }

  render() {
    let tableWrapperStyle = {
      paddingTop: this.topPadding || 0,
      paddingBottom: this.bottomPadding || 0,
      paddingLeft: this.leftPadding || 0,
      paddingRight: this.rightPadding || 0
    };

    this.renderColumnHeaders();

    return (
        <div id={this.componentId} className="big-table-container">
          {this.columnHeaders}
          <div className="viewing-region">
            <div className="big-table-wrapper" style={tableWrapperStyle}>
              {this.table}
            </div>
          </div>
        </div>
    );
  }
}

export default View;
