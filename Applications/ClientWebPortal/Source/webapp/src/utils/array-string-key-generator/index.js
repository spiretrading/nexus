import { Security } from 'spire-client';
import definitionsService from 'services/definitions';

export default class {
  get(values) {
    let key = "";
    for (let i=0; i<values.length; i++) {
      if (values[i] == null) {
        throw new TypeError('Values must not contain a null');
      }

      key += '(';
      if (values[i] instanceof Security) {
        let marketDatabase = definitionsService.getMarketDatabase();
        key += values[i].toString(marketDatabase);
      } else if (values[i].toString != null) {
        key += values[i].toString();
      } else {
        key += values[i];
      }
      key += ')';
    }
    return key;
  }
}
