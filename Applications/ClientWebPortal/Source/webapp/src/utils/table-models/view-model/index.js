import Model from 'utils/table-models/model';
import ViewData from 'utils/table-models/view-model/view-data';
import DefaultStyleRule from 'utils/table-models/default-style-rule';
import numberFormatter from 'utils/number-formatter';
import DataChangeType from 'utils/table-models/model/data-change-type';

export default class extends Model {
  constructor(sourceModel) {
    super();
    this.sourceModel = sourceModel;
    this.defaultStyle = new DefaultStyleRule();

    this.getDataChangeListener = this.getDataChangeListener.bind(this);
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
    return new ViewData(value, display, this.getStyle(x, y));
  }

 /** @private */
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

  /** @private */
  getStyle(x, y) {
    return this.defaultStyle;
  }

  addDataChangeListener(listener) {
    let dataChangeListener = this.getDataChangeListener(listener);
    return this.source.addDataChangeListener(dataChangeListener);
  }

  /** @private */
  getDataChangeListener(listener) {
    return function(dataChangeType, payload) {
      if (changeType == DataChangeType.REMOVE) {
        let transformed = this.transformRemoveRowEventPayload(payload);
        listener(dataChangeType, transformed);
      } else if (changeType == DataChangeType.UPDATE) {
        let transformed = this.transformUpdateRowEventPayload(payload);
        listener(dataChangeType, transformed);
      } else {
        listener(dataChangeType, payload);
      }
    }.bind(this);
  }

  /** @private */
  transformRemoveRowEventPayload(payload) {
    let display = this.getDisplay(payload.row);
    return {
      index: payload.index,
      row: new ViewData(payload.row, display, this.defaultStyle)
    };
  }

  /** @private */
  transformUpdateRowEventPayload(payload) {
    let display = this.getDisplay(payload.original);
    return {
      index: payload.index,
      row: new ViewData(payload.original, display, this.defaultStyle)
    };
  }

  removeDataChangeListener(subId) {
    this.source.removeDataChangeListener(subId);
  }
}
