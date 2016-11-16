// services
import administrationClient from './services/administration-service';
import complianceServiceClient from './services/compliance-service';
import definitionsServiceClient from './services/definitions-service';
import marketDataServiceClient from './services/market-data-service';
import serviceLocatorClient from './services/service-locator';

import serviceLocatorResultCodes from './services/service-locator/result-codes';
import directoryEntry from './services/service-locator/directory-entry';
import accountIdentity from './services/administration-service/account-identity';

// definitions
import money from './definitions/money';
import countryCode from './definitions/country/code';
import countryDatabase from './definitions/country/database';
import countryDatabaseEntry from './definitions/country/database-entry'
import currencyId from './definitions/currency/id';
import currencyDatabase from './definitions/currency/database';
import currencyDatabaseEntry from './definitions/currency/database-entry';

// module export configurations
export const AdministrationClient = administrationClient;
export const ComplianceServiceClient = complianceServiceClient;
export const DefinitionsServiceClient = definitionsServiceClient;
export const MarketDataServiceClient = marketDataServiceClient;
export const ServiceLocatorClient = serviceLocatorClient;

export const ServiceLocatorResultCode = serviceLocatorResultCodes;
export const DirectoryEntry = directoryEntry;
export const AccountIdentity = accountIdentity;

export const Money = money;
export const CountryCode = countryCode;
export const CountryDatabase = countryDatabase;
export const CountryDatabaseEntry = countryDatabaseEntry;
export const CurrencyId = currencyId;
export const CurrencyDatabase = currencyDatabase;
export const CurrencyDatabaseEntry = currencyDatabaseEntry;
