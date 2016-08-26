import numberFormatter from 'utils/number-formatter';
import definitionsService from 'services/definitions';

class CurrencyFormatter {
  format(countryIsoNumber, amount) {
    let sign = definitionsService.getCurrencySign(countryIsoNumber);
    let number = numberFormatter.formatWithComma(amount);
    return sign + ' ' + number;
  }
}

export default new CurrencyFormatter();
