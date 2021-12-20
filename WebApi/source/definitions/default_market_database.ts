import { DefaultCountries } from './default_country_database';
import { DefaultCurrencies } from './default_currency_database';
import { MarketCode, MarketDatabase } from './market';

/** Builds a MarketDatabase containing a set of common marketplaces. */
export function buildDefaultMarketDatabase(): MarketDatabase {
  const database = new MarketDatabase();
  database.add(new MarketDatabase.Entry(new MarketCode('XASX'),
    DefaultCountries.AU, 'Australian_Eastern_Standard_Time',
    DefaultCurrencies.AUD, 100, 'Australian Stock Market', 'ASX'));
  database.add(new MarketDatabase.Entry(new MarketCode('CHIA'),
    DefaultCountries.AU, 'Australian_Eastern_Standard_Time',
    DefaultCurrencies.AUD, 100, 'CHI-X Australia', 'CXA'));
  database.add(new MarketDatabase.Entry(new MarketCode('XASE'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'NYSE MKT LLC', 'ASEX'));
  database.add(new MarketDatabase.Entry(new MarketCode('ARCX'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'NYSE ARCA', 'ARCX'));
  database.add(new MarketDatabase.Entry(new MarketCode('BATS'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'BATS Exchange', 'BATS'));
  database.add(new MarketDatabase.Entry(new MarketCode('BATY'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'BATS-Y Exchange', 'BATY'));
  database.add(new MarketDatabase.Entry(new MarketCode('XBOS'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'NASDAQ OMX BX', 'BOSX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XCBO'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'Chicago Board of Options Exchange', 'CBOE'));
  database.add(new MarketDatabase.Entry(new MarketCode('XCIS'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'National Stock Exchange', 'NSEX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XADF'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'FINRA Alternative Display Facility', 'ADFX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XISX'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'International Securities Exchange', 'ISE'));
  database.add(new MarketDatabase.Entry(new MarketCode('EDGA'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'EDGA Exchange', 'EDGA'));
  database.add(new MarketDatabase.Entry(new MarketCode('EDGX'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'EDGX Exchange', 'EDGX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XNYS'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100, 'NYSE',
    'NYSE'));
  database.add(new MarketDatabase.Entry(new MarketCode('XNAS'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100, 'NASDAQ',
    'NSDQ'));
  database.add(new MarketDatabase.Entry(new MarketCode('XPHL'),
    DefaultCountries.US, 'Eastern_Time', DefaultCurrencies.USD, 100,
    'NASDAQ OMX PHLX', 'PHLX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XATS'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Alpha Exchange', 'XATS'));
  database.add(new MarketDatabase.Entry(new MarketCode('CHIC'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'CHI-X Canada', 'CHIC'));
  database.add(new MarketDatabase.Entry(new MarketCode('XCNQ'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Canadian Securities Exchange', 'CSE'));
  database.add(new MarketDatabase.Entry(new MarketCode('XCX2'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'CX2', 'CX2'));
  database.add(new MarketDatabase.Entry(new MarketCode('LYNX'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'LYNX', 'LYNX'));
  database.add(new MarketDatabase.Entry(new MarketCode('MATN'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'MATCH Now', 'MATN'));
  database.add(new MarketDatabase.Entry(new MarketCode('NEOE'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Aequitas NEO Exchange', 'NEOE'));
  database.add(new MarketDatabase.Entry(new MarketCode('OMGA'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Omega', 'OMGA'));
  database.add(new MarketDatabase.Entry(new MarketCode('PURE'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Pure Trading', 'PURE'));
  database.add(new MarketDatabase.Entry(new MarketCode('XTSE'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'Toronto Stock Exchange', 'TSX'));
  database.add(new MarketDatabase.Entry(new MarketCode('XTSX'),
    DefaultCountries.CA, 'Eastern_Time', DefaultCurrencies.CAD, 100,
    'TSX Venture Exchange', 'TSXV'));
  return database;
}

export const defaultMarketDatabase = buildDefaultMarketDatabase();

export namespace DefaultMarkets {
  export const ASX = new MarketCode('XASX');
  export const CXA = new MarketCode('CHIA');
  export const ASEX = new MarketCode('XASE');
  export const ARCX = new MarketCode('ARCX');
  export const BATS = new MarketCode('BATS');
  export const BATY = new MarketCode('BATY');
  export const BOSX = new MarketCode('BOSX');
  export const CBOE = new MarketCode('XCBO');
  export const CSE = new MarketCode('XCNQ');
  export const NSEX = new MarketCode('XCIS');
  export const ADFX = new MarketCode('XADF');
  export const ISE = new MarketCode('XISX');
  export const EDGA = new MarketCode('EDGA');
  export const EDGX = new MarketCode('EDGX');
  export const PHLX = new MarketCode('XPHL');
  export const CHIC = new MarketCode('CHIC');
  export const LYNX = new MarketCode('LYNX');
  export const NASDAQ = new MarketCode('XNAS');
  export const NYSE = new MarketCode('XNYS');
  export const MATN = new MarketCode('MATN');
  export const NEOE = new MarketCode('NEOE');
  export const OMGA = new MarketCode('OMGA');
  export const PURE = new MarketCode('PURE');
  export const TSX = new MarketCode('XTSE');
  export const TSXV = new MarketCode('XTSX');
  export const XATS = new MarketCode('XATS');
  export const XCX2 = new MarketCode('XCX2');
}
