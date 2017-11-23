import PortfolioModel from '../../../../webapp/utils/table-models/portfolio-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';
import {
  DirectoryEntry,
  Money,
  CurrencyId,
  Security
} from 'spire-client';

describe('PortfolioModel', function() {
  beforeAll(function() {
    this.baseCurrencyId = CurrencyId.fromData(124);
    this.portfolioModel = new PortfolioModel(new MockRiskServiceClient(), this.baseCurrencyId);
  });

  it('toRowData', function() {
    let mockData = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    let rowData = this.portfolioModel.toRowData(mockData);

    // account
    expect(rowData[0] instanceof DirectoryEntry).toBe(true);
    expect(rowData[0].id).toBe(1);
    expect(rowData[0].type).toBe(1);
    expect(rowData[0].name).toBe('account1');
    // security
    expect(rowData[1] instanceof Security).toBe(true);
    expect(rowData[1].country.value).toBe(124);
    expect(rowData[1].market.value).toBe('XTSE');
    expect(rowData[1].symbol).toBe('RY');
    // quantity
    expect(rowData[2]).toBe(100);
    // side
    expect(rowData[3]).toBe('Long');
    // average price
    expect(rowData[4] instanceof Money).toBe(true);
    expect(rowData[4].toNumber()).toBe(1000);
    // total P&L
    expect(rowData[5] instanceof Money).toBe(true);
    expect(rowData[5].toNumber()).toBe(1988);
    // unrealized P&L
    expect(rowData[6] instanceof Money).toBe(true);
    expect(rowData[6].toNumber()).toBe(1111);
    // realized P&L
    expect(rowData[7] instanceof Money).toBe(true);
    expect(rowData[7].toNumber()).toBe(877);
    // fees
    expect(rowData[8] instanceof Money).toBe(true);
    expect(rowData[8].toNumber()).toBe(123);
    // cost basis
    expect(rowData[9] instanceof Money).toBe(true);
    expect(rowData[9].toNumber()).toBe(100000);
    // currency
    expect(rowData[10] instanceof CurrencyId).toBe(true);
    expect(rowData[10].toNumber()).toBe(124);
    // volume
    expect(rowData[11]).toBe(100);
    // transaction count
    expect(rowData[12]).toBe(1);
  });
});

