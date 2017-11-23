import StyleRule from 'utils/table-models/style-rules/style-rule';

const BLACK = 'black';

export default class extends StyleRule {
  getFontColor(value) {
    return BLACK;
  }
}
