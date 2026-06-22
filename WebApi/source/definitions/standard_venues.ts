import { Countries } from './standard_countries';
import { Currencies } from './standard_currencies';
import { Venue, VenueDatabase } from './venue';

/** Builds a VenueDatabase containing a set of common venues. */
export function buildVenueDatabase(): VenueDatabase {
  const database = new VenueDatabase();
  database.add(new VenueDatabase.Entry(new Venue('XASX'), Countries.AU, 'ASX',
    'Australia/Sydney', Currencies.AUD, 'Australian Stock Market', 'ASX'));
  database.add(new VenueDatabase.Entry(new Venue('CHIA'), Countries.AU, 'CXA',
    'Australia/Sydney', Currencies.AUD, 'CHI-X Australia', 'CXA'));
  database.add(new VenueDatabase.Entry(new Venue('XATS'), Countries.CA, 'ALP',
    'America/Toronto', Currencies.CAD, 'Alpha Exchange', 'XATS'));
  database.add(new VenueDatabase.Entry(new Venue('CHIC'), Countries.CA, 'CHI',
    'America/Toronto', Currencies.CAD, 'CHI-X Canada ATS', 'CHIC'));
  database.add(new VenueDatabase.Entry(new Venue('XCNQ'), Countries.CA, 'CNQ',
    'America/Toronto', Currencies.CAD, 'Canadian Securities Exchange', 'CSE'));
  database.add(new VenueDatabase.Entry(new Venue('CSE2'), Countries.CA, 'CS2',
    'America/Toronto', Currencies.CAD, 'Canadian Securities Exchange - CSE2',
    'CSE2'));
  database.add(new VenueDatabase.Entry(new Venue('XCXD'), Countries.CA, '',
    'America/Toronto', Currencies.CAD, 'NASDAQ CXD', 'CXD'));
  database.add(new VenueDatabase.Entry(new Venue('XCX2'), Countries.CA, 'CHT',
    'America/Toronto', Currencies.CAD, 'CX2', 'CX2'));
  database.add(new VenueDatabase.Entry(new Venue('LYNX'), Countries.CA, 'LYX',
    'America/Toronto', Currencies.CAD, 'Lynx ATS', 'LYNX'));
  database.add(new VenueDatabase.Entry(new Venue('MATN'), Countries.CA, 'TCM',
    'America/Toronto', Currencies.CAD, 'MATCH Now', 'MATN'));
  database.add(new VenueDatabase.Entry(new Venue('NEOE'), Countries.CA, 'AQL',
    'America/Toronto', Currencies.CAD, 'Aequitas NEO Exchange', 'NEOE'));
  database.add(new VenueDatabase.Entry(new Venue('OMGA'), Countries.CA, 'OMG',
    'America/Toronto', Currencies.CAD, 'Omega ATS', 'OMGA'));
  database.add(new VenueDatabase.Entry(new Venue('PURE'), Countries.CA, 'PUR',
    'America/Toronto', Currencies.CAD, 'Pure Trading', 'PURE'));
  database.add(new VenueDatabase.Entry(new Venue('XTSE'), Countries.CA, 'TSE',
    'America/Toronto', Currencies.CAD, 'Toronto Stock Exchange', 'TSX'));
  database.add(new VenueDatabase.Entry(new Venue('XTSX'), Countries.CA, 'CDX',
    'America/Toronto', Currencies.CAD, 'TSX Venture Exchange', 'TSXV'));
  return database;
}

export const venueDatabase = buildVenueDatabase();

export namespace Venues {
  export const ASX = new Venue('XASX');
  export const CXA = new Venue('CHIA');
  export const CSE = new Venue('XCNQ');
  export const CSE2 = new Venue('CSE2');
  export const CHIC = new Venue('CHIC');
  export const CXD = new Venue('XCXD');
  export const LYNX = new Venue('LYNX');
  export const MATN = new Venue('MATN');
  export const NEOE = new Venue('NEOE');
  export const OMGA = new Venue('OMGA');
  export const PURE = new Venue('PURE');
  export const TSX = new Venue('XTSE');
  export const TSXV = new Venue('XTSX');
  export const XATS = new Venue('XATS');
  export const XCX2 = new Venue('XCX2');
}
