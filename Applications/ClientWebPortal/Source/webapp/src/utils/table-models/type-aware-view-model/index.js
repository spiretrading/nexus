import DataChangeType from 'utils/table-models/notify-change-model/data-change-type';
import ArrayStringKeyGenerator from 'utils/array-string-key-generator';
import definitionsService from 'services/definitions';
import numberFormatter from 'utils/number-formatter';
import currencyFormatter from 'utils/currency-formatter';
import ViewModel from 'utils/table-models/view-model';
import ViewData from 'utils/table-models/view-model/view-data';
import {
  DirectoryEntry,
  Security,
  CurrencyId,
  Money
} from 'spire-client';

// import ValueComparer from './value-comparer';

const GREEN = '#24aa08';
const RED = '#d32f2f';
const BLACK = 'black';
const WHITE = 'white';
const LIGHT_GRAY = '#fbfbfb';

export default class extends ViewModel {
  constructor(sourceModel) {
    super(sourceModel);
  }

  getRowCount() {
    return super.getRowCount();
  }

  getColumnCount() {
    return super.getColumnCount();
  }

  getColumnName(columnIndex) {
    return super.getColumnName(columnIndex);
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(x, y);

    if (value instanceof Security) {
      return value.toString(definitionsService.getMarketDatabase());
    } else if (value instanceof DirectoryEntry) {
      return value.name;
    } else if (value instanceof CurrencyId) {
      return definitionsService.getCurrencyCode(value.toNumber());
    } else if (value instanceof Money) {
      return currencyFormatter.formatById(
        currencyId.toNumber(),
        value
      );
    } else {
      return super.toViewData(value);
    }
  }

  /** @private */
  securityToViewData(value) {

  }
}
