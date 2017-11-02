import ChainableModel from './chainable-model';
import DataChangeType from './data-change-type';

export default class extends ChainableModel{
  constructor(sourceModel, columnsToOmit) {
    super(sourceModel);
    this.omittedCount = 0;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);

    // initialize omittedToSourceColumns mapping
    this.omittedToSourceColumns = [];
    let sourceColumnsCount = this.sourceModel.getColumnCount();
    for (let x=0; x<sourceColumnsCount; x++) {
      if (this.hasElement(columnsToOmit, x)) {
        this.omittedCount++;
      } else {
        this.omittedToSourceColumns.push(x);
      }
    }
  }

  /** @private */
  hasElement(array, element) {
    for (let i=0; i<array.length; i++) {
      if (array[i] == element) {
        return true;
      }
    }
    return false;
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount() - this.omittedCount;
  }

  getColumnHeader(x) {
    return this.sourceModel.getColumnHeader(this.omittedToSourceColumns[x]);
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(this.omittedToSourceColumns[x], y);
    return this.sourceModel.getValueAt(this.omittedToSourceColumns[x], y);
  }

  onDataChange(dataChangeType, rowIndex, toIndex) {
    let listeners = this.dataChangeListeners.values();
    if (dataChangeType != DataChangeType.MOVE) {
      for (let i=0; i<listeners.length; i++) {
        listeners[i](dataChangeType, rowIndex);
      }
    } else {
      for (let i=0; i<listeners.length; i++) {
        listeners[i](DataChangeType.MOVE, rowIndex, toIndex);
      }
    }
  }
}
