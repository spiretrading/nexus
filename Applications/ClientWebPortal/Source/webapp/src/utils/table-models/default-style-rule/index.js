import StyleRule from 'utils/table-models/style-rule';

const BLACK = 'black';
const WHITE = 'white';
const LIGHT_GRAY = '#fbfbfb';

export default class extends StyleRule {
  getFontColor(value) {
    return BLACK;
  }

  getBackgroundColor(y, value) {
    if (y % 2 == 1) {
      return LIGHT_GRAY;
    } else {
      return WHITE;
    }
  }
}
