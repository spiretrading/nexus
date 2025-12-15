import { ComplianceRuleSchema, CountryDatabase, CurrencyDatabase,
  EntitlementDatabase, VenueDatabase } from '..';

/** Client used to access definitions services. */
export abstract class DefinitionsClient {

  /** Returns the organization's name. */
  public abstract get organizationName(): string;

  /** Returns the entitlement database. */
  public abstract get entitlementDatabase(): EntitlementDatabase;

  /** Returns the country database. */
  public abstract get countryDatabase(): CountryDatabase;

  /** Returns the currency database. */
  public abstract get currencyDatabase(): CurrencyDatabase;

  /** Returns the venue database. */
  public abstract get venueDatabase(): VenueDatabase;

  /** Returns the list of compliace rule schemas. */
  public abstract get complianceRuleSchemas(): ComplianceRuleSchema[];

  /**
   * Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
