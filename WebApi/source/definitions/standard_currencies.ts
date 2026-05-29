import { Currency, CurrencyDatabase } from './currency';

/** Builds a CurrencyDatabase containing a set of common currencies. */
export function buildCurrencyDatabase(): CurrencyDatabase {
  const database = new CurrencyDatabase();
  database.add(new CurrencyDatabase.Entry(
    new Currency(36), 'AUD', 'Australian Dollar', '$'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(124), 'CAD', 'Canadian Dollar', '$'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(978), 'EUR', 'Euro', '€'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(826), 'GBP', 'Pound Sterling', '£'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(344), 'HKD', 'Hong Kong Dollar', '$'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(392), 'JPY', 'Yen', '¥'));
  database.add(new CurrencyDatabase.Entry(
    new Currency(840), 'USD', 'US Dollar', '$'));
  return database;
}

export namespace Currencies {
  export const AUD = new Currency(36);
  export const CAD = new Currency(124);
  export const EUR = new Currency(978);
  export const GBP = new Currency(826);
  export const HKD = new Currency(344);
  export const JPY = new Currency(392);
  export const USD = new Currency(840);
}
