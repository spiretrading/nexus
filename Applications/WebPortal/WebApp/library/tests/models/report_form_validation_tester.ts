import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { ValidationError } from '../../source/models';
import { ReportDefinition, ReportFormTemplate, ReportParameterDefinition } from
  '../../source/pages/report_page';
import { ReportParameterInput } from
  '../../source/pages/report_page/report_parameter_input';

function synchronousState(component: any): void {
  component.setState = (update: any) => {
    if(typeof update === 'function') {
      update = update(component.state, component.props);
    }
    component.state = {...component.state, ...update};
  };
}

describe('ReportFormValidation', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('enables_valid_drafts_and_revalidates_committed_submission', async () => {
    const report = ReportDefinition.fromJson({id: 'test', name: 'Test',
      description: '', parameters: [{name: 'count', label: 'Count',
        type: 'Integer', required: true}],
      output: {media_type: 'text/plain', extension: 'txt'}});
    const submissions: ReportFormTemplate.Value[] = [];
    const component = new ReportFormTemplate({title: 'Test', reports: [report],
      value: ReportFormTemplate.makeValue(report), accountModel: null,
      scopeModel: null, onSubmit: value => submissions.push(value)});
    synchronousState(component);
    const form = () => component.render().props.children;
    const enabled = () => form().props.children[6].props.valid;
    const validate = (error: ValidationError) =>
      form().props.children[2].props.onValidation('count', error);
    assert.equal(enabled(), false);
    validate(ValidationError.NONE);
    assert.equal(enabled(), true);
    validate(ValidationError.FORMAT);
    assert.equal(enabled(), false);
    validate(ValidationError.REQUIRED);
    assert.equal(enabled(), false);
    validate(ValidationError.NONE);
    const previousDocument = globalThis.document;
    let blur = () => {};
    globalThis.document = {activeElement: {blur: () => blur()}} as any;
    (component as any).mounted = true;
    try {
      form().props.onSubmit({preventDefault() {}});
      await Promise.resolve();
      assert.equal(submissions.length, 0);
      blur = () => queueMicrotask(() => {
        (component as any).props = {...component.props,
          value: {...component.props.value, parameters: {count: 42}}};
      });
      form().props.onSubmit({preventDefault() {}});
      await Promise.resolve();
      assert.equal(submissions.length, 1);
      assert.equal(submissions[0].parameters.count, 42);
      validate(ValidationError.FORMAT);
      form().props.onSubmit({preventDefault() {}});
      await Promise.resolve();
      assert.equal(submissions.length, 1);
    } finally {
      globalThis.document = previousDocument;
    }
  });

  it('validates_cleared_committed_parameters', () => {
    for(const required of [false, true]) {
      const errors: ValidationError[] = [];
      const parameter = new ReportParameterInput({definition:
        new ReportParameterDefinition('count', 'Count', 'Integer', required),
        value: 1, accountModel: null, scopeModel: null, onChange() {},
        onValidationChange: error => errors.push(error)});
      synchronousState(parameter);
      parameter.render().props.children.props.children.props.onChange(null);
      const expected = (() => {
        if(required) {
          return ValidationError.REQUIRED;
        }
        return ValidationError.NONE;
      })();
      assert.deepEqual(errors, [expected]);
      assert.equal(parameter.render().props.children.props.validation.error,
        expected);
    }
  });
});
