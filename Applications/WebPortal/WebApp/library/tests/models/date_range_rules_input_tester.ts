import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateRangeInput } from
  '../../source/components/date_range_input/date_range_input';
import { DateRange, DateRangeOption, DateRangeRules, DateRuleType,
  DayOffsetDateRule, DayOfMonthDateRule, MonthBoundaryDateRule,
  SpecificDateRule, Weekday, WeekdayDateRule } from
  '../../source/models';
import { makeParametersDateRangeOptions } from
  '../../source/pages/report_page/parameters_date_range_options';

function makeInput(overrides = {}): any {
  const reference = new Beam.Date(2026, 10, 7);
  const input = new DateRangeInput({value: new DateRange(reference, reference),
    referenceDate: reference, options: [], ...overrides});
  (input as any).setState = (update: any, callback?: () => void) => {
    if(typeof update === 'function') {
      update = update(input.state, input.props);
    }
    Object.assign(input, {state: {...input.state, ...update}});
    callback?.();
  };
  return input;
}

function updateProps(input: any, props: any): void {
  const previous = input.props;
  input.props = {...previous, ...props};
  input.componentDidUpdate(previous);
}

describe('DateRangeInput rules', () => {
  const original = globalThis.window;
  beforeEach(() => {
    StyleSheetTestUtils.suppressStyleInjection();
    globalThis.window = {clearTimeout: () => {}} as unknown as
      Window & typeof globalThis;
  });
  afterEach(() => {
    StyleSheetTestUtils.clearBufferAndResumeStyleInjection();
    globalThis.window = original;
  });

  it('offset_controls_preserve_rule_types_and_calendar_parameters', () => {
    const rules = [new DayOffsetDateRule(-2),
      new WeekdayDateRule(-2, Weekday.FRIDAY),
      new DayOfMonthDateRule(-2, 15),
      new MonthBoundaryDateRule(-2, MonthBoundaryDateRule.Boundary.LAST, -1)];
    for(const rule of rules) {
      const input = makeInput();
      input.onStartRule(rule);
      const element = input.render().props.children[1].props.start;
      const editor = new element.type(element.props);
      editor.setState = (update: any) => {
        editor.state = {...editor.state, ...update};
      };
      editor.props = {...editor.props, onChange: (value: any) => {
        const previous = editor.props;
        editor.props = {...editor.props, value};
        editor.componentDidUpdate(previous);
      }};
      editor.onCountChange(3);
      assert.ok(editor.props.value instanceof rule.constructor);
      assert.equal(editor.props.value.offset, -3);
      editor.onDirectionChange('After');
      assert.ok(editor.props.value instanceof rule.constructor);
      assert.equal(editor.props.value.offset, 3);
      editor.onCountChange(0);
      assert.equal(editor.state.direction, 'After');
      editor.onCountChange(undefined);
      assert.equal(editor.props.value.offset, null);
      editor.onCountChange(4);
      assert.equal(editor.props.value.offset, 4);
      if(rule instanceof WeekdayDateRule ||
          rule instanceof DayOfMonthDateRule) {
        assert.equal(editor.props.value.day, rule.day);
      } else if(rule instanceof MonthBoundaryDateRule) {
        assert.equal(editor.props.value.boundary, rule.boundary);
        assert.equal(editor.props.value.dayOffset, rule.dayOffset);
      }
    }
  });

  it('retains_rules_when_the_parent_echoes_the_resolved_value', () => {
    const values: DateRange[] = [];
    const changes: DateRangeRules[] = [];
    const input = makeInput({onChange: (value: DateRange) => values.push(value),
      onRulesChange: (rules: DateRangeRules) => changes.push(rules)});
    input.onStartRule(new WeekdayDateRule(-2, Weekday.MONDAY));
    input.onEndRule(new WeekdayDateRule(0, Weekday.FRIDAY));
    const value = values.at(-1);
    assert.equal(value.start.toJson(), '20260921');
    assert.equal(value.end.toJson(), '20261009');
    updateProps(input, {value: new DateRange(value.start, value.end)});
    assert.equal(input.state.selection, 'custom');
    assert.equal(input.state.rules.start.type, DateRuleType.WEEKDAY);
    assert.equal(input.state.rules.start.offset, -2);
    assert.equal((changes.at(-1).end as WeekdayDateRule).day, Weekday.FRIDAY);
  });

  it('applies_replacement_rules_and_reference_together', () => {
    const input = makeInput();
    const rules = new DateRangeRules(new DayOfMonthDateRule(-1, 31),
      new DayOffsetDateRule(0));
    updateProps(input, {rules, referenceDate: new Beam.Date(2024, 3, 31)});
    assert.equal(input.state.rules.start.day, 31);
    assert.equal(input.state.value.start.toJson(), '20240229');
    assert.equal(input.state.value.end.toJson(), '20240331');
    updateProps(input, {referenceDate: new Beam.Date(2024, 4, 1)});
    assert.equal(input.state.value.start.toJson(), '20240331');
    assert.equal(input.state.value.end.toJson(), '20240401');
  });

  it('does_not_publish_invalid_or_incomplete_resolved_ranges', () => {
    const values: DateRange[] = [];
    const input = makeInput({onChange: (value: DateRange) => values.push(value)});
    input.onStartRule(new DayOffsetDateRule(1));
    assert.equal(values.length, 0);
    assert.equal(input.getValidation().valid, false);
    input.onStartRule(new DayOffsetDateRule(null));
    assert.equal(values.length, 0);
    assert.equal(input.getValidation().valid, false);
    input.onStartRule(new DayOffsetDateRule(-1));
    assert.equal(values.at(-1).start.toJson(), '20261006');
    assert.equal(input.getValidation().valid, true);
  });

  it('rejects_fixed_dates_below_the_cpp_minimum', () => {
    const values: DateRange[] = [];
    const input = makeInput({boundsRequired: false,
      onChange: (value: DateRange) => values.push(value)});
    input.onStartInput(new Beam.Date(1399, 12, 31), true);
    input.onCommit();
    assert.equal(input.getValidation().valid, false);
    assert.equal(values.length, 0);
    input.onStartInput(new Beam.Date(1400, 1, 1), true);
    input.onCommit();
    assert.equal(input.getValidation().valid, true);
    assert.equal(values.length, 1);
  });

  it('commits_edits_after_simultaneous_external_rules_and_value_updates', () => {
    const values: DateRange[] = [];
    const input = makeInput({onChange: (value: DateRange) => values.push(value)});
    const original = input.props.value;
    const date = new Beam.Date(2026, 10, 6);
    updateProps(input, {value: new DateRange(date, original.end),
      rules: new DateRangeRules(
        new SpecificDateRule(date),
        new SpecificDateRule(original.end))});
    input.onStartRule(new SpecificDateRule(original.start));
    assert.equal(values.at(-1).start.toJson(), '20261007');
    assert.equal(values.at(-1).end.toJson(), '20261007');
  });

  it('preserves_an_incomplete_fixed_bound_when_the_other_rule_changes', () => {
    const values: DateRange[] = [];
    const input = makeInput({boundsRequired: false,
      onChange: (value: DateRange) => values.push(value)});
    input.onStartInput(null, false);
    input.onEndRule(new DayOffsetDateRule(0));
    assert.equal(input.getValidation().valid, false);
    assert.equal(values.length, 0);
    input.onStartInput(null, true);
    input.onCommit();
    assert.equal(input.getValidation().valid, true);
    assert.equal(values.at(-1).start, null);
  });

  it('publishes_replacement_rules_when_selecting_a_preset', () => {
    const changes: DateRangeRules[] = [];
    const date = new Beam.Date(2026, 9, 1);
    const input = makeInput({options: [
      new DateRangeOption('saved', 'Saved range', date, date)],
      onRulesChange: (rules: DateRangeRules) => changes.push(rules)});
    input.onStartRule(new DayOffsetDateRule(0));
    input.onSelect('saved');
    assert.equal(input.state.selection, 'saved');
    assert.equal(changes.at(-1).start.type, DateRuleType.SPECIFIC_DATE);
    assert.equal((changes.at(-1).start as SpecificDateRule).date.toJson(),
      '20260901');
    assert.equal((changes.at(-1).end as SpecificDateRule).date.toJson(),
      '20260901');
  });

  it('expands_presets_into_relative_custom_rules', () => {
    const reference = new Beam.Date(2026, 10, 7);
    const changes: DateRangeRules[] = [];
    const input = makeInput({options: makeParametersDateRangeOptions(reference),
      onRulesChange: (rules: DateRangeRules) => changes.push(rules)});
    input.onSelect('previous-month');
    input.onSelect('custom');
    assert.equal(input.state.rules.start.type, DateRuleType.MONTH_BOUNDARY);
    assert.equal(input.state.rules.start.boundary,
      MonthBoundaryDateRule.Boundary.FIRST);
    assert.equal(input.state.rules.start.offset, -1);
    assert.equal(input.state.rules.end.boundary,
      MonthBoundaryDateRule.Boundary.LAST);
    assert.equal(input.state.rules.end.offset, -1);
    assert.equal(changes.at(-1).start.type, DateRuleType.MONTH_BOUNDARY);
  });

  it('keeps_the_active_preset_when_its_reference_changes', () => {
    const reference = new Beam.Date(2026, 10, 7);
    const input = makeInput({options: makeParametersDateRangeOptions(reference)});
    input.onSelect('month-to-date');
    const next = new Beam.Date(2026, 11, 5);
    updateProps(input, {referenceDate: next,
      options: makeParametersDateRangeOptions(next)});
    assert.equal(input.state.selection, 'month-to-date');
    assert.equal(input.state.value.start.toJson(), '20261101');
    assert.equal(input.state.value.end.toJson(), '20261105');
    input.onSelect('custom');
    const after = new Beam.Date(2026, 12, 2);
    updateProps(input, {referenceDate: after,
      options: makeParametersDateRangeOptions(after)});
    assert.equal(input.state.selection, 'custom');
    assert.equal(input.state.value.start.toJson(), '20261201');
    assert.equal(input.state.value.end.toJson(), '20261202');
  });

  it('resets_to_specific_dates_when_the_caller_replaces_the_value', () => {
    const input = makeInput();
    input.onStartRule(new DayOffsetDateRule(-7));
    const value = new DateRange(new Beam.Date(2026, 1, 1),
      new Beam.Date(2026, 1, 31));
    updateProps(input, {value});
    assert.equal(input.state.rules.start.type, DateRuleType.SPECIFIC_DATE);
    assert.equal(input.state.rules.end.type, DateRuleType.SPECIFIC_DATE);
    assert.ok(input.state.rules.start.date.equals(value.start));
  });
});
