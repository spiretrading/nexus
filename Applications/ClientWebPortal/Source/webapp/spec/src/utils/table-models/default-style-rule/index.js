import DefaultStyleRule from '../../../../webapp/utils/table-models/style-rules/default-style-rule';

const BLACK = 'black';
const WHITE = 'white';
const LIGHT_GRAY = '#fbfbfb';

describe('DefaultStyleRule', function() {
  it('getFontColor', function() {
    let defaultStyleRule = new DefaultStyleRule();
    let someValue = 5;
    expect(defaultStyleRule.getFontColor(soemValue)).toBe(BLACK);
  });

  it('getBackgroundColor - odd number row', function() {
    let defaultStyleRule = new DefaultStyleRule();
    let someValue = 5;
    expect(defaultStyleRule.getBackgroundColor(5, soemValue)).toBe(LIGHT_GRAY);
  });

  it('getBackgroundColor - even number row', function() {
    let defaultStyleRule = new DefaultStyleRule();
    let someValue = 5;
    expect(defaultStyleRule.getBackgroundColor(4, soemValue)).toBe(WHITE);
  });
});
