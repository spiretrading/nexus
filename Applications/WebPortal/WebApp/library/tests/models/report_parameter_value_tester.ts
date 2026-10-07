import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { DateRange, ValidationError } from '../../source/models';
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
