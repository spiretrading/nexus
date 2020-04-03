import { CountryCode, CountryDatabase } from './country';

/** Builds a CountryDatabase containing a set of common countries. */
export function buildDefaultCountryDatabase(): CountryDatabase {
  const database = new CountryDatabase();
  database.add(new CountryDatabase.Entry(new CountryCode(36), 'Australia', 'AU',
    'AUS'));
  database.add(new CountryDatabase.Entry(new CountryCode(76), 'Brazil', 'BR',
    'BRA'));
  database.add(new CountryDatabase.Entry(new CountryCode(124), 'Canada', 'CA',
    'CAN'));
  database.add(new CountryDatabase.Entry(new CountryCode(156), 'China', 'CN',
    'CHN'));
  database.add(new CountryDatabase.Entry(new CountryCode(840), 'United States',
    'US', 'USA'));
  return database;
}

export namespace DefaultCountries {
  export const AU = new CountryCode(36);
  export const BR = new CountryCode(76);
  export const CA = new CountryCode(124);
  export const CN = new CountryCode(156);
  export const US = new CountryCode(840);
}
