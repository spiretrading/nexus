import {
  Money
} from 'spire-client';
import ChainableModel from './chainable-model';
import DataChangeType from './data-change-type';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import currencyFormatter from 'utils/currency-formatter';
import ViewModelData from './portfolio-view-model-data';
import ValueComparer from './value-comparer';

const GREEN = '#24aa08';
const RED = '#d32f2f';
const BLACK = 'black';
const WHITE = 'white';
const LIGHT_GRAY = '#fbfbfb';

export default class extends ChainableModel {
  constructor(sourceModel, currencyModel) {
    super(sourceModel);
    this.currencyModel = currencyModel;
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(x, y);
    let display = this.stringifyValue(value, x, y);
    let color = this.getColor(x, value);
    let backgroundColor = this.getBackgroundColor(y);
    return new ViewModelData(value, display, color, backgroundColor, ValueComparer);
  }

  /** @private */
  stringifyValue(value, x, y) {
    // constructor name
    let constructorName = value.constructor.name;
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

    let currencyId;
    if (constructorName === 'Money') {
      currencyId = this.sourceModel.getValueAt(10, y);  // currency ID column
    }

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

  /** @private */
  getColor(x, value) {
    if (x == 5 ||
      x == 6 ||
      x == 7 ||
      x == 13 ||
      x == 14) {
      let zero = new Money(0);
      let compareResult = value.compare(zero);
      if (compareResult < 0) {
        return RED;
      } else if (compareResult > 0) {
        return GREEN;
      }
    }

    return BLACK;
  }

  /** @private */
  getBackgroundColor(y) {
    if (y % 2 == 1) {
      return LIGHT_GRAY;
    } else {
      return WHITE;
    }
  }

  getColumnHeader(x) {
    return this.sourceModel.getColumnHeader(x);
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
}
