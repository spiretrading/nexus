import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as React from 'react';
import { AccountGroupQueryModel } from '../../source/components';
import { ReportDefinition, ReportFormTemplate, ShareReportModal } from
  '../../source/pages/report_page';
import { ReportRecipientQueryModel } from
  '../../source/pages/report_page/report_recipient_query_model';

class Accounts extends AccountGroupQueryModel {
  public queries: string[] = [];
  public account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
  public group = Beam.DirectoryEntry.makeDirectory(2, 'Group');

  public async submit(query: string): Promise<readonly Beam.DirectoryEntry[]> {
    this.queries.push(query);
    return [Beam.DirectoryEntry.STAR_DIRECTORY, this.account, this.group];
  }
}

function find(element: React.ReactElement, name: string): any {
  if((element.type as any).name === name) {
    return element.props;
  }
  for(const child of React.Children.toArray(element.props.children)) {
    if(React.isValidElement(child)) {
      const result = find(child, name);
      if(result) {
        return result;
      }
    }
  }
  return null;
}

describe('Report recipients', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('excludes_global_recipients_and_preserves_query_semantics', async () => {
    const accounts = new Accounts();
    const recipients = new ReportRecipientQueryModel(accounts);
    assert.deepEqual(await recipients.submit(' any query '),
      [accounts.account, accounts.group]);
    assert.equal(accounts.queries[0], ' any query ');
    assert.equal(await recipients.parse(' * '), null);
    assert.equal(await recipients.parse('Alice'), accounts.account);
    assert.equal(await recipients.parse('Group'), accounts.group);
    assert.equal(await recipients.parse('missing'), null);
    assert.equal(await accounts.parse('*'), Beam.DirectoryEntry.STAR_DIRECTORY);
  });

  it('separates_sharing_from_parameters_and_rejects_global_submission', () => {
    const accounts = new Accounts();
    const report = ReportDefinition.fromJson({id: 'test', name: 'Test',
      parameters: [{name: 'account', label: 'Account', type: 'DirectoryEntry',
        required: true, default: Beam.DirectoryEntry.STAR_DIRECTORY.toJson()}],
      output: {media_type: 'text/csv', extension: 'csv'}});
    const props: React.ComponentProps<typeof ReportFormTemplate> = {
      title: 'Create', reports: [report],
      value: ReportFormTemplate.makeValue(report), accountModel: accounts,
      scopeModel: null};
    const form = new ReportFormTemplate(props);
    const rendered = form.render();
    assert.equal(find(rendered, 'ParametersFieldset').accountModel, accounts);
    const recipients = find(rendered, 'ShareFieldset').accountModel;
    assert.ok(recipients instanceof ReportRecipientQueryModel);
    assert.ok(find(rendered, 'SubmitSection').valid);
    assert.equal(find(form.render(), 'ShareFieldset').accountModel, recipients);
    props.value.recipients = [Beam.DirectoryEntry.STAR_DIRECTORY];
    assert.equal(find(form.render(), 'SubmitSection').valid, false);
    props.value.recipients = [accounts.account, accounts.group];
    assert.ok(find(form.render(), 'SubmitSection').valid);
    props.accountModel = new Accounts();
    assert.equal(find(form.render(), 'ShareFieldset').accountModel.source,
      props.accountModel);
  });

  it('restricts_modal_parsing_and_submission_with_a_stable_model', async () => {
    const props = {model: new Accounts(),
      selected: [] as Beam.DirectoryEntry[]};
    const modal = new ShareReportModal(props);
    const rendered = modal.render();
    const model = rendered.props.model;
    assert.equal(await model.parse('*'), null);
    assert.deepEqual(await model.submit('*'),
      [props.model.account, props.model.group]);
    assert.equal(rendered.props.canSubmit([]), false);
    assert.equal(rendered.props.canSubmit([Beam.DirectoryEntry.STAR_DIRECTORY]),
      false);
    assert.ok(rendered.props.canSubmit([props.model.account]));
    assert.equal(modal.render().props.model, model);
    props.model = new Accounts();
    assert.equal(modal.render().props.model.source, props.model);
  });
});
