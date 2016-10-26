/** Label appearance formatter */
class LabelFormatter {
  // from: buying_power to: Buying Power
  toCapitalWithSpace(label) {
    label = label.replace(/_/g, ' ');
    return label.replace(/\b\w/g, l => l.toUpperCase());
  }

  // from: Buying Power to: buying_power
  toLowerCaseWithUnderscore(label) {
    label = label.replace(/\b\w/g, l => l.toLowerCase());
    return label = label.replace(/ /g, '_');
  }
}

export default new LabelFormatter();
