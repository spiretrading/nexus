import PortfolioModel from '../../../../webapp/utils/table-models/portfolio-model';
import {
  DirectoryEntry,
  Money,
  CurrencyId,
  Security
} from 'spire-client';

describe('ViewModel', function() {
  beforeAll(function() {
    this.portfolioModel = new PortfolioModel(new MockRiskServiceClient());
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
          index: Security.fromData({
            country: securityCountry,
            market: securityMarket,
            symbol: securitySymbol
          })
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
