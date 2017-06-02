class NumberFormatter {
  formatWithComma(number) {
    var parts = number.toString().split(".");
    parts[0] = parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
    return parts.join(".");
  }

  formatTwoDecimalsWithComma(number) {
    let numberStr = number.toString().replace(/,/g, '');
    let numbers = numberStr.split('.');
    let formattedWholeNumber = this.formatWithComma(numbers[0]);
    if (numbers[1] != null && numbers[1] != '') {
      if (numbers[1].length == 0) {
        numbers[1] += '00';
      } else if (numbers[1].length == 1) {
        numbers[1] += '0';
      }
      let twoDecimialsNumber = numbers[1].substring(0, 2);
      return formattedWholeNumber + '.' + twoDecimialsNumber;
    } else {
      formattedWholeNumber += '.00';
      return formattedWholeNumber;
    }
  }
}

export default new NumberFormatter();
