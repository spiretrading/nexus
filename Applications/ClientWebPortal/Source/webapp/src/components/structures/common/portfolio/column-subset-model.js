import DataChangeType from './data-change-type';
import uuid from 'uuid';
import HashMap from 'hashmap';

export default class {
  constructor(sourceModel, columnsToOmit) {
    this.omittedCount = 0;
    this.sourceModel = sourceModel;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
    this.dataChangeListeners = new HashMap();

    // initialize omittedToSourceColumns mapping
    this.omittedToSourceColumns = [];
    let sourceColumnsCount = this.sourceModel.getColumnCount();
    for (let x=0; x<sourceColumnsCount; x++) {
      if (columnsToOmit.includes(x)) {
        this.omittedCount++;
      } else {
        this.omittedToSourceColumns.push(x);
      }
    }
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount() - this.omittedCount;
  }

  getValueAt(x, y) {
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

  addDataChangeListener(listener) {
    let subId = uuid.v4();
    this.dataChangeListeners.set(subId, listener);
    return subId;
  }

  removeDataChangeListener(subId) {
    this.dataChangeListeners.remove(subId);
  }
}
