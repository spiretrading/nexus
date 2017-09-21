import DataChangeType from './data-change-type';
import uuid from 'uuid';
import HashMap from 'hashmap';
import definitionsService from 'services/definitions';

export default class {
  constructor(sourceModel) {
    this.sourceModel = sourceModel;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
    this.dataChangeListeners = new HashMap();
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(x, y);
    let constructorName = null;
    switch(x) {
      case 0:
        constructorName = 'DirectoryEntry';
        break;
      case 1:
        constructorName = 'Security';
        break;
      case 10:
        constructorName = 'CurrencyId';
        break;
    }
    value = this.stringifyValue(value, constructorName);
    return value;
  }

  onDataChange(dataChangeType, rowIndex, toIndex) {
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      if (dataChangeType != DataChangeType.MOVE) {
        listeners[i](dataChangeType, rowIndex);
      } else {
        listeners[i](dataChangeType, rowIndex, toIndex);
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

  /** @private */
  stringifyValue(value, constructorName) {
    if (value.toString != null) {
      if (constructorName == 'Security') {
        return value.toString(definitionsService.getMarketDatabase());
      } else if (constructorName == 'DirectoryEntry') {
        return value.name;
      } else if (constructorName == 'CurrencyId') {
        return definitionsService.getCurrencyCode(value.toNumber());
      } else {
        return value.toString();
      }
    } else {
      return '' + value;
    }
  }

  /** @private */
  stringifyRow(row) {
    for (let property in row) {
      row[property] = this.stringifyValue(row[property], row[property].constructor.name);
    }
    return row;
  }
}
