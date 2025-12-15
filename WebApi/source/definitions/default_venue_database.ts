import { DefaultCountries } from './default_country_database';
import { DefaultCurrencies } from './default_currency_database';
import { Venue, VenueDatabase } from './venue';

/** Builds a VenueDatabase containing a set of common venues. */
export function buildDefaultVenueDatabase(): VenueDatabase {
  const database = new VenueDatabase();
  database.add(new VenueDatabase.Entry(new Venue('XASX'),
    DefaultCountries.AU, '', 'Australian_Eastern_Standard_Time',
    DefaultCurrencies.AUD, 'Australian Stock Market', 'ASX'));
  database.add(new VenueDatabase.Entry(new Venue('CHIA'),
    DefaultCountries.AU, '', 'Australian_Eastern_Standard_Time',
    DefaultCurrencies.AUD, 'CHI-X Australia', 'CXA'));
  database.add(new VenueDatabase.Entry(new Venue('XASE'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NYSE MKT LLC', 'ASEX'));
  database.add(new VenueDatabase.Entry(new Venue('ARCX'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NYSE ARCA', 'ARCX'));
  database.add(new VenueDatabase.Entry(new Venue('BATS'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'BATS Exchange', 'BATS'));
  database.add(new VenueDatabase.Entry(new Venue('BATY'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'BATS-Y Exchange', 'BATY'));
  database.add(new VenueDatabase.Entry(new Venue('XBOS'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NASDAQ OMX BX', 'BOSX'));
  database.add(new VenueDatabase.Entry(new Venue('XCBO'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'Chicago Board of Options Exchange', 'CBOE'));
  database.add(new VenueDatabase.Entry(new Venue('XCIS'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'National Stock Exchange', 'NSEX'));
  database.add(new VenueDatabase.Entry(new Venue('XADF'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'FINRA Alternative Display Facility', 'ADFX'));
  database.add(new VenueDatabase.Entry(new Venue('XISX'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'International Securities Exchange', 'ISE'));
  database.add(new VenueDatabase.Entry(new Venue('EDGA'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'EDGA Exchange', 'EDGA'));
  database.add(new VenueDatabase.Entry(new Venue('EDGX'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'EDGX Exchange', 'EDGX'));
  database.add(new VenueDatabase.Entry(new Venue('XNYS'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NYSE', 'NYSE'));
  database.add(new VenueDatabase.Entry(new Venue('XNAS'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NASDAQ', 'NSDQ'));
  database.add(new VenueDatabase.Entry(new Venue('XPHL'),
    DefaultCountries.US, '', 'Eastern_Time', DefaultCurrencies.USD,
    'NASDAQ OMX PHLX', 'PHLX'));
  database.add(new VenueDatabase.Entry(new Venue('XATS'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Alpha Exchange', 'XATS'));
  database.add(new VenueDatabase.Entry(new Venue('CHIC'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'CHI-X Canada', 'CHIC'));
  database.add(new VenueDatabase.Entry(new Venue('XCNQ'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Canadian Securities Exchange', 'CSE'));
  database.add(new VenueDatabase.Entry(new Venue('XCX2'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'CX2', 'CX2'));
  database.add(new VenueDatabase.Entry(new Venue('LYNX'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'LYNX', 'LYNX'));
  database.add(new VenueDatabase.Entry(new Venue('MATN'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'MATCH Now', 'MATN'));
  database.add(new VenueDatabase.Entry(new Venue('NEOE'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Aequitas NEO Exchange', 'NEOE'));
  database.add(new VenueDatabase.Entry(new Venue('OMGA'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Omega', 'OMGA'));
  database.add(new VenueDatabase.Entry(new Venue('PURE'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Pure Trading', 'PURE'));
  database.add(new VenueDatabase.Entry(new Venue('XTSE'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'Toronto Stock Exchange', 'TSX'));
  database.add(new VenueDatabase.Entry(new Venue('XTSX'),
    DefaultCountries.CA, '', 'Eastern_Time', DefaultCurrencies.CAD,
    'TSX Venture Exchange', 'TSXV'));
  return database;
}

export const defaultVenueDatabase = buildDefaultVenueDatabase();

export namespace DefaultMarkets {
  export const ASX = new Venue('XASX');
  export const CXA = new Venue('CHIA');
  export const ASEX = new Venue('XASE');
  export const ARCX = new Venue('ARCX');
  export const BATS = new Venue('BATS');
  export const BATY = new Venue('BATY');
  export const BOSX = new Venue('BOSX');
  export const CBOE = new Venue('XCBO');
  export const CSE = new Venue('XCNQ');
  export const NSEX = new Venue('XCIS');
  export const ADFX = new Venue('XADF');
  export const ISE = new Venue('XISX');
  export const EDGA = new Venue('EDGA');
  export const EDGX = new Venue('EDGX');
  export const PHLX = new Venue('XPHL');
  export const CHIC = new Venue('CHIC');
  export const LYNX = new Venue('LYNX');
  export const NASDAQ = new Venue('XNAS');
  export const NYSE = new Venue('XNYS');
  export const MATN = new Venue('MATN');
  export const NEOE = new Venue('NEOE');
  export const OMGA = new Venue('OMGA');
  export const PURE = new Venue('PURE');
  export const TSX = new Venue('XTSE');
  export const TSXV = new Venue('XTSX');
  export const XATS = new Venue('XATS');
  export const XCX2 = new Venue('XCX2');
}
