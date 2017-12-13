import Model from 'utils/table-models/model';
import ViewModel from 'utils/table-models/view-model';
import ViewData from 'utils/table-models/view-model/view-data';
import { Security, DirectoryEntry, Money } from 'spire-client';
import definitionsService from 'services/definitions';
import currencyFormatter from 'utils/currency-formatter';
import DataChangeType from 'utils/table-models/model/data-change-type';

export default class extends ViewModel {
  constructor(sourceModel, columnStyles, currencyIdColumnIndex) {
    super(sourceModel);
    this.columnStyles = columnStyles;
    this.currencyIdColumnIndex = currencyIdColumnIndex;
    this.onDataChange = this.onDataChange.bind(this);

    // replace parent's registered data change listener
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }

  /** @protected */
  getValueAt(x, y) {
    let currencyId = this.sourceModel.getValueAt(this.currencyIdColumnIndex, y);
    let columnName = this.getColumnName(x);
    let value = this.sourceModel.getValueAt(x, y);
    return this.toViewData(currencyId, columnName, value, y);
  }

  /** @protected */
  getDisplay(currencyId, value) {
    if (value instanceof Security) {
      let marketDatabase = definitionsService.getMarketDatabase();
      return value.toString(marketDatabase);
    } else if (value instanceof DirectoryEntry) {
      return value.name;
    } else if (value instanceof Money) {
      return currencyFormatter.formatById(currencyId.toNumber(), value.toNumber());
    } else {
      return super.getDisplay(value);
    }
  }

  /** @protected */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.REMOVE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        row: Object.freeze(payload.row.map((currentValue, index) => {
          return this.toViewData(
            payload.row[this.currencyIdColumnIndex],
            this.getColumnName(index),
            currentValue,
            payload.index
          );
        }))
      });
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        original: Object.freeze(payload.original.map((currentValue, index) => {
          return this.toViewData(
            payload.original[this.currencyIdColumnIndex],
            this.getColumnName(index),
            currentValue,
            payload.index
          );
        }))
      });
    } else {
      super.onDataChange(dataChangeType, payload);
    }
  }

  /** @private */
  toViewData(currencyId, columnName, value, y) {
    if (value == null) {
      return super.toViewData(value, y);
    } else {
      let display = this.getDisplay(currencyId, value);
      let style = this.getStyle(columnName);
      let generatedStyle = {
        fontColor: style.getFontColor(value),
        backgroundColor: style.getBackgroundColor(y, value)
      };
      return new ViewData(value, display, generatedStyle);
    }
  }

  /** @private */
  getStyle(columnName) {
    if (this.columnStyles[columnName] != null) {
      return this.columnStyles[columnName];
    } else {
      return this.defaultStyle;
    }
  }
}
