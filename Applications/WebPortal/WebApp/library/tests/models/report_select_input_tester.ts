import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as React from 'react';
import { InputGroup } from '../../source/components/input_group';
import { ReportParameterDefinition } from
  '../../source/pages/report_page/report_parameter_definition';
import { ReportParameterInput } from
  '../../source/pages/report_page/report_parameter_input';

describe('ReportSelectInput', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('input_group_waits_for_select_change_before_validation', async () => {
    const calls: string[] = [];
    const group = new InputGroup({label: 'Time',
      children: React.createElement('select', {
        onChange: () => calls.push('change')}),
      onValidate: () => calls.push('validate')});
    group.setState = () => calls.push('render');
    group.componentDidMount();
    calls.length = 0;
    const content = group.render().props.children.props;
    content.onInput({target: {tagName: 'SELECT'}});
    await Promise.resolve();
    assert.deepEqual(calls, []);
    content.input.props.onChange('1');
    assert.deepEqual(calls, ['change']);
    await Promise.resolve();
    assert.deepEqual(calls, ['change', 'render', 'validate']);
    calls.length = 0;
    content.onInput({target: {tagName: 'INPUT'}});
    await Promise.resolve();
    assert.deepEqual(calls, ['render', 'validate']);
    group.componentWillUnmount();
  });

  it('report_parameter_waits_for_select_change_before_validation',
      async () => {
    const calls: string[] = [];
    const parameter = new ReportParameterInput({
      definition: new ReportParameterDefinition('time', 'Time', 'Time', true),
      value: null, accountModel: null, scopeModel: null,
      onChange: () => calls.push('change'),
      onValidationChange: () => calls.push('validate')});
    parameter.componentDidMount();
    const field = parameter.render();
    field.props.onInput({target: {tagName: 'SELECT'}});
    await Promise.resolve();
    assert.deepEqual(calls, []);
    field.props.children.props.children.props.onChange(null);
    assert.deepEqual(calls, ['validate', 'change']);
    parameter.componentWillUnmount();
  });
});
