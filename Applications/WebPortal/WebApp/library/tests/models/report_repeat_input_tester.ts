import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { DateRuleType, DayOffsetDateRule, DayOfMonthDateRule, Interval,
  MonthBoundaryDateRule, SpecificDateRule, Weekday, WeekdayDateRule } from
  '../../source/models';
import { ReportDefinition } from
  '../../source/pages/report_page/report_definition';
import { ReportFormTemplate } from
  '../../source/pages/report_page/report_form_template';
import { copyReportFormValue, parseReportFormValue, reportFormValueToJson }
  from '../../source/pages/report_page/report_form_value';
import { isValidReportRepeat, ReportRepeatInput } from
  '../../source/pages/report_page/report_repeat_input';

function makeInput(): any {
  const input = new ReportRepeatInput({interval: null,
    reference: new Beam.Date(2026, 10, 8), onChange: (interval, rule) => {
      const previous = input.props;
      Object.assign(input, {props: {...input.props, interval, rule}});
      input.componentDidUpdate(previous);
    }});
  input.setState = (update: any) => {
    Object.assign(input, {state: {...input.state, ...update}});
  };
  return input;
}

describe('ReportRepeatInput', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('presets_and_custom_rules', () => {
    const input = makeInput();
    for(const [preset, count, unit] of [
        ['daily', 1, Interval.Unit.DAY],
        ['weekly', 1, Interval.Unit.WEEK],
        ['fortnight', 2, Interval.Unit.WEEK],
        ['month_start', 1, Interval.Unit.MONTH],
        ['month_end', 1, Interval.Unit.MONTH]]) {
      input.onSelectionChange(preset);
      assert.equal(input.state.selection, preset);
      assert.equal(input.props.interval.count, count);
      assert.equal(input.props.interval.unit, unit);
      assert.ok(isValidReportRepeat(input.props.interval, input.props.rule));
    }
    input.onSelectionChange('custom');
    input.onRuleChange(new MonthBoundaryDateRule(0,
      MonthBoundaryDateRule.Boundary.LAST, -1));
    assert.equal(input.state.selection, 'custom');
    assert.equal(input.props.rule.dayOffset, -1);
    input.onIntervalChange(new Interval(2, Interval.Unit.WEEK));
    assert.equal(input.props.rule, null);
    input.onDateChange('rule');
    assert.equal(input.props.rule.type, DateRuleType.WEEKDAY);
    input.onSelectionChange('never');
    assert.equal(input.props.interval, null);
    assert.equal(input.props.rule, null);
  });

  it('rejects_incomplete_or_incompatible_rules', () => {
    const monthly = new Interval(1, Interval.Unit.MONTH);
    assert.ok(isValidReportRepeat(monthly, new DayOffsetDateRule(0)));
    assert.ok(!isValidReportRepeat(new Interval(null, Interval.Unit.MONTH), null));
    assert.ok(!isValidReportRepeat(monthly, new DayOfMonthDateRule(0, null)));
    assert.ok(!isValidReportRepeat(monthly,
      new SpecificDateRule(new Beam.Date(2026, 10, 8))));
    assert.ok(!isValidReportRepeat(monthly,
      new WeekdayDateRule(0, Weekday.TUESDAY)));
    assert.ok(!isValidReportRepeat(monthly, new DayOfMonthDateRule(1, 15)));
  });

  it('preserves_rules_in_submission_loading_and_copying', () => {
    const definition = new ReportDefinition('test', 'Test', '', [], null);
    const value = ReportFormTemplate.makeValue(definition);
    value.scheduled = true;
    value.repeats = true;
    value.repeatInterval = new Interval(1, Interval.Unit.MONTH);
    value.repeatRule = new MonthBoundaryDateRule(0,
      MonthBoundaryDateRule.Boundary.FIRST, -1);
    const json = reportFormValueToJson(value);
    assert.deepEqual(json.repeat_interval.rule, {type: 'MONTH_BOUNDARY',
      value: {offset: 0, boundary: 'FIRST', day_offset: -1}});
    const loaded = parseReportFormValue(json, [definition]);
    const copied = copyReportFormValue(loaded, [definition]);
    assert.deepEqual(copied.repeatRule, value.repeatRule);
    assert.notEqual(copied.repeatRule, loaded.repeatRule);
    value.repeats = false;
    assert.equal(reportFormValueToJson(value).repeat_interval, null);
  });
});
