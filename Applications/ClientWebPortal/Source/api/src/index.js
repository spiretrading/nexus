// services
import administrationClient from './services/administration-service';
import complianceServiceClient from './services/compliance-service';
import definitionsServiceClient from './services/definitions-service';
import marketDataServiceClient from './services/market-data-service';
import serviceLocatorClient from './services/service-locator';
import riskServiceClient from './services/risk-service';

import serviceLocatorResultCodes from './services/service-locator/result-codes';
import directoryEntry from './definitions/directory-entry';
import accountIdentity from './services/administration-service/account-identity';

// definitions
import dataType from './definitions/data-type';
import money from './definitions/money';
import countryCode from './definitions/country-code';
import countryDatabase from './definitions/country-database';
import countryDatabaseEntry from './definitions/country-database-entry'
import currencyId from './definitions/currency-id';
import currencyPair from './definitions/currency-pair';
import currencyDatabase from './definitions/currency-database';
import currencyDatabaseEntry from './definitions/currency-database-entry';
import security from './definitions/security';
import exchangeRate from './definitions/exchange-rate'
import exchangeRateTable from './definitions/exchange-rate-table';

// module export configurations
export const AdministrationClient = administrationClient;
export const ComplianceServiceClient = complianceServiceClient;
export const DefinitionsServiceClient = definitionsServiceClient;
export const MarketDataServiceClient = marketDataServiceClient;
export const ServiceLocatorClient = serviceLocatorClient;
export const RiskServiceClient = riskServiceClient;

export const ServiceLocatorResultCode = serviceLocatorResultCodes;
export const DirectoryEntry = directoryEntry;
export const AccountIdentity = accountIdentity;

export const DataType = dataType;
export const Money = money;
export const CountryCode = countryCode;
export const CountryDatabase = countryDatabase;
export const CountryDatabaseEntry = countryDatabaseEntry;
export const CurrencyId = currencyId;
export const CurrencyPair = currencyPair;
export const CurrencyDatabase = currencyDatabase;
export const CurrencyDatabaseEntry = currencyDatabaseEntry;
export const Security = security;
export const ExchangeRate = exchangeRate;
export const ExchangeRateTable = exchangeRateTable;
