import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Stores the data needed to display the CompliancePage. */
export class ComplianceModel {

  /**
   * Constructs a ComplianceModel.
   * @param directoryEntry The account/group represented by this model.
   * @param schemas The list of existing schemas.
   * @param entries The list of applicable compliance rules.
   * @param currencyDatabase The database of available currencies, uses the
   *        default database if one is not explicitly provided.
   */
  constructor(directoryEntry: Beam.DirectoryEntry,
      schemas: Nexus.ComplianceRuleSchema[],
      entries: Nexus.ComplianceRuleEntry[],
      currencyDatabase?: Nexus.CurrencyDatabase) {
    this._directoryEntry = directoryEntry;
    this._schemas = schemas.slice();
    this._entries = entries.slice();
    this.newId = 1;
    for(const entry of this._entries) {
      this.newId = Math.max(this.newId, entry.id + 1);
    }
    this.nextId = this.newId;
    if(currencyDatabase) {
      this._currencyDatabase = currencyDatabase;
    } else {
      this._currencyDatabase = Nexus.buildDefaultCurrencyDatabase();
    }
  }

  /** Returns the list of available schemas. */
  public get schemas(): Nexus.ComplianceRuleSchema[] {
    return this._schemas.slice();
  }

  /** Returns the list of applicable compliance rules. */
  public get entries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.slice();
  }

  /** Returns the list of newly added rules. */
  public get newEntries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.filter(entry => entry.id >= this.newId);
  }

  /** Returns the list of updated rules. */
  public get updatedEntries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.filter(entry => entry.id < this.newId);
  }

  /** Returns the list of deleted rules. */
  public get deletedEntries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.filter(entry => {
      return entry.id < this.newId &&
        entry.state === Nexus.ComplianceRuleEntry.State.DELETED;
    });
  }

  /**
   * Adds a new rule based on a schema.
   * @param schema The schema used to base the new rule off of.
   */
  public add(schema: Nexus.ComplianceRuleSchema): void {
    const entry = new Nexus.ComplianceRuleEntry(this.nextId,
      this._directoryEntry, Nexus.ComplianceRuleEntry.State.ACTIVE, schema);
    ++this.nextId;
    this._entries.push(entry);
  }

  /**
   * Updates an existing rule.
   * @param entry The updated entry.
   */
  public update(entry: Nexus.ComplianceRuleEntry): void {
    for(let i = 0; i != this._entries.length; ++i) {
      if(this._entries[i].id === entry.id) {
        this._entries[i] = entry;
        break;
      }
    }
  }

  /** Returns the database of available currencies. */
  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this._currencyDatabase;
  }

  public clone(): ComplianceModel {
    return new ComplianceModel(this._directoryEntry, this._schemas,
      this._entries, this._currencyDatabase);
  }

  private _directoryEntry: Beam.DirectoryEntry;
  private _schemas: Nexus.ComplianceRuleSchema[];
  private _entries: Nexus.ComplianceRuleEntry[];
  private newId: number;
  private nextId: number;
  private _currencyDatabase: Nexus.CurrencyDatabase;
}
