import StyleRule from 'utils/table-models/style-rules/style-rule';
import { Money } from 'spire-client';

const GREEN = '#24aa08';
const RED = '#d32f2f';
const BLACK = 'black';

export default class extends StyleRule {
  getFontColor(money) {
    let compareToZero = money.compare(Money.fromNumber(0));
    if (compareToZero > 0) {
      return GREEN;
    } else if (compareToZero < 0) {
      return RED;
    } else {
      return BLACK;
    }
  }
}
