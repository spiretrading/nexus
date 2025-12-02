import * as Beam from 'beam';
import { CountryDatabase, CurrencyDatabase, EntitlementDatabase,
  VenueDatabase } from '..';
import { ComplianceRuleSchema } from '../compliance_service';
import { DefinitionsClient } from './definitions_client';

/** Implements the DefinitionsClient using HTTP requests. */
export class HttpDefinitionsClient extends DefinitionsClient {
  public get organizationName(): string {
    return this._organizationName;
  }

  public get entitlementDatabase(): EntitlementDatabase {
    return this._entitlementDatabase;
  }

  public get countryDatabase(): CountryDatabase {
    return this._countryDatabase;
  }

  public get currencyDatabase(): CurrencyDatabase {
    return this._currencyDatabase;
  }

  public get venueDatabase(): VenueDatabase {
    return this._venueDatabase;
  }

  public get complianceRuleSchemas(): ComplianceRuleSchema[] {
    return this._complianceRuleSchemas.slice();
  }

  public async open(): Promise<void> {
    const organizationNameResponse = await Beam.post(
      '/api/definitions_service/load_organization_name', {});
    this._organizationName = organizationNameResponse;
    const entitlementResponse = await Beam.post(
      '/api/administration_service/load_entitlements_database', {});
    this._entitlementDatabase = EntitlementDatabase.fromJson(
      entitlementResponse);
    const countryResponse = await Beam.post(
      '/api/definitions_service/load_country_database', {});
    this._countryDatabase = CountryDatabase.fromJson(countryResponse);
    const currencyResponse = await Beam.post(
      '/api/definitions_service/load_currency_database', {});
    this._currencyDatabase = CurrencyDatabase.fromJson(currencyResponse);
    const venueResponse = await Beam.post(
      '/api/definitions_service/load_venue_database', {});
    this._venueDatabase = VenueDatabase.fromJson(venueResponse);
    const complianceRuleSchemasResponse = await Beam.post(
      '/api/definitions_service/load_compliance_rule_schemas', {});
    this._complianceRuleSchemas =
      Beam.arrayFromJson(ComplianceRuleSchema, complianceRuleSchemasResponse);
  }

  public async close(): Promise<void> {
    return;
  }

  private _organizationName: string;
  private _entitlementDatabase: EntitlementDatabase;
  private _countryDatabase: CountryDatabase;
  private _currencyDatabase: CurrencyDatabase;
  private _venueDatabase: VenueDatabase;
  private _complianceRuleSchemas: ComplianceRuleSchema[];
}
