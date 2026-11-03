import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { ReportDefinition, ReportOutputDefinition, ReportParameterDefinition }
  from '../../source/pages/report_page';

describe('ReportDefinition', () => {
  it('parses_and_serializes_report_metadata', () => {
    const json = {id: 'profit_and_loss', name: 'Profit and Loss',
      description: 'Profit and loss for an account or group.',
      parameters: [
        {name: 'account', label: 'Account / Group', type: 'DirectoryEntry',
          required: true},
        {name: 'period', label: 'Date Range', type: 'DateRange',
          required: true},
        {name: 'currency', label: 'Currency', type: 'Currency',
          required: false, default: 'USD'}],
      output: {media_type: 'text/csv', extension: 'csv'}};
    const definition = ReportDefinition.fromJson(JSON.parse(
      JSON.stringify({...json, command: {executable: '/reports/profit'},
        access: ['Risk Managers']})));
    assert.equal(definition.id, 'profit_and_loss');
    assert.equal(definition.name, 'Profit and Loss');
    assert.equal(definition.description, json.description);
    assert.deepEqual(definition.parameters.map(parameter => parameter.name),
      ['account', 'period', 'currency']);
    assert.equal(definition.parameters[0].type, 'DirectoryEntry');
    assert.equal(definition.parameters[0].required, true);
    assert.equal(definition.parameters[2].defaultValue, 'USD');
    assert.equal(definition.output.mediaType, 'text/csv');
    assert.equal(definition.output.extension, 'csv');
    assert.deepEqual(definition.toJson(), json);
  });

  it('preserves_defaults_and_absence', () => {
    const defaults: unknown[] = [false, 0, '', null, [],
      {start: '2026-09-01', end: '2026-09-30'}];
    for(const defaultValue of defaults) {
      const json = {name: 'value', label: 'Value', type: 'SharedType',
        required: false, default: defaultValue};
      const parameter = ReportParameterDefinition.fromJson(json);
      assert.deepEqual(parameter.defaultValue, defaultValue);
      assert.deepEqual(JSON.parse(JSON.stringify(parameter.toJson())), json);
    }
    const absent = ReportParameterDefinition.fromJson({name: 'value',
      label: 'Value', type: 'Scope'});
    assert.equal(absent.required, false);
    assert.equal(absent.defaultValue, undefined);
    assert.equal(Object.hasOwn(absent.toJson(), 'default'), false);
  });

  it('constructs_parameterless_and_local_definitions', () => {
    const empty = ReportDefinition.fromJson({id: 'archive', name: 'Archive',
      parameters: [],
      output: {media_type: 'application/zip', extension: 'zip'}});
    assert.equal(empty.description, '');
    assert.deepEqual(empty.parameters, []);
    assert.deepEqual(empty.output.toJson(),
      {media_type: 'application/zip', extension: 'zip'});
    const parameters = [new ReportParameterDefinition('scope', 'Scope',
      'Scope', true)];
    const definition = new ReportDefinition('trades', 'Trades', '', parameters,
      new ReportOutputDefinition('application/json', 'json'));
    parameters.push(new ReportParameterDefinition('currency', 'Currency',
      'Currency', false, 'USD'));
    assert.equal(definition.parameters.length, 1);
    const copy = ReportDefinition.fromJson(definition.toJson());
    assert.deepEqual(copy.toJson(), definition.toJson());
  });
});
