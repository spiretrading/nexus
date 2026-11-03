import * as assert from 'node:assert/strict';
import { it } from 'node:test';
import * as Beam from 'beam';
import { DateRange, DateRangeRules, DayOffsetDateRule, Interval,
  OrdinalDayOfMonthDateRule, SpecificDateRule } from '../../source/models';
import { EditScheduledReportController, ReportDefinition, ReportFormTemplate }
  from '../../source/pages/report_page';

it('enables_save_when_only_the_date_range_rules_change', () => {
  const today = Beam.Date.today();
  const definition = new ReportDefinition('test', 'Test', '', [], null);
  const original = {...ReportFormTemplate.makeValue(definition), parameters: {
    period: new DateRange(today, today, new DateRangeRules(
      new SpecificDateRule(today), new SpecificDateRule(today)))}};
  const model: any = {reports: [definition], accountModel: null,
    scopeModel: null};
  const controller = new EditScheduledReportController({model, id: 'test'});
  const state = {...controller.state, loaded: true, original, value: original};
  Object.assign(controller, {state});
  assert.ok(controller.render().props.submitDisabled);
  Object.assign(controller, {state: {...state, value: {...original,
    parameters: {period: new DateRange(today, today, new DateRangeRules(
      new DayOffsetDateRule(0), new DayOffsetDateRule(0)))}}}});
  assert.ok(!controller.render().props.submitDisabled);
});

it('tracks_recurrence_changes_and_restoring_the_saved_value', () => {
  const definition = new ReportDefinition('test', 'Test', '', [], null);
  const original = {...ReportFormTemplate.makeValue(definition),
    scheduled: true, repeats: true,
    repeatInterval: new Interval(1, Interval.Unit.YEAR), repeatMonth: 2,
    repeatRule: new OrdinalDayOfMonthDateRule(0,
      OrdinalDayOfMonthDateRule.Position.LAST,
      OrdinalDayOfMonthDateRule.Day.WEEKDAY)};
  const model: any = {reports: [definition], accountModel: null,
    scopeModel: null};
  const controller = new EditScheduledReportController({model, id: 'test'});
  const state = {...controller.state, loaded: true, original, value: original};
  for(const change of [{repeats: false}, {repeatMonth: 3},
      {repeatInterval: new Interval(2, Interval.Unit.YEAR)},
      {repeatRule: new OrdinalDayOfMonthDateRule(0,
        OrdinalDayOfMonthDateRule.Position.FIRST,
        OrdinalDayOfMonthDateRule.Day.WEEKDAY)}]) {
    Object.assign(controller, {state: {...state,
      value: {...original, ...change}}});
    assert.ok(!controller.render().props.submitDisabled);
    Object.assign(controller, {state: {...state, value: {...original,
      repeatInterval: new Interval(1, Interval.Unit.YEAR),
      repeatRule: new OrdinalDayOfMonthDateRule(0,
        OrdinalDayOfMonthDateRule.Position.LAST,
        OrdinalDayOfMonthDateRule.Day.WEEKDAY)}}});
    assert.ok(controller.render().props.submitDisabled);
  }
});

it('ignores_absent_months_and_inactive_recurrence_values', () => {
  const definition = new ReportDefinition('test', 'Test', '', [], null);
  const model: any = {reports: [definition], accountModel: null,
    scopeModel: null};
  const controller = new EditScheduledReportController({model, id: 'test'});
  for(const repeats of [false, true]) {
    const original = {...ReportFormTemplate.makeValue(definition),
      scheduled: true, repeats};
    let repeatInterval = null;
    if(repeats) {
      original.repeatInterval = new Interval(1, Interval.Unit.MONTH);
      repeatInterval = new Interval(1, Interval.Unit.MONTH);
    }
    const value: ReportFormTemplate.Value = {...original, repeatInterval,
      repeatMonth: null, repeatRule: null};
    Object.assign(controller, {state: {...controller.state, loaded: true,
      original, value}});
    assert.ok(controller.render().props.submitDisabled);
  }
});
