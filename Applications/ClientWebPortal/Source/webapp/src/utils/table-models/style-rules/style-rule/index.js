const WHITE = 'white';
const LIGHT_GRAY = '#fbfbfb';

export default class {
  /** @protected */
  getFontColor(value) {
    throw new TypeError('Method not implemented');
  }

  /** @protected */
  getBackgroundColor(y, value) {
    if (y % 2 == 1) {
      return LIGHT_GRAY;
    } else {
      return WHITE;
    }
  }
}
