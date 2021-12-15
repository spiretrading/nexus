import * as Beam from 'beam';
import { Currency, Money } from '..';
import { EntitlementKey } from './entitlement_key';
import { MarketDataTypeSet } from './market_data_type';

/** Stores the database of all market data entitlements. */
export class EntitlementDatabase {

  /** Parses a database from a JSON object. */
  public static fromJson(value: any): EntitlementDatabase {
    const database = new EntitlementDatabase();
    database._entries = Beam.arrayFromJson(EntitlementDatabase.Entry,
      value.entries);
    return database;
  }

  /** Constructs an empty database. */
  constructor() {
    this._entries = [];
  }

  /**
   * Returns the entry belonging to a group.
   * @param group - The group to search for.
   * @return The entry belonging to the specified group.
   */
  public fromGroup(group: Beam.DirectoryEntry): EntitlementDatabase.Entry {
    for(let entry of this._entries) {
      if(entry.group.equals(group)) {
        return entry;
      }
    }
    return EntitlementDatabase.Entry.NONE;
  }

  /**
   * Adds an entry.
   * @param entry - The entry to add.
   */
  public add(entry: EntitlementDatabase.Entry): void {
    if(this.fromGroup(entry.group).group.equals(Beam.DirectoryEntry.INVALID)) {
      this._entries.push(entry);
    }
  }

  /**
   * Removes an entry.
   * @param entry - The entitlement entry to remove.
   */
  public delete(entry: EntitlementDatabase.Entry): void {
    for(let i = 0; i < this._entries.length; ++i) {
      if(this._entries[i].group.equals(entry.group)) {
        this._entries.splice(i, 1);
        break;
      }
    }
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return Beam.arrayToJson(this._entries);
  }

  [Symbol.iterator]() {
    return this._entries[Symbol.iterator]();
  }

  private _entries: EntitlementDatabase.Entry[];
}

export module EntitlementDatabase {

  /** Stores a single entitlement entry. */
  export class Entry {

    /** Represents an invalid entry. */
    public static readonly NONE = new Entry('', Money.ZERO,
      new Currency(), Beam.DirectoryEntry.INVALID,
      new Beam.Map<EntitlementKey, MarketDataTypeSet>());

    /** Constructs an entry from a JSON object. */
    public static fromJson(value: any) {
      return new Entry(value.name, Money.fromJson(value.price),
        Currency.fromJson(value.currency),
        Beam.DirectoryEntry.fromJson(value.group_entry),
        Beam.Map.fromJson<EntitlementKey, MarketDataTypeSet>(EntitlementKey,
          MarketDataTypeSet, value.applicability));
    }

    /**
     * Constructs an entry.
     * @param name - The display name.
     * @param price - The entitlement's price.
     * @param currency - The currency the entitlement is priced in.
     * @param group - The DirectoryEntry representing the entitlement's group.
     * @param applicability - Specifies the types of market data granted by
     *        this entitlement.
     */
    constructor(name: string, price: Money, currency: Currency,
        group: Beam.DirectoryEntry,
        applicability: Beam.Map<EntitlementKey, MarketDataTypeSet>) {
      this._name = name;
      this._price = price;
      this._currency = currency;
      this._group = group;
      this._applicability = applicability;
    }

    /** Returns the display name. */
    public get name(): string {
      return this._name;
    }

    /** Returns the entitlement's price. */
    public get price(): Money {
      return this._price;
    }

    /** Returns the currency the entitlement is priced in. */
    public get currency(): Currency {
      return this._currency;
    }

    /** Returns the DirectoryEntry representing this entitlement's group. */
    public get group(): Beam.DirectoryEntry {
      return this._group;
    }

    /** Returns the types of market data granted by this entitlement. */
    public get applicability(): Beam.Map<EntitlementKey, MarketDataTypeSet> {
      return this._applicability;
    }

    /** Tests two entries for equality. */
    public equals(other: Entry): boolean {
      return other && this._group.equals(other._group);
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        name: this._name,
        price: this._price,
        currency: this._currency.toJson(),
        group_entry: this.group.toJson(),
        applicability: this._applicability.toJson()
      };
    }

    private _name: string;
    private _price: Money;
    private _currency: Currency;
    private _group: Beam.DirectoryEntry;
    private _applicability: Beam.Map<EntitlementKey, MarketDataTypeSet>;
  }
}
