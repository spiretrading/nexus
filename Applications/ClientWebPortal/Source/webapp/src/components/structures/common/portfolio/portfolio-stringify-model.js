import DataChangeType from './data-change-type';
import uuid from 'uuid';
import HashMap from 'hashmap';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import currencyFormatter from 'utils/currency-formatter';

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
    switch(x) {
      case 0:
        constructorName = 'DirectoryEntry';
        break;
      case 1:
        constructorName = 'Security';
        break;
      case 2:
      case 11:
      case 12:
        constructorName = 'Number';
        break;
      case 10:
        constructorName = 'CurrencyId';
        break;
    }

    let constructorName = value.constructor.name;
    let currencyId;
    if (constructorName == 'Money') {
      currencyId = this.sourceModel.getValueAt(10, y);
    }

    value = this.stringifyValue(value, constructorName, currencyId);
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
  stringifyValue(value, constructorName, currencyId) {
    if (value.toString != null) {
      if (constructorName == 'Security') {
        return value.toString(definitionsService.getMarketDatabase());
      } else if (constructorName == 'DirectoryEntry') {
        return value.name;
      } else if (constructorName == 'CurrencyId') {
        return definitionsService.getCurrencyCode(value.toNumber());
      } else if (constructorName == 'Money') {
        return currencyFormatter.formatById(
          currencyId.toNumber(),
          value
        );
      } else if (constructorName == 'Number') {
        return numberFormatter.formatWithComma(value);
      } else {
        return value.toString();
      }
    } else {
      return '' + value;
    }
  }

  setSourceModel(sourceModel) {
    // unsub from previous source model
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);

    // sub new source model
    this.sourceModel = sourceModel;
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }
}
