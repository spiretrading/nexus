import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateRangeInput } from '../../source/components/date_range_input';
import { DateExpression, DateOffset, DateRange, DateRangeExpressions,
  DateRangeRules, DayOffsetDateRule, SpecificDateRule } from
  '../../source/models';
import { makeParametersDateRangeOptions } from
  '../../source/pages/report_page/parameters_date_range_options';

const TODAY = new Beam.Date(2026, 10, 20);

function input(value: DateRange): DateRangeInput {
  return new DateRangeInput({value, rules: value.rules, referenceDate: TODAY,
    customType: DateRangeInput.CustomType.EXPRESSION,
    options: makeParametersDateRangeOptions(TODAY)});
}

describe('DateRangeInput presets', () => {
  it('recognizes_relative_preset_rules', () => {
    const rules = new DateRangeRules(
      new DayOffsetDateRule(0), new DayOffsetDateRule(0));
    assert.equal(input(new DateRange(TODAY, TODAY, rules)).state.selection,
      'today');
    const yesterday = new Beam.Date(2026, 10, 19);
    assert.equal(input(new DateRange(yesterday, yesterday,
      rules)).state.selection, 'today');
  });

  it('keeps_fixed_rules_custom_when_dates_match_a_relative_preset', () => {
    const rules = new DateRangeRules(
      new SpecificDateRule(TODAY), new SpecificDateRule(TODAY));
    assert.equal(input(new DateRange(TODAY, TODAY, rules)).state.selection,
      'custom');
  });

  it('keeps_custom_expression_editors_open_when_they_match_a_preset', () => {
    const expression: DateExpression = {mode: DateExpression.Mode.DAY_OFFSET,
      date: {value: 0, unit: DateOffset.Unit.DAY}};
    const value = new DateRange(TODAY, TODAY,
      new DateRangeRules(new DayOffsetDateRule(0), new DayOffsetDateRule(0)),
      new DateRangeExpressions(expression, expression));
    assert.equal(input(value).state.selection, 'custom');
  });
});
