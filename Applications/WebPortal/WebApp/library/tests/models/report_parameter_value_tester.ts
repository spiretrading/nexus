import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { DateRange, DateRangeRules, DateRuleType, DayOffsetDateRule,
  ValidationError } from '../../source/models';
import { ReportParameterInput } from
  '../../source/pages/report_page/report_parameter_input';
import { copyReportFormValue, parseReportFormValue, reportFormValueToJson }
  from '../../source/pages/report_page/report_form_value';
import { parseReportParameterValue, reportParameterValueToJson,
  ReportDefinition, ReportFormTemplate, ReportParameterDefinition,
  validateReportParameter } from '../../source/pages/report_page';

describe('ReportParameterValue', () => {
  it('parses_shared_types_and_round_trips', () => {
    const account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    const cases: [string, any][] = [
      ['DirectoryEntry', account.toJson()],
      ['DirectoryEntryList', [account.toJson()]],
      ['Date', '20261002'],
      ['DateRange', {start: '20260901', end: '20260930'}],
      ['DateTime', new Beam.DateTime(new Beam.Date(2026, 10, 2),
        Beam.Duration.HOUR.multiply(9)).toJson()],
      ['Time', Beam.Duration.HOUR.multiply(9).toJson()],
      ['Scope', Nexus.Scope.GLOBAL.toJson()],
      ['Decimal', 0], ['Integer', 10],
      ['Money', Nexus.Money.ONE.toJson()],
      ['Currency', Nexus.buildCurrencyDatabase().fromCode('USD').currency.
        toJson()]];
    for(const [type, json] of cases) {
      const value = parseReportParameterValue(type, json);
      assert.deepEqual(reportParameterValueToJson(value), json);
      assert.equal(validateReportParameter(new ReportParameterDefinition(
        'value', 'Value', type, true), value), ValidationError.NONE);
    }
    assert.equal((parseReportParameterValue('Scope', '*') as Nexus.Scope).
      isGlobal, true);
    assert.equal((parseReportParameterValue('Money', '10.25') as Nexus.Money).
      toString(), '10.25');
    assert.equal((parseReportParameterValue('Date', '2026-10-02') as Beam.Date).
      equals(new Beam.Date(2026, 10, 2)), true);
    const currency = parseReportParameterValue('Currency', 'USD') as
      Nexus.Currency;
    assert.equal(currency.equals(
      Nexus.buildCurrencyDatabase().fromCode('USD').currency), true);
  });

  it('preserves_date_rules_in_submissions_and_saved_forms', () => {
    const report = new ReportDefinition('test', 'Test', '', [
      new ReportParameterDefinition('period', 'Period', 'DateRange', true)],
      null);
    const cases = [
      {type: 'SPECIFIC_DATE', value: {date: '20261007'}},
      {type: 'DAY_OFFSET', value: {offset: 0}},
      {type: 'WEEKDAY', value: {offset: -2, day: 'FRIDAY'}},
      {type: 'DAY_OF_MONTH', value: {offset: 1, day: 31}},
      {type: 'MONTH_BOUNDARY', value: {offset: 0, boundary: 'LAST',
        day_offset: -1}}];
    for(const start of cases) {
      const json = {start: '20261007', end: '20261007',
        rules: {start, end: cases[1]}};
      const value = parseReportParameterValue('DateRange', json) as DateRange;
      assert.deepEqual(reportParameterValueToJson(value), json);
      const form = {...ReportFormTemplate.makeValue(report),
        parameters: {period: value}};
      const request = reportFormValueToJson(form);
      assert.deepEqual(request.parameters.period.rules, json.rules);
      const loaded = parseReportFormValue(request, [report]);
      assert.deepEqual(reportFormValueToJson(loaded).parameters.period,
        request.parameters.period);
      const copy = copyReportFormValue(loaded, [report]);
      assert.deepEqual(reportFormValueToJson(copy).parameters.period,
        request.parameters.period);
      assert.notEqual((copy.parameters.period as DateRange).rules,
        (loaded.parameters.period as DateRange).rules);
    }
  });

  it('submits_untouched_presets_and_saved_relative_dates_together', () => {
    const today = Beam.Date.today();
    const rules = new DateRangeRules(
      new DayOffsetDateRule(0),
      new DayOffsetDateRule(0));
    const report = new ReportDefinition('test', 'Test', '',
      ['preset', 'saved'].map(name => new ReportParameterDefinition(
        name, name, 'DateRange', true)), null);
    const form = {...ReportFormTemplate.makeValue(report), parameters: {
      preset: new DateRange(today, today),
      saved: new DateRange(new Beam.Date(2000, 1, 1),
        new Beam.Date(2000, 1, 1), rules)}};
    const request = reportFormValueToJson(form);
    for(const name of ['preset', 'saved']) {
      assert.deepEqual(request.parameters[name], {
        start: today.toJson(), end: today.toJson(),
        rules: {start: {type: 'DAY_OFFSET', value: {offset: 0}},
          end: {type: 'DAY_OFFSET', value: {offset: 0}}}});
    }
    const copy = copyReportFormValue(form, [report]);
    assert.deepEqual(reportFormValueToJson(copy), request);
    const saved = {...request, parameters: {...request.parameters,
      saved: {...request.parameters.saved,
        start: '20000101', end: '20000101'}}};
    const loaded = parseReportFormValue(saved, [report]);
    assert.ok((loaded.parameters.saved as DateRange).start.equals(today));
    assert.deepEqual(reportFormValueToJson(loaded), request);
  });

  it('commits_date_rules_when_the_resolved_dates_do_not_change', () => {
    const values: DateRange[] = [];
    const today = Beam.Date.today();
    const input = new ReportParameterInput({
      definition: new ReportParameterDefinition(
        'period', 'Period', 'DateRange', true),
      value: new DateRange(today, today), accountModel: null,
      scopeModel: null, onChange: value => values.push(value as DateRange),
      onValidationChange: () => {}});
    const props = input.render().props;
    const rules = new DateRangeRules(
      new DayOffsetDateRule(0),
      new DayOffsetDateRule(0));
    props.onValidationChange({valid: true});
    props.onRulesChange(rules);
    assert.equal(values.length, 1);
    assert.equal(values[0].rules, rules);
    assert.ok(values[0].start.equals(today));
    assert.ok(values[0].end.equals(today));
    props.onValidationChange({valid: false});
    props.onRulesChange(new DateRangeRules(new DayOffsetDateRule(null),
      rules.end));
    assert.equal(values.length, 1);
    const saved = new ReportParameterInput({...input.props, value: values[0]});
    assert.equal(saved.render().props.rules, rules);
  });

  it('validates_required_values_ranges_and_entry_lists', () => {
    const parameter = (type: string, required: boolean) =>
      new ReportParameterDefinition('value', 'Value', type, required);
    assert.equal(validateReportParameter(parameter('Integer', true), 0),
      ValidationError.NONE);
    assert.equal(validateReportParameter(parameter('Integer', true), null),
      ValidationError.REQUIRED);
    assert.equal(validateReportParameter(parameter('Integer', false), null),
      ValidationError.NONE);
    assert.equal(validateReportParameter(parameter('Integer', true), 1.5),
      ValidationError.FORMAT);
    const partial = new DateRange(new Beam.Date(2026, 10, 2), null);
    assert.equal(validateReportParameter(
      parameter('DateRange', false), partial), ValidationError.NONE);
    assert.equal(validateReportParameter(parameter('DateRange', true), partial),
      ValidationError.REQUIRED);
    assert.equal(validateReportParameter(parameter('DateRange', true),
      new DateRange(new Beam.Date(2026, 10, 2), new Beam.Date(2026, 10, 1))),
      ValidationError.OUT_OF_RANGE);
    const account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    assert.equal(validateReportParameter(parameter('DirectoryEntryList', true),
      [account, account]), ValidationError.DUPLICATE);
    assert.equal(validateReportParameter(parameter('DirectoryEntryList', false),
      [Beam.DirectoryEntry.INVALID]), ValidationError.FORMAT);
  });

  it('initializes_datetime_defaults', () => {
    const cases: [string, string, number][] = [
      ['2026-10-07T12:30:45', '20261007T123045', 45045000],
      ['20261007T123045', '20261007T123045', 45045000],
      ['2026-10-07T12:30:45.125', '20261007T123045.125', 45045125],
      ['20261007T123045.125', '20261007T123045.125', 45045125]];
    for(const [input, expected, ticks] of cases) {
      const report = ReportDefinition.fromJson({id: 'test', name: 'Test',
        parameters: [{name: 'timestamp', label: 'Timestamp', type: 'DateTime',
          required: true, default: input}],
        output: {media_type: 'text/csv', extension: 'csv'}});
      const form = ReportFormTemplate.makeValue(report);
      const value = form.parameters.timestamp as Beam.DateTime;
      assert.ok(value.date.equals(new Beam.Date(2026, 10, 7)));
      assert.equal(value.timeOfDay.ticks, ticks);
      assert.equal(validateReportParameter(report.parameters[0], value),
        ValidationError.NONE);
      assert.equal(reportParameterValueToJson(value), expected);
    }
    for(const value of [Beam.DateTime.POS_INFIN, Beam.DateTime.NEG_INFIN,
        Beam.DateTime.NOT_A_DATE_TIME]) {
      assert.equal(parseReportParameterValue('DateTime', value.toJson()),
        value);
    }
  });

  it('initializes_report_defaults', () => {
    const report = ReportDefinition.fromJson({id: 'test', name: 'Test',
      parameters: [{name: 'count', label: 'Count', type: 'Integer', default: 0},
        {name: 'currency', label: 'Currency', type: 'Currency', default: 'USD'},
        {name: 'amount', label: 'Amount', type: 'Money'}],
      output: {media_type: 'text/csv', extension: 'csv'}});
    const value = ReportFormTemplate.makeValue(report);
    assert.equal(value.reportType, 'test');
    assert.equal(value.parameters.count, 0);
    assert.equal(value.parameters.amount, null);
    assert.deepEqual(value.recipients, []);
    assert.equal(value.scheduled, false);
    assert.equal(value.repeats, false);
  });
});
