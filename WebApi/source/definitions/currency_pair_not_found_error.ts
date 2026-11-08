/** Signals that no exchange rate exists for a currency pair. */
export class CurrencyPairNotFoundError extends Error {

  /** Constructs a CurrencyPairNotFoundError. */
  constructor() {
    super('Currency pair not found.');
  }
}
