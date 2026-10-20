import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { DateRuleInput } from '../../source/components/date_rule_input';
import { DateRangeInput } from
  '../../source/components/date_range_input/date_range_input';
import { DateExpression, DateOffset, DateRangeExpressions, DateRange,
  DateRangeOption, DateRangeRules, DateRuleType,
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

function setStartRule(input: any, start: any): void {
  input.setRules(new DateRangeRules(start, input.state.rules.end), true);
}

function setEndRule(input: any, end: any): void {
  input.setRules(new DateRangeRules(input.state.rules.start, end), true);
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

  it('shared_rule_editor_preserves_empty_fixed_date_drafts', () => {
    const reference = new Beam.Date(2026, 10, 8);
    const input = new DateRuleInput({value: new SpecificDateRule(reference),
      reference, dateValue: null, label: 'From', onChange: () => {}});
    assert.equal(input.render().props.children[1].props.value, null);
  });

  it('offset_controls_preserve_rule_types_and_calendar_parameters', () => {
    const rules = [new DayOffsetDateRule(-2),
      new WeekdayDateRule(-2, Weekday.FRIDAY),
      new DayOfMonthDateRule(-2, 15),
      new MonthBoundaryDateRule(-2, MonthBoundaryDateRule.Boundary.LAST, -1)];
    for(const rule of rules) {
      const input = makeInput();
      setStartRule(input, rule);
      const editor = new DateRuleInput({value: rule, label: 'Start',
        reference: input.state.reference, onChange: () => {}}) as any;
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
    setStartRule(input, new WeekdayDateRule(-2, Weekday.MONDAY));
    setEndRule(input, new WeekdayDateRule(0, Weekday.FRIDAY));
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
    const input = makeInput({
      onChange: (value: DateRange) => values.push(value)});
    setStartRule(input, new DayOffsetDateRule(1));
    assert.equal(values.length, 0);
    assert.equal(input.getValidation().valid, false);
    setStartRule(input, new DayOffsetDateRule(null));
    assert.equal(values.length, 0);
    assert.equal(input.getValidation().valid, false);
    setStartRule(input, new DayOffsetDateRule(-1));
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

  it('commits_edits_after_simultaneous_external_rules_and_value_updates',
      () => {
    const values: DateRange[] = [];
    const input = makeInput({
      onChange: (value: DateRange) => values.push(value)});
    const original = input.props.value;
    const date = new Beam.Date(2026, 10, 6);
    updateProps(input, {value: new DateRange(date, original.end),
      rules: new DateRangeRules(
        new SpecificDateRule(date),
        new SpecificDateRule(original.end))});
    setStartRule(input, new SpecificDateRule(original.start));
    assert.equal(values.at(-1).start.toJson(), '20261007');
    assert.equal(values.at(-1).end.toJson(), '20261007');
  });

  it('preserves_an_incomplete_fixed_bound_when_the_other_rule_changes', () => {
    const values: DateRange[] = [];
    const input = makeInput({boundsRequired: false,
      onChange: (value: DateRange) => values.push(value)});
    input.onStartInput(null, false);
    setEndRule(input, new DayOffsetDateRule(0));
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
    setStartRule(input, new DayOffsetDateRule(0));
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
    const input = makeInput({
      options: makeParametersDateRangeOptions(reference)});
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
    setStartRule(input, new DayOffsetDateRule(-7));
    const value = new DateRange(new Beam.Date(2026, 1, 1),
      new Beam.Date(2026, 1, 31));
    updateProps(input, {value});
    assert.equal(input.state.rules.start.type, DateRuleType.SPECIFIC_DATE);
    assert.equal(input.state.rules.end.type, DateRuleType.SPECIFIC_DATE);
    assert.ok(input.state.rules.start.date.equals(value.start));
  });
});

describe('DateRangeInput expressions', () => {
  const fixed = (date: Beam.Date): DateExpression =>
    ({mode: DateExpression.Mode.FIXED, date});
  const offset = (value: number): DateExpression =>
    ({mode: DateExpression.Mode.DAY_OFFSET,
      date: {value, unit: DateOffset.Unit.DAY}});

  it('publishes_expressions_even_when_resolved_dates_are_unchanged', () => {
    const values: DateRange[] = [];
    const input = makeInput({customType: DateRangeInput.CustomType.EXPRESSION,
      onChange: (value: DateRange) => values.push(value)});
    input.onStartExpressionChange(offset(0));
    assert.equal(values.length, 1);
    assert.equal(values[0].expressions.start.mode,
      DateExpression.Mode.DAY_OFFSET);
    input.onStartExpressionChange(fixed(input.state.reference));
    assert.equal(values.length, 2);
    assert.equal(values[1].expressions.start.mode, DateExpression.Mode.FIXED);
    updateProps(input, {value: values[1]});
    assert.equal(input.state.selection, 'custom');
    input.onStartExpressionChange(offset(0));
    updateProps(input, {value: new DateRange(input.state.value.start,
      input.state.value.end)});
    assert.equal(input.state.expressions.start.mode, DateExpression.Mode.FIXED);
  });

  it('preserves_incomplete_bounds_and_only_publishes_valid_commits', () => {
    const values: DateRange[] = [];
    const input = makeInput({boundsRequired: false,
      customType: DateRangeInput.CustomType.EXPRESSION,
      onChange: (value: DateRange) => values.push(value)});
    input.onStartExpressionInput(fixed(null), false);
    input.onEndExpressionChange(offset(1));
    assert.equal(input.getValidation().valid, false);
    assert.equal(values.length, 0);
    input.onStartExpressionChange(fixed(null), true);
    assert.equal(values.length, 1);
    input.onStartExpressionChange(offset(2));
    assert.equal(input.getValidation().valid, false);
    assert.equal(values.length, 1);
    input.onStartExpressionInput(offset(0));
    assert.equal(values.length, 1);
    input.onStartExpressionChange(offset(0));
    assert.equal(values.length, 2);
  });

  it('uses_external_expressions_and_resolves_reference_updates', () => {
    const original = globalThis.window;
    globalThis.window = {clearTimeout: () => {}} as any;
    try {
      const values: DateRange[] = [];
      const input = makeInput({customType: DateRangeInput.CustomType.EXPRESSION,
        onChange: (value: DateRange) => values.push(value)});
      updateProps(input, {referenceDate: new Beam.Date(2024, 3, 1),
        value: new DateRange(null, null, undefined,
          new DateRangeExpressions(offset(-1), offset(0)))});
      assert.equal(input.state.value.start.toJson(), '20240229');
      updateProps(input, {referenceDate: new Beam.Date(2024, 3, 2)});
      assert.equal(input.state.value.start.toJson(), '20240301');
      assert.equal(input.state.value.end.toJson(), '20240302');
      assert.equal(values.length, 0);
      updateProps(input, {value: new DateRange(new Beam.Date(2024, 2, 29),
        new Beam.Date(2024, 3, 1), undefined,
        new DateRangeExpressions(offset(-1), offset(0)))});
      assert.equal(input.state.expressions.start.mode,
        DateExpression.Mode.DAY_OFFSET);
      assert.equal(input.state.value.start.toJson(), '20240301');
      updateProps(input, {customType: DateRangeInput.CustomType.FIXED});
      updateProps(input, {customType: DateRangeInput.CustomType.EXPRESSION,
        value: new DateRange(null, null, undefined,
          new DateRangeExpressions(offset(-2), offset(0)))});
      assert.equal(input.state.value.start.toJson(), '20240229');
    } finally {
      globalThis.window = original;
    }
  });
});
