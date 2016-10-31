// end points
import AdminClient from './end-points/admin';
import ComplianceServiceClient from './end-points/compliance-service';
import DefinitionsServiceClient from './end-points/definitions-service';
import MarketDataServiceClient from './end-points/market-data-service';
import ServiceLocatorClient from './end-points/service-locator';
import ServiceLocatorResultCodes from './end-points/service-locator/result-codes';

// data
import MoneyData from './data/money';

export const Admin = AdminClient;
export const ComplianceService = ComplianceServiceClient;
export const DefinitionsService = DefinitionsServiceClient;
export const MarketDataService = MarketDataServiceClient;
export const ServiceLocator = ServiceLocatorClient;
export const ServiceLocatorResultCode = ServiceLocatorResultCodes;

export const Money = MoneyData;
