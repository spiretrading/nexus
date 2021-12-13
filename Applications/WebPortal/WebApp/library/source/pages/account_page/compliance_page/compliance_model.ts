import * as Nexus from 'nexus';

/** Stores the data needed to display the CompliancePage. */
export class ComplianceModel {

  /**
   * Constructs a ComplianceModel.
   * @param roles The account's roles.
   * @param schemas The list of existing schemas.
   * @param entries The list of applicable compliance rules.
   * @param currencyDatabase The database of available currencies, uses the
   *        default database if one is not explicitly provided.
   */
  constructor(roles: Nexus.AccountRoles, schemas: Nexus.ComplianceRuleSchema[],
      entries: Nexus.ComplianceRuleEntry[],
      currencyDatabase?: Nexus.CurrencyDatabase) {
    this._roles = roles;
    this._schemas = schemas.slice();
    this._entries = entries.slice();
    if(currencyDatabase) {
      this._currencyDatabase = currencyDatabase;
    } else {
      this._currencyDatabase = Nexus.buildDefaultCurrencyDatabase();
    }
  }

  /** Returns the account's roles. */
  public get roles(): Nexus.AccountRoles {
    return this._roles;
  }

  /** Returns the list of available schemas. */
  public get schemas(): Nexus.ComplianceRuleSchema[] {
    return this._schemas.slice();
  }

  /** Returns the list of applicable compliance rules. */
  public get entries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.slice();
  }

  /**
   * Updates (or adds) a rule.
   * @param entry The updated entry, if an existing entry doesn't exist then
   *              one will be added.
   */
  public update(entry: Nexus.ComplianceRuleEntry) {
    if(entry.id === -1) {
      this._entries.push(entry);
    } else {
      for(let i = 0; i != this._entries.length; ++i) {
        if(this._entries[i].id === entry.id) {
          this._entries[i] = entry;
          break;
        }
      }
    }
  }

  /** Returns the database of available currencies. */
  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this._currencyDatabase;
  }

  public clone(): ComplianceModel {
    return new ComplianceModel(this._roles, this._schemas, this._entries,
      this._currencyDatabase);
  }

  private _roles: Nexus.AccountRoles;
  private _schemas: Nexus.ComplianceRuleSchema[];
  private _entries: Nexus.ComplianceRuleEntry[];
  private _currencyDatabase: Nexus.CurrencyDatabase;
}
