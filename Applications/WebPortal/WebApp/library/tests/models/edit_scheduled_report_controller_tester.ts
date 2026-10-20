import * as assert from 'node:assert/strict';
import { it } from 'node:test';
import * as Beam from 'beam';
import { DateRange, DateRangeRules, DayOffsetDateRule, SpecificDateRule } from
  '../../source/models';
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
