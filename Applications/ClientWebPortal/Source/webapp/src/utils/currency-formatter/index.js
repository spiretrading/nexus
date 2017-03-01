import numberFormatter from 'utils/number-formatter';
import definitionsService from 'services/definitions';

class CurrencyFormatter {
  formatById(countryIsoNumber, amount) {
    let sign;
    let number = numberFormatter.formatTwoDecimalsWithComma(amount);

    if (definitionsService.doesCurrencyExist.apply(definitionsService, [countryIsoNumber])) {
      sign = definitionsService.getCurrencySignFromId(countryIsoNumber);

    } else {
      sign = '';
    }

    return sign + ' ' + number;
  }

  formatByCode(code, amount) {
    let currencyId = definitionsService.getCurrencyId(code);
    return this.formatById(currencyId, amount);
  }
}

export default new CurrencyFormatter();
