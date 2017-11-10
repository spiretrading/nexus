import ChainableModel from 'utils/table-models/chainable-model';
import ViewData from 'utils/table-models/view-model/view-data';
import DefaultStyleRule from 'utils/table-models/default-style-rule';
import numberFormatter from 'utils/number-formatter';

export default class extends ChainableModel {
  constructor(sourceModel) {
    super(sourceModel);
    this.defaultStyle = new DefaultStyleRule();
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(columnIndex) {
    return this.sourceModel.getColumnName(columnIndex);
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(x, y);
    return this.toViewData(x, y, value);
  }

  /** @protected */
  toViewData(x, y, value) {
    let display = this.getDisplay(value);
    return new ViewData(value, display, this.defaultStyle);
  }

  getDisplay(value) {
    if (value == null) {
      return 'N/A';
    } else if (typeof value === 'number' && !Number.isNaN(value)) {
      return numberFormatter.formatWithComma(value);
    } else if (typeof value === 'string') {
      return value;
    } else {
      return value.toString();
    }
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }
}
