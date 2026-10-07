import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { EditScheduledReportController, Interval, LocalAccountGroupQueryModel,
  LocalEditScheduledReportModel, LocalTickerQueryModel, ReportDefinition,
  ReportFormTemplate, ScopeQueryModel } from 'web_portal';

class DemoEditScheduledReportModel extends LocalEditScheduledReportModel {
  public async loadReport(id: string): Promise<ReportFormTemplate.Value> {
    const fail = settings.failQueries;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated schedule retrieval failure.');
    }
    return super.loadReport(id);
  }

  public async submit(id: string, value: ReportFormTemplate.Value):
      Promise<void> {
    const fail = settings.failEdits;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated schedule update failure.');
    }
    await super.submit(id, value);
    console.log('Edit Scheduled Report', id, value);
  }
}

interface State {
  revision: number;
  id: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {revision: 0, id: '1'};
  }

  public render(): JSX.Element {
    return <div style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label>Schedule <select value={this.state.id}
            onChange={this.onScheduleChange}>
          <option value='1'>Recurring Profit and Loss</option>
          <option value='2'>Trading Volume</option>
        </select></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <label><input type='checkbox' onChange={this.onFailEdits}/>
          Fail edits</label>
      </div>
      <EditScheduledReportController key={this.state.revision} model={model}
        id={this.state.id} onSaved={this.onSaved}
        onError={this.onError}/>
    </div>;
  }

  private reload(): void {
    this.setState(state => ({revision: state.revision + 1}));
  }

  private onDelayChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.delay = Math.max(0, Number(event.target.value));
  };

  private onFailQueries = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failQueries = event.target.checked;
    this.reload();
  };

  private onFailEdits = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failEdits = event.target.checked;
  };

  private onScheduleChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.setState({id: event.target.value});
  };

  private onSaved = () => {
    console.log('Saved', this.state.id);
    this.reload();
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function makeConfigurations(count: number):
    Map<string, ReportFormTemplate.Value> {
  const date = new Date();
  date.setDate(date.getDate() + 17);
  return new Map(Array.from({length: count}, (_, index) => {
    const value = ReportFormTemplate.makeValue(definitions[index % 2]);
    value.parameters = {...value.parameters,
      accounts: [Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]};
    value.scheduled = true;
    value.scheduleDateTime = new Beam.DateTime(new Beam.Date(date.getFullYear(),
      date.getMonth() + 1, date.getDate()), Beam.Duration.HOUR.multiply(9));
    value.repeats = index % 2 === 0;
    value.repeatInterval = new Interval(1, Interval.Unit.MONTH);
    return [String(index + 1), value];
  }));
}

function makeDefinitions(): ReportDefinition[] {
  const today = Beam.Date.today();
  return ['Profit and Loss', 'Trading Volume'].map((name, index) =>
    ReportDefinition.fromJson({id: `report_${index}`, name, description: '',
      parameters: [
        {name: 'accounts', label: 'Account / Group',
          type: 'DirectoryEntryList',
          required: true,
          default: [Beam.DirectoryEntry.STAR_DIRECTORY.toJson()]},
        {name: 'scope', label: 'Scope', type: 'Scope', required: true,
          default: '*'},
        {name: 'period', label: 'Date Range', type: 'DateRange',
          required: true,
          default: {start: today.toJson(), end: today.toJson()}},
        {name: 'currency', label: 'Currency', type: 'Currency', required: true,
          default: 'USD'}],
      output: {media_type: 'text/csv', extension: 'csv'}}));
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failQueries: false, failEdits: false};
const definitions = makeDefinitions();
const model = new DemoEditScheduledReportModel(definitions,
  new LocalAccountGroupQueryModel([
    Beam.DirectoryEntry.makeAccount(1, 'Alice'),
    Beam.DirectoryEntry.makeAccount(2, 'Bob'),
    Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]),
  new ScopeQueryModel(new LocalTickerQueryModel([
    Nexus.Ticker.parse('ABX.TSX'), Nexus.Ticker.parse('BMO.TSX')])),
  makeConfigurations(2));
ReactDOM.render(<App/>, document.getElementById('main'));