describe('onDataChange', function() {
  let portfolioModel;
  let baseCurrencyId = CurrencyId.fromData(124);

  beforeEach(function() {
    portfolioModel = new PortfolioModel(new MockRiskServiceClient(), baseCurrencyId);

  });

  it('Row add - one row', function(done) {
    portfolioModel.addDataChangeListener(function(dataChangeType, payload) {
      expect(dataChangeType).toBe(DataChangeType.ADD);
      expect(payload).toBe(0);
      expect(portfolioModel.getRowCount()).toBe(1);
      expect(portfolioModel.getColumnCount()).toBe(16);
      expect(portfolioModel.getValueAt(13, 0).equals(Money.fromNumber(1988))).toBe(true);
      expect(portfolioModel.getValueAt(14, 0).equals(Money.fromNumber(1111))).toBe(true);
      expect(portfolioModel.getValueAt(15, 0).equals(Money.fromNumber(123))).toBe(true);
      done();
    });
    let mockData = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    portfolioModel.onDataReceived([mockData]);
  });

  it('Row add - three rows, two accounts', function(done) {
    let mockData1 = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    let mockData2 = createMockData(2, 1, 'account2', 245, 248, 5784, 124, 124, 'XTSE', 'TD', 311, 1, 34, 3841);
    portfolioModel.onDataReceived([mockData1, mockData2]);

    portfolioModel.addDataChangeListener(function(dataChangeType, payload) {
      expect(dataChangeType).toBe(DataChangeType.ADD);
      expect(payload).toBe(2);
      expect(portfolioModel.getRowCount()).toBe(3);
      expect(portfolioModel.getColumnCount()).toBe(16);
      expect(portfolioModel.getValueAt(8, 2).equals(Money.fromNumber(614))).toBe(true);   // fees
      expect(portfolioModel.getValueAt(13, 2).equals(Money.fromNumber(7345))).toBe(true); // acc total p&l
      expect(portfolioModel.getValueAt(14, 2).equals(Money.fromNumber(837))).toBe(true);  // acc unrealized p&l
      expect(portfolioModel.getValueAt(15, 2).equals(Money.fromNumber(737))).toBe(true);  // acc fees
      done();
    });

    let mockData3 = createMockData(1, 1, 'account1', 614, 6245, 6163, 124, 124, 'XTSE', 'MSFT', 25, 3, 67, -274);
    portfolioModel.onDataReceived([mockData3]);
  });

  it('Row remove - remove last remaining row of an account', function(done) {
    let mockData1 = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    let mockData2 = createMockData(2, 1, 'account2', 245, 248, 5784, 124, 124, 'XTSE', 'TD', 311, 1, 34, 3841);
    let mockData3 = createMockData(1, 1, 'account1', 614, 6245, 6163, 124, 124, 'XTSE', 'MSFT', 25, 3, 67, -274);
    portfolioModel.onDataReceived([mockData1, mockData2, mockData3]);

    portfolioModel.addDataChangeListener(function(dataChangeType, payload) {
      expect(dataChangeType).toBe(DataChangeType.REMOVE);
      expect(payload.index).toBe(1);
      expect(portfolioModel.getRowCount()).toBe(2);
      expect(portfolioModel.getColumnCount()).toBe(16);
      expect(portfolioModel.getValueAt(8, 1).equals(Money.fromNumber(614))).toBe(true);   // fees
      expect(portfolioModel.getValueAt(13, 1).equals(Money.fromNumber(7345))).toBe(true); // acc total p&l
      expect(portfolioModel.getValueAt(14, 1).equals(Money.fromNumber(837))).toBe(true);  // acc unrealized p&l
      expect(portfolioModel.getValueAt(15, 1).equals(Money.fromNumber(737))).toBe(true);  // acc fees
      expect(portfolioModel.accountTotals.count()).toBe(1);
      done();
    });

    let mockData4 = createMockData(2, 1, 'account2', 245, 248, 5784, 124, 124, 'XTSE', 'TD', 311, 0, 34, 3841);
    portfolioModel.onDataReceived([mockData4]);
  });

  it('Row remove - remove one of the rows of an account', function(done) {
    let mockData1 = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    let mockData2 = createMockData(2, 1, 'account2', 245, 248, 5784, 124, 124, 'XTSE', 'TD', 311, 1, 34, 3841);
    let mockData3 = createMockData(1, 1, 'account1', 614, 6245, 6163, 124, 124, 'XTSE', 'MSFT', 25, 3, 67, -274);
    portfolioModel.onDataReceived([mockData1, mockData2, mockData3]);

    portfolioModel.addDataChangeListener(function(dataChangeType, payload) {
      expect(dataChangeType).toBe(DataChangeType.REMOVE);
      expect(payload.index).toBe(0);
      expect(portfolioModel.getRowCount()).toBe(2);
      expect(portfolioModel.getColumnCount()).toBe(16);
      expect(portfolioModel.getValueAt(8, 1).equals(Money.fromNumber(614))).toBe(true);     // fees
      expect(portfolioModel.getValueAt(13, 1).equals(Money.fromNumber(5357))).toBe(true);   // acc total p&l
      expect(portfolioModel.getValueAt(14, 1).equals(Money.fromNumber(-274))).toBe(true);   // acc unrealized p&l
      expect(portfolioModel.getValueAt(15, 1).equals(Money.fromNumber(614))).toBe(true);    // acc fees
      expect(portfolioModel.accountTotals.count()).toBe(2);
      done();
    });

    let mockData4 = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 0, 100, 1111);
    portfolioModel.onDataReceived([mockData4]);
  });

  it('Row update', function(done) {
    let mockData1 = createMockData(1, 1, 'account1', 123, 1000, 100000, 124, 124, 'XTSE', 'RY', 100, 1, 100, 1111);
    let mockData2 = createMockData(2, 1, 'account2', 245, 248, 5784, 124, 124, 'XTSE', 'TD', 311, 1, 34, 3841);
    let mockData3 = createMockData(1, 1, 'account1', 614, 6245, 6163, 124, 124, 'XTSE', 'MSFT', 25, 3, 67, -274);
    portfolioModel.onDataReceived([mockData1, mockData2, mockData3]);

    portfolioModel.addDataChangeListener(function(dataChangeType, payload) {
      expect(dataChangeType).toBe(DataChangeType.UPDATE);
      expect(payload.index).toBe(2);
      expect(portfolioModel.getRowCount()).toBe(3);
      expect(portfolioModel.getColumnCount()).toBe(16);
      expect(portfolioModel.getValueAt(8, 2).equals(Money.fromNumber(731))).toBe(true);     // fees
      expect(portfolioModel.getValueAt(13, 2).equals(Money.fromNumber(6456))).toBe(true);   // acc total p&l
      expect(portfolioModel.getValueAt(14, 2).equals(Money.fromNumber(599))).toBe(true);    // acc unrealized p&l
      expect(portfolioModel.getValueAt(15, 2).equals(Money.fromNumber(854))).toBe(true);    // acc fees
      expect(portfolioModel.accountTotals.count()).toBe(2);
      done();
    });

    let mockData4 = createMockData(1, 1, 'account1', 731, 5711, 8120, 124, 124, 'XTSE', 'MSFT', 67, 11, 244, -512);
    portfolioModel.onDataReceived([mockData4]);
  });
});

function createMockData(
  accountId,
  accountType,
  accountName,
  fees,
  grossPnL,
  costBasis,
  currencyNum,
  securityCountry,
  securityMarket,
  securitySymbol,
  quantity,
  transactionCount,
  volume,
  unrealizedPnL) {
  let security = Security.fromData({
    country: securityCountry,
    market: securityMarket,
    symbol: securitySymbol
  });
  security.toString = function() {
    return securityMarket + '.' + securitySymbol;
  };

  return {
    account: DirectoryEntry.fromData({
      id: accountId,
      type: accountType,
      name: accountName
    }),
    inventory: {
      fees: Money.fromNumber(fees),
      gross_profit_and_loss: Money.fromNumber(grossPnL),
      position: {
        cost_basis: Money.fromNumber(costBasis),
        key: {
          currency: CurrencyId.fromData(currencyNum),
          index: security
        },
        quantity: quantity
      },
      transaction_count: transactionCount,
      volume: volume
    },
    unrealized_profit_and_loss: {
      is_initialized: true,
      value: Money.fromNumber(unrealizedPnL)
    }
  };
}

class MockRiskServiceClient {
  subscribePortfolio(listener) {
    return new Promise((resolve, reject) => {
      resolve('mock-sub-id');
    });
  }
}
