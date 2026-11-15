import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';
import { Ticker } from '../definitions/ticker';

/** Used as a key into an account's inventory. */
export class RiskPortfolioKey {

  /** Parses a RiskPortfolioKey from JSON. */
  public static fromJson(value: any): RiskPortfolioKey {
    return new RiskPortfolioKey(Beam.DirectoryEntry.fromJson(value.account),
      Ticker.fromJson(value.ticker));
  }

  /** The portfolio's account. */
  public account: Beam.DirectoryEntry;

  /** The ticker being indexed. */
  public ticker: Ticker;

  /** Constructs an empty key. */
  constructor();

  /**
   * Constructs a RiskPortfolioKey.
   * @param account - The portfolio's account.
   * @param ticker - The ticker being indexed.
   */
  constructor(account: Beam.DirectoryEntry, ticker: Ticker);
  constructor(account?: Beam.DirectoryEntry, ticker?: Ticker) {
    this.account = account ?? Beam.DirectoryEntry.INVALID;
    this.ticker = ticker ?? Ticker.NONE;
  }

  /** Tests if two keys are equal. */
  public equals(other: RiskPortfolioKey): boolean {
    return other && this.account.equals(other.account) &&
      this.ticker.equals(other.ticker);
  }

  /** Returns a hash of this key. */
  public hash(): number {
    let seed = 0;
    seed = Beam.hashCombine(seed, this.account.id);
    return Beam.hashCombine(seed, this.ticker.hash());
  }

  /** Converts this key to JSON. */
  public toJson(): any {
    return {
      account: this.account.toJson(),
      ticker: this.ticker.toJson()
    };
  }
}

/** Stores an inventory update. */
export class RiskInventoryEntry {

  /** Parses a RiskInventoryEntry from JSON. */
  public static fromJson(value: any): RiskInventoryEntry {
    return new RiskInventoryEntry(RiskPortfolioKey.fromJson(value.key),
      Inventory.fromJson(value.value));
  }

  /** The account and ticker being updated. */
  public key: RiskPortfolioKey;

  /** The updated inventory. */
  public value: Inventory;

  /** Constructs an empty entry. */
  constructor();

  /**
   * Constructs a RiskInventoryEntry.
   * @param key - The account and ticker being updated.
   * @param value - The updated inventory.
   */
  constructor(key: RiskPortfolioKey, value: Inventory);
  constructor(key?: RiskPortfolioKey, value?: Inventory) {
    this.key = key ?? new RiskPortfolioKey();
    this.value = value ?? new Inventory();
  }

  /** Tests if two inventory entries are equal. */
  public equals(other: RiskInventoryEntry): boolean {
    return other && this.key.equals(other.key) &&
      this.value.equals(other.value);
  }

  /** Converts this entry to JSON. */
  public toJson(): any {
    return {
      key: this.key.toJson(),
      value: this.value.toJson()
    };
  }
}
