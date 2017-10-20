import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import uuid from 'uuid';

const BUFFER_CELL_NUM = 4;

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, {});
    this.componentModel = componentModel;
    this.componentId = uuid.v4();
    this.isCntrlPressed = false;
    this.isShiftPressed = false;
    this.activeRowIndex = null;

    this.isInitialized = false;
    this.isViewingRegionDimensionsSet = false;  // due to bug in Safari you need to manually set
    this.onViewingRegionScroll = this.onViewingRegionScroll.bind(this);
    this.render = this.render.bind(this);
    this.onColumnHeaderClick = this.onColumnHeaderClick.bind(this);
    this.onTableRowClick = this.onTableRowClick.bind(this);
    this.onKeyDownCheckSelectionModifiers = this.onKeyDownCheckSelectionModifiers.bind(this);
    this.onKeyUpCheckSelectionModifiers = this.onKeyUpCheckSelectionModifiers.bind(this);
    this.cellSidePaddings = 55;
  }

  setTableModel(tableModel) {
    this.tableModel = tableModel;
  }

  updateColumnChange() {
    this.columnWidthsPx = [];
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

    this.totalWidthPx = 0;

    $.contextMenu({
      selector: '#' + this.componentId + ' .viewing-region',
      appendTo: '#' + this.componentId,
      items: {
        menu1: {name: 'Stop'},
        menu2: {name: 'Settings'}
      }
    });
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
      paddingRight: this.rightPadding || 0,
      width: this.totalWidthPx
    });
  }

  /** @private */
  getTextWidth(textLength, font) {
    // re-use canvas object for better performance
    let Ws = '';
    for (let i=0; i<textLength; i++) {
      Ws += 'A';
    }
    var canvas = this.canvas || (this.canvas = document.createElement("canvas"));
    var context = canvas.getContext("2d");
    context.font = font;
    var metrics = context.measureText(Ws);
    return Math.ceil(metrics.width) + this.cellSidePaddings;
  }

  /** @private */
  onColumnHeaderClick(e) {
    let columnIndex = $(e.currentTarget).attr('data-index');
    this.controller.onSortColumnSelected(Number(columnIndex));
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
    $.contextMenu('destroy');
    $('#' + this.componentId + ' .context-menu-list').remove();
  }

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

    let columnLengths = this.tableModel.getColumnLengths();
    let $columnHeaders = $('#' + this.componentId + ' .column-headers div');
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
          color: cellValue.color,
          backgroundColor: cellValue.backgroundColor
        };

        let cellClassName = '';
        if (this.tableModel.isRowSelectedControlModified(y)) {
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
  }

  componentWillUpdate() {
    if (!this.isInitialized) {
      if (!this.isViewingRegionDimensionsSet) {
        this.setViewingRegionDimensions();
        this.isViewingRegionDimensionsSet = true;
      }
      this.initialize();
      this.isInitialized = true;
    }

    if (this.isInitialized) {
      this.updateViewingRegiongCoordinates();
      this.renderTable();
      this.calculatePaddings();
    }
  }

  componentDidUpdate() {
    if (this.isInitialized) {
      this.adjustColumnWidths();
    }
  }

  /** @private */
  adjustColumnWidths() {
    let columnLengths = this.tableModel.getColumnLengths();
    let $columnHeaders = $('#' + this.componentId + ' .column-headers>div');
    let totalWidth = 0;

    let coords = this.getXCoordinatesIncludingBuffers();

    let columnCount = this.tableModel.getColumnCount();
    for (let i=0; i<columnCount; i++) {
      let $columnHeader = $($columnHeaders[i]);
      let columnHeaderText = $columnHeader.text();
      let maxLength;
      if (columnLengths[i] != null && columnLengths[i].length != null) {
        maxLength = Math.max(columnHeaderText.length, columnLengths[i].length);
      } else {
        maxLength = columnHeaderText.length;
      }
      let maxWidth = this.getTextWidth(maxLength, this.componentModel.fontFamily);
      totalWidth += maxWidth;
      if (coords.leftX <= i && coords.rightX >= i) {
        let childIndex = i + 1 - coords.leftX;
        $('#' + this.componentId + ' .big-table-wrapper table tr td:nth-child(' + childIndex + ')').width(maxWidth);
      }
      $columnHeader.width(maxWidth);
      this.columnWidthsPx[i] = maxWidth;
    }

    $('#' + this.componentId + ' .column-headers').width(totalWidth);
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
            arrowIconClass = 'icon-arrow-icon-down';
          } else {
            // descending
            arrowIconClass = 'icon-arrow-icon-up';
          }
        }
      }
      cells.push(
        <div key={i} onClick={this.onColumnHeaderClick} data-index={i}>
          <div className="header-label">
            {this.tableModel.getColumnHeader(i)}
            <span className={arrowIconClass}></span>
          </div>
        </div>
      );
    }
    this.columnHeaders =  <div className="column-headers">
                            {cells}
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
