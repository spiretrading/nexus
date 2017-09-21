import ColumnType from 'components/reusables/common/big-table/column-type';

let columns = [
  {
    id: 1,
    name: 'Account',
    type: ColumnType.STRING,
    isPrimaryKey: true
  },
  {
    id: 2,
    name: 'Security',
    type: ColumnType.STRING,
    isPrimaryKey: true
  },
  {
    id: 3,
    name: 'Quantity',
    type: ColumnType.Number,
    isPrimaryKey: false
  },
  {
    id: 4,
    name: 'Side',
    type: ColumnType.STRING,
    isPrimaryKey: false
  },
  {
    id: 5,
    name: 'Average Price',
    type: ColumnType.MONEY,
    isPrimaryKey: false
  },
  {
    id: 6,
    name: 'Total P/L',
    type: ColumnType.POSITIVE_NEGATIVE_MONEY,
    isPrimaryKey: false
  },
  {
    id: 7,
    name: 'Unrealized P/L',
    type: ColumnType.POSITIVE_NEGATIVE_MONEY,
    isPrimaryKey: false
  },
  {
    id: 8,
    name: 'Realized P/L',
    type: ColumnType.POSITIVE_NEGATIVE_MONEY,
    isPrimaryKey: false
  },
  {
    id: 9,
    name: 'Fees',
    type: ColumnType.MONEY,
    isPrimaryKey: false
  },
  {
    id: 10,
    name: 'Cost Basis',
    type: ColumnType.MONEY,
    isPrimaryKey: false
  },
  {
    id: 11,
    name: 'Currency',
    type: ColumnType.STRING,
    isPrimaryKey: false
  },
  {
    id: 12,
    name: 'Volume',
    type: ColumnType.NUMBER,
    isPrimaryKey: false
  },
  {
    id: 13,
    name: 'Trades',
    type: ColumnType.NUMBER,
    isPrimaryKey: false
  },
  {
    id: 14,
    name: 'Acc. Total P/L',
    type: ColumnType.POSITIVE_NEGATIVE_MONEY,
    isPrimaryKey: false
  },
  {
    id: 15,
    name: 'Acc. Unrealized P/L',
    type: ColumnType.POSITIVE_NEGATIVE_MONEY,
    isPrimaryKey: false
  },
  {
    id: 16,
    name: 'Acc. Fees',
    type: ColumnType.MONEY,
    isPrimaryKey: false
  }
];
export default columns;
