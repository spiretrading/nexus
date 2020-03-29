import { Currency, CurrencyDatabase } from './currency';

/** Builds a CurrencyDatabase containing a set of common currencies. */
export function buildDefaultCurrencyDatabase(): CurrencyDatabase {
  const database = new CurrencyDatabase();
  database.add(new CurrencyDatabase.Entry(new Currency(36), 'AUD', '$'));
  database.add(new CurrencyDatabase.Entry(new Currency(124), 'CAD', '$'));
  database.add(new CurrencyDatabase.Entry(new Currency(978), 'EUR', '€'));
  database.add(new CurrencyDatabase.Entry(new Currency(840), 'USD', '$'));
  return database;
}

export namespace DefaultCurrencies {
  export const AUD = new Currency(36);
  export const CAD = new Currency(124);
  export const EUR = new Currency(978);
  export const USD = new Currency(840);
}
