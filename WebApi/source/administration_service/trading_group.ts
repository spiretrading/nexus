import * as Beam from 'beam';

/** Represents a group of traders and their managers. */
export class TradingGroup {

  /** Constructs a TradingGroup from a JSON object. */
  public static fromJson(value: any): TradingGroup {
    return new TradingGroup(Beam.DirectoryEntry.fromJson(value.entry),
      Beam.DirectoryEntry.fromJson(value.managers_directory),
      Beam.arrayFromJson(Beam.DirectoryEntry, value.managers),
      Beam.DirectoryEntry.fromJson(value.traders_directory),
      Beam.arrayFromJson(Beam.DirectoryEntry, value.traders));
  }

  /**
   * Constructs a TradingGroup.
   * @param entry The DirectoryEntry containing the TradingGroup.
   * @param managersDirectory The DirectoryEntry containing the managers.
   * @param managers The list of managers in the group.
   * @param tradersDirectory The DirectoryEntry containing the traders.
   * @param traders The list of traders in the group.
   */
  constructor(entry: Beam.DirectoryEntry,
      managersDirectory: Beam.DirectoryEntry, managers: Beam.DirectoryEntry[],
      tradersDirectory: Beam.DirectoryEntry, traders: Beam.DirectoryEntry[]) {
    this._entry = entry;
    this._managersDirectory = managersDirectory;
    this._managers = managers.slice();
    this._tradersDirectory = tradersDirectory;
    this._traders = traders;
  }

  /** Returns the DirectoryEntry containing the TradingGroup. */
  public get entry(): Beam.DirectoryEntry {
    return this._entry;
  }

  /** Returns the DirectoryEntry containing the managers. */
  public get managersDirectory(): Beam.DirectoryEntry {
    return this._managersDirectory;
  }

  /** Returns the list of managers in the group. */
  public get managers(): Beam.DirectoryEntry[] {
    return this._managers.slice();
  }

  /** Returns the DirectoryEntry containing the traders. */
  public get tradersDirectory(): Beam.DirectoryEntry {
    return this._tradersDirectory;
  }

  /** Returns the list of traders in the group. */
  public get traders(): Beam.DirectoryEntry[] {
    return this._traders.slice();
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      entry: this._entry.toJson(),
      managers_directory: this._managersDirectory.toJson(),
      managers: Beam.arrayToJson(this._managers),
      traders_directory: this._tradersDirectory.toJson(),
      traders: Beam.arrayToJson(this._traders)
    };
  }

  private _entry: Beam.DirectoryEntry;
  private _managersDirectory: Beam.DirectoryEntry;
  private _managers: Beam.DirectoryEntry[];
  private _tradersDirectory: Beam.DirectoryEntry;
  private _traders: Beam.DirectoryEntry[];
}
