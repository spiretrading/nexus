class NumberFormatter {
  formatWithComma(number) {
    var parts = number.toString().split(".");
    parts[0] = parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
    return parts.join(".");
  }

  formatCurrencyWithComma(number) {
    let numberStr = number.toString().replace(/,/g, '');
    let numbers = numberStr.split('.');
    let formattedWholeNumber = this.formatWithComma(numbers[0]);
    if (numbers[1] != null && numbers[1] != '') {
      let twoDecimialsNumber = numbers[1].substring(0, 2);
      return formattedWholeNumber + '.' + twoDecimialsNumber;
    } else {
      formattedWholeNumber = formattedWholeNumber.replace('.', '');
      return formattedWholeNumber;
    }
  }
}

export default new NumberFormatter();
