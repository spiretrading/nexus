import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { LocalAccountGroupQueryModel, LocalTickerQueryModel, ReportDefinition,
  ReportFormTemplate, ScopeQueryModel } from 'web_portal';

interface Properties {

  /** The displayed form heading. */
  title: string;

  /** Whether to create or edit a report. */
  mode: ReportFormTemplate.Mode;

  /** Whether to display the Run fieldset. */
  showRuntime: boolean;

  /** Reports the submitted form values. */
  onSubmit?: (value: ReportFormTemplate.Value) => void;
}

/** Demonstrates reports using the shared parameter types and local lookups. */
export class ReportFormTemplateExample extends
    React.Component<Properties, {value: ReportFormTemplate.Value}> {
  constructor(props: Properties) {
    super(props);
    const alice = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    const group = Beam.DirectoryEntry.makeDirectory(2, 'Alpha Group');
    this.accounts = new LocalAccountGroupQueryModel([alice, group]);
    this.scopes = new ScopeQueryModel(new LocalTickerQueryModel([
      Nexus.Ticker.parse('ABX.TSX'), Nexus.Ticker.parse('BMO.TSX')]));
    const today = Beam.Date.today();
    this.reports = [ReportDefinition.fromJson({id: 'profit_and_loss',
      name: 'Profit and Loss', description: '', parameters: [
        {name: 'accounts', label: 'Account / Group', type: 'DirectoryEntryList',
          required: true, default: [group.toJson()]},
        {name: 'scope', label: 'Scope', type: 'Scope', required: true,
          default: '*'},
        {name: 'period', label: 'Date Range', type: 'DateRange', required: true,
          default: {start: today.toJson(), end: today.toJson()}},
        {name: 'currency', label: 'Currency', type: 'Currency', required: true,
          default: 'USD'}],
      output: {media_type: 'text/csv', extension: 'csv'}}),
      ReportDefinition.fromJson({id: 'parameter_types',
        name: 'Parameter Types', description: '', parameters: [
          {name: 'account', label: 'Account', type: 'DirectoryEntry',
            required: true, default: alice.toJson()},
          {name: 'date', label: 'Date', type: 'Date', required: true,
            default: today.toJson()},
          {name: 'datetime', label: 'Date & Time', type: 'DateTime',
            required: true, default: Beam.DateTime.now().toJson()},
          {name: 'time', label: 'Time', type: 'Time', required: true,
            default: Beam.Duration.HOUR.multiply(9).toJson()},
          {name: 'decimal', label: 'Decimal', type: 'Decimal', required: true,
            default: 1.5},
          {name: 'integer', label: 'Integer', type: 'Integer', required: true,
            default: 0},
          {name: 'money', label: 'Money', type: 'Money', required: true,
            default: '10.25'},
          {name: 'optional', label: 'Optional Value', type: 'Decimal'}],
        output: {media_type: 'application/json', extension: 'json'}})];
    this.state = {value: ReportFormTemplate.makeValue(this.reports[0])};
  }

  public render(): JSX.Element {
    return <ReportFormTemplate {...this.props} reports={this.reports}
      value={this.state.value} accountModel={this.accounts}
      scopeModel={this.scopes} onChange={this.onChange}/>;
  }

  private onChange = (value: ReportFormTemplate.Value) => {
    this.setState({value});
  };

  private accounts: LocalAccountGroupQueryModel;
  private scopes: ScopeQueryModel;
  private reports: readonly ReportDefinition[];
}
