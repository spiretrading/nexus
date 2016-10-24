/** Label appearance formatter */
class LabelFormatter {
  toCapitalWithSpace(label) {
    label = label.replace(/_/g, ' ');
    return label.replace(/\b\w/g, l => l.toUpperCase());
  }
}

export default new LabelFormatter();
