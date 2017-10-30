import definitionsService from 'services/definitions';

class ValueComparer {

}

ValueComparer.compare = (a, b) => {
  if (a == null && b == null) {
    return 0;
  } else if (a == null) {
    return 1;
  } else if (b == null) {
    return -1;
  }

  let constructorName = a.constructor.name;
  let result;
  if (constructorName == 'DirectoryEntry') {
    result = a.name.localeCompare(b.name);
  } else if (constructorName == 'Security') {
    let stringA = a.toString(definitionsService.getMarketDatabase());
    let stringB = b.toString(definitionsService.getMarketDatabase());
    result = stringA.localeCompare(stringB);
  } else if (constructorName == 'CurrencyId') {
    let stringA = definitionsService.getCurrencyCode(a.toNumber());
    let stringB = definitionsService.getCurrencyCode(b.toNumber());
    result = stringA.localeCompare(stringB);
  } else if (typeof a == 'string') {
    result = a.localeCompare(b);
  } else {
    result = a - b;
  }

  return result;
}

export default ValueComparer;
