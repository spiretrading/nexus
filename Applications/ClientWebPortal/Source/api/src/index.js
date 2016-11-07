// end points
import administrationClient from './end-points/admin';
import complianceServiceClient from './end-points/compliance-service';
import definitionsServiceClient from './end-points/definitions-service';
import marketDataServiceClient from './end-points/market-data-service';
import serviceLocatorClient from './end-points/service-locator';
import serviceLocatorResultCodes from './end-points/service-locator/result-codes';

// data
import MoneyData from './data/money';

export const AdministrationClient = administrationClient;
export const ComplianceServiceClient = complianceServiceClient;
export const DefinitionsServiceClient = definitionsServiceClient;
export const MarketDataServiceClient = marketDataServiceClient;
export const ServiceLocatorClient = serviceLocatorClient;
export const ServiceLocatorResultCode = serviceLocatorResultCodes;

export const Money = MoneyData;
