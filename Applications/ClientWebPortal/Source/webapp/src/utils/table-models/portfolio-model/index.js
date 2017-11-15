import Model from 'utils/table-models/model';
import IndexedModel from 'utils/table-models/indexed-model';

const KEY_INDICES = [0, 1, 10];
const COLUMN_NAMES = [
  'Account',
  'Security',
  'Quantity',
  'Side',
  'Average Price',
  'Total P&L',
  'Unrealized P&L',
  'Realized P&L',
  'Fees',
  'Cost Basis',
  'Currency',
  'Volume',
  'Trades',
  'Acc. Total P&L',
  'Acc. Unrealized P&L',
  'Acc. Fees'
];

export default class extends Model {
  constructor(riskServiceClient) {
    super();
    this.riskServiceClient = riskServiceClient;
    this.indexedModel = new IndexedModel(KEY_INDICES, COLUMN_NAMES);
    this.onDataReceived = this.onDataReceived.bind(this);

    this.riskServiceClient.subscribePortfolio(this.onDataReceived)
      .then(subscriptionId => {
        this.subscriptionId = subscriptionId;
      });
  }

  getRowCount() {
    return this.indexedModel.getRowCount();
  }

  getColumnCount(columnIndex) {
    return this.indexedModel.getColumnCount(columnIndex);
  }

  getValueAt(x, y) {
    return this.indexedModel.getValueAt(x, y);
  }

  getColumnName(columnIndex) {
    return this.indexedModel.getColumnName(columnIndex);
  }

  addDataChangeListener(listener) {
    return this.indexedModel.addDataChangeListener(listener);
  }

  removeDataChangeListener(subId) {
    this.indexedModel.removeDataChangeListener(subId);
  }

  dispose() {
    this.riskServiceClient.unsubscribePortfolio(this.subscriptionId);
  }

  /** @private */
  onDataReceived(data) {
    for (let i=0; i<data.length; i++) {
      let rowData = Object.freeze(this.toRowData(data[i]));
      if (data[i].inventory.transaction_count == 0) {
        this.indexedModel.removeRow(rowData);
      } else {
        this.indexedModel.update(rowData);
      }
    }
  }

  /** @private */
  toRowData(data) {
    let rowData = [];
    // account
    rowData.push(data.account);
    // security
    rowData.push(data.inventory.position.key.index);
    // quantity
    rowData.push(data.inventory.position.quantity);
    // side
    if (data.inventory.position.quantity > 0) {
      rowData.push('Long');
    } else if (data.inventory.position.quantity < 0) {
      rowData.push('Short');
    } else {
      rowData.push('Flat');
    }
    // average price
    let averagePrice = null;
    if (data.inventory.position.quantity != 0) {
      let costBasis = data.inventory.position.cost_basis;
      let quantity = data.inventory.position.quantity;
      averagePrice = costBasis.divide(quantity);
      if (averagePrice.toNumber() < 0) {
        averagePrice.multiply(-1);
      }
    }
    rowData.push(averagePrice);
    // totalPnL, unrealizedPnL, realizedPnL
    let totalPnL = null;
    let unrealizedPnL = null;
    let grossPnL = data.inventory.gross_profit_and_loss;
    let fees = data.inventory.fees;
    if (data.unrealized_profit_and_loss.is_initialized) {
      unrealizedPnL = data.unrealized_profit_and_loss.value;
      totalPnL = unrealizedPnL.add(grossPnL).subtract(fees);
    }
    rowData.push(totalPnL);
    rowData.push(unrealizedPnL);
    rowData.push(grossPnL.subtract(fees));
    // fees
    rowData.push(fees);
    // cost Basis
    rowData.push(data.inventory.position.cost_basis);
    // currency
    rowData.push(data.inventory.position.key.currency);
    // volume
    rowData.push(data.inventory.volume);
    // transaction count
    rowData.push(data.inventory.transaction_count);

    return rowData;
  }
}
