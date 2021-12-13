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

  public get roles(): Nexus.AccountRoles {
    return this._roles;
  }

  public get schemas(): Nexus.ComplianceRuleSchema[] {
    return this._schemas.slice();
  }

  public get entries(): Nexus.ComplianceRuleEntry[] {
    return this._entries.slice();
  }

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

  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this._currencyDatabase;
  }

  private _roles: Nexus.AccountRoles;
  private _currencyDatabase: Nexus.CurrencyDatabase;
  private _schemas: Nexus.ComplianceRuleSchema[];
  private _entries: Nexus.ComplianceRuleEntry[];
}
