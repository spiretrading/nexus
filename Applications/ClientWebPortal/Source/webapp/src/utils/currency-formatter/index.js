import numberFormatter from 'utils/number-formatter';
import definitionsService from 'services/definitions';

class CurrencyFormatter {
  constructor() {
    this.formatByCode = this.formatByCode.bind(this);
  }

  formatById(countryIsoNumber, amount) {
    let sign;
    let number = numberFormatter.formatTwoDecimalsWithComma(amount);

    if (definitionsService.doesCurrencyExist(countryIsoNumber)) {
      sign = definitionsService.getCurrencySignFromId(countryIsoNumber);

    } else {
      sign = '';
    }

    return sign + number;
  }

  formatByCode(code, amount) {
    let currencyId = definitionsService.getCurrencyId(code);
    return this.formatById(currencyId, amount);
  }
}

export default new CurrencyFormatter();
