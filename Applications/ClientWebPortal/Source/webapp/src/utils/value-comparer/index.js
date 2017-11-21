import definitionsService from 'services/definitions';
import { DirectoryEntry, Money, Security, CurrencyId } from 'spire-client';

export default class {
  compare(a, b) {
    if (a == null && b == null) {
      return 0;
    } else if (a == null) {
      return 1;
    } else if (b == null) {
      return -1;
    }

    let result;
    if (a instanceof DirectoryEntry && b instanceof DirectoryEntry) {
      result = a.compare(b);
    } else if (a instanceof Security && b instanceof Security) {
      let stringA = a.toString(definitionsService.getMarketDatabase());
      let stringB = b.toString(definitionsService.getMarketDatabase());
      result = stringA.localeCompare(stringB);
    } else if (a instanceof CurrencyId && b instanceof CurrencyId) {
      let stringA = definitionsService.getCurrencyCode(a.toNumber());
      let stringB = definitionsService.getCurrencyCode(b.toNumber());
      result = stringA.localeCompare(stringB);
    } else if (typeof a == 'string' && typeof b == 'string') {
      result = a.localeCompare(b);
    } else {
      result = a - b;
    }

    return result;
  }
}
