import { Countries } from './standard_countries';
import { Currencies } from './standard_currencies';
import { Venue, VenueDatabase } from './venue';

/** Builds a VenueDatabase containing a set of common venues. */
export function buildVenueDatabase(): VenueDatabase {
  const database = new VenueDatabase();
  database.add(new VenueDatabase.Entry(new Venue('XASX'), Countries.AU, 'ASX',
    'Australia/Sydney', Currencies.AUD,
    'Australian Securities Exchange', 'ASX'));
  database.add(new VenueDatabase.Entry(new Venue('CHIA'), Countries.AU, 'CXA',
    'Australia/Sydney', Currencies.AUD, 'TMX Australia Exchange', 'CXA'));
  database.add(new VenueDatabase.Entry(new Venue('XATS'), Countries.CA, 'ALP',
    'America/Toronto', Currencies.CAD,
    'TSX Alpha Exchange - Alpha Classic', 'XATS'));
  database.add(new VenueDatabase.Entry(new Venue('CHIC'), Countries.CA, 'CHI',
    'America/Toronto', Currencies.CAD, 'Nasdaq Canada - CXC', 'CHIC'));
  database.add(new VenueDatabase.Entry(new Venue('XCNQ'), Countries.CA, 'CNQ',
    'America/Toronto', Currencies.CAD, 'Canadian Securities Exchange', 'CSE'));
  database.add(new VenueDatabase.Entry(new Venue('CSE2'), Countries.CA, 'CS2',
    'America/Toronto', Currencies.CAD, 'Canadian Securities Exchange - CSE2',
    'CSE2'));
  database.add(new VenueDatabase.Entry(new Venue('XCXD'), Countries.CA, 'CHD',
    'America/Toronto', Currencies.CAD, 'Nasdaq Canada - CXD', 'CXD'));
  database.add(new VenueDatabase.Entry(new Venue('XCX2'), Countries.CA, 'CHT',
    'America/Toronto', Currencies.CAD, 'Nasdaq Canada - CX2', 'CX2'));
  database.add(new VenueDatabase.Entry(new Venue('LYNX'), Countries.CA, 'LYX',
    'America/Toronto', Currencies.CAD, 'Lynx ATS', 'LYNX'));
  database.add(new VenueDatabase.Entry(new Venue('MATN'), Countries.CA, 'TCM',
    'America/Toronto', Currencies.CAD, 'Cboe Canada - MATCHNow', 'MATN'));
  database.add(new VenueDatabase.Entry(new Venue('NEOE'), Countries.CA, 'AQL',
    'America/Toronto', Currencies.CAD, 'Cboe Canada - NEO-L', 'NEOL'));
  database.add(new VenueDatabase.Entry(new Venue('OMGA'), Countries.CA, 'OMG',
    'America/Toronto', Currencies.CAD, 'Omega ATS', 'OMGA'));
  database.add(new VenueDatabase.Entry(new Venue('PURE'), Countries.CA, 'PUR',
    'America/Toronto', Currencies.CAD,
    'Canadian Securities Exchange - PURE', 'PURE'));
  database.add(new VenueDatabase.Entry(new Venue('XTSE'), Countries.CA, 'TSE',
    'America/Toronto', Currencies.CAD, 'Toronto Stock Exchange', 'TSX'));
  database.add(new VenueDatabase.Entry(new Venue('XTSX'), Countries.CA, 'CDX',
    'America/Toronto', Currencies.CAD, 'TSX Venture Exchange', 'TSXV'));
  database.add(new VenueDatabase.Entry(new Venue('XATX'), Countries.CA, 'ALX',
    'America/Toronto', Currencies.CAD, 'Alpha-X', 'ALX'));
  database.add(new VenueDatabase.Entry(new Venue('ADRK'), Countries.CA, 'ALD',
    'America/Toronto', Currencies.CAD, 'Alpha DRK', 'ALD'));
  database.add(new VenueDatabase.Entry(new Venue('XICX'), Countries.CA, 'ICX',
    'America/Toronto', Currencies.CAD, 'Instinet Canada Cross', 'ICX'));
  database.add(new VenueDatabase.Entry(new Venue('LICA'), Countries.CA, 'LIQ',
    'America/Toronto', Currencies.CAD, 'Liquidnet Canada', 'LIQ'));
  database.add(new VenueDatabase.Entry(new Venue('NEON'), Countries.CA, 'AQN',
    'America/Toronto', Currencies.CAD, 'Cboe Canada - NEO-N', 'NEON'));
  return database;
}

export const venueDatabase = buildVenueDatabase();

export namespace Venues {
  export const ALD = new Venue('ADRK');
  export const ALX = new Venue('XATX');
  export const ASX = new Venue('XASX');
  export const CXA = new Venue('CHIA');
  export const CSE = new Venue('XCNQ');
  export const CSE2 = new Venue('CSE2');
  export const CHIC = new Venue('CHIC');
  export const CXD = new Venue('XCXD');
  export const ICX = new Venue('XICX');
  export const LIQ = new Venue('LICA');
  export const LYNX = new Venue('LYNX');
  export const MATN = new Venue('MATN');
  export const NEOE = new Venue('NEOE');
  export const NEON = new Venue('NEON');
  export const OMGA = new Venue('OMGA');
  export const PURE = new Venue('PURE');
  export const TSX = new Venue('XTSE');
  export const TSXV = new Venue('XTSX');
  export const XATS = new Venue('XATS');
  export const XCX2 = new Venue('XCX2');
}
