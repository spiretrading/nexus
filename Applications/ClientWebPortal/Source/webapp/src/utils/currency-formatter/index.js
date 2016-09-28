import numberFormatter from 'utils/number-formatter';
import definitionsService from 'services/definitions';

class CurrencyFormatter {
  format(countryIsoNumber, amount) {
    let sign;
    let number = numberFormatter.formatWithComma(amount);

    if (definitionsService.doesCurrencyExist.apply(definitionsService, [countryIsoNumber])) {
      sign = definitionsService.getCurrencySign(countryIsoNumber);

    } else {
      sign = '';
    }

    return sign + ' ' + number;
  }
}

export default new CurrencyFormatter();
