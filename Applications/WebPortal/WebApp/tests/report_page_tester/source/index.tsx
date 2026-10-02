import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { ActivityTable, CompositeReportModel, DateRange, GeneratedReportsModel,
  getScopeLabel, Interval, LocalCreateReportModel, LocalTickerQueryModel,
  LocalAccountGroupQueryModel, LocalEditScheduledReportModel,
  LocalGeneratedReportsModel,
  LocalReportActivityModel, LocalScheduledReportsModel, ReportActivityModel,
  ReportActivityStatusTag, ReportController, ReportDefinition,
  ReportFormTemplate, ReportParameterValue, ReportTable, ScopeQueryModel,
  ScheduledReportsModel } from 'web_portal';

class DelayedScheduledReportsModel extends LocalScheduledReportsModel {
  public async loadSchedules(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    await new Promise(resolve => window.setTimeout(resolve, 750));
    return super.loadSchedules(submission);
  }

  public async duplicate(id: string): Promise<ScheduledReportsModel.Schedule> {
    await editModel.load();
    const value = await editModel.loadReport(id);
    const schedule = await super.duplicate(id);
    editModel.set(schedule.id, value);
    return schedule;
  }

  public async delete(id: string): Promise<void> {
    await super.delete(id);
    await editModel.load();
    editModel.delete(id);
  }

  public async run(id: string): Promise<void> {
    console.log('Run Now', id);
  }
}

class DelayedGeneratedReportsModel extends LocalGeneratedReportsModel {
  public async loadReports(submission: GeneratedReportsModel.Submission):
      Promise<GeneratedReportsModel.Response> {
    const delay = settings.delay;
    const fail = settings.failQueries;
    await new Promise(resolve => window.setTimeout(resolve, delay));
    if(fail) {
      throw new Error('Simulated query failure.');
    }
    if(settings.empty) {
      return {status: GeneratedReportsModel.ResponseStatus.READY,
        isEmpty: true, filteredCount: 0, reports: []};
    }
    return super.loadReports(submission);
  }

  public async delete(ids: readonly string[]): Promise<void> {
    const fail = settings.failDeletions;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated deletion failure.');
    }
    await super.delete(ids);
    console.log('Delete', ids);
  }

  public async share(ids: readonly string[],
      recipients: readonly Beam.DirectoryEntry[]): Promise<void> {
    await super.share(ids, recipients);
    console.log('Share', ids, recipients);
  }

  public async download(ids: readonly string[]): Promise<void> {
    await super.download(ids);
    console.log('Download', ids);
  }
}

class DelayedReportActivityModel extends LocalReportActivityModel {
  public async loadActivities(submission: ReportActivityModel.Submission):
      Promise<ReportActivityModel.Response> {
    const fail = settings.failQueries;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated query failure.');
    }
    if(settings.empty) {
      return {status: ReportActivityModel.ResponseStatus.READY,
        isEmpty: true, totalCount: 0, activities: []};
    }
    return super.loadActivities(submission);
  }

  public async cancel(ids: readonly string[]): Promise<void> {
    const fail = settings.failDeletions;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated cancellation failure.');
    }
    await super.cancel(ids);
    console.log('Cancel', ids);
  }

  public async retry(ids: readonly string[]): Promise<void> {
    const fail = settings.failRetries;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated retry failure.');
    }
    await super.retry(ids);
    console.log('Retry', ids);
  }
}

class DemoCreateReportModel extends LocalCreateReportModel {
  public async submit(value: ReportFormTemplate.Value): Promise<string> {
    const fail = settings.failCreation;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated report creation failure.');
    }
    await super.submit(value);
    const report = this.reports.find(entry => entry.id === value.reportType);
    const parameters = report.parameters.map(parameter => ({
      label: parameter.label,
      value: formatParameter(parameter.type, value.parameters[parameter.name])
    }));
    console.log('Create Report', value);
    if(value.scheduled) {
      await scheduledModel.load();
      await editModel.load();
      const id = scheduledModel.add(makeSchedule(value));
      editModel.set(id, value);
      return id;
    }
    await activityModel.load();
    return activityModel.add({type: report.name,
      parameters: parameters.map(parameter => parameter.value),
      status: ReportActivityStatusTag.Status.GENERATING,
      dateModified: Beam.Date.today()});
  }
}

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
    await scheduledModel.load();
    scheduledModel.update({...makeSchedule(value), id});
    await super.submit(id, value);
    console.log('Edit Scheduled Report', id, value);
  }
}

interface State {
  page: ReportController.Page;
  scheduleId: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {page: ReportController.Page.SCHEDULED, scheduleId: null};
  }

  public render(): JSX.Element {
    return <div onClick={this.onClick} style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Test page <select value={this.state.page}
            onChange={this.onPageChange}>
          <option value={ReportController.Page.GENERATED}>Generated</option>
          <option value={ReportController.Page.SCHEDULED}>Scheduled</option>
          <option value={ReportController.Page.ACTIVITY}>Activity</option>
          <option value={ReportController.Page.CREATE}>Create</option>
          {this.state.page === ReportController.Page.EDIT_SCHEDULED &&
            <option value={ReportController.Page.EDIT_SCHEDULED}>
              Edit Scheduled Report
            </option>}
        </select></label>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <label><input type='checkbox' onChange={this.onFailDeletions}/>
          Fail deletions / cancellations</label>
        <label><input type='checkbox' onChange={this.onFailRetries}/>
          Fail retries</label>
        <label><input type='checkbox' onChange={this.onFailCreation}/>
          Fail creation</label>
        <label><input type='checkbox' onChange={this.onFailEdits}/>
          Fail edits</label>
        <label><input type='checkbox' onChange={this.onEmpty}/>
          Empty results</label>
      </div>
      <ReportController model={model} page={this.state.page}
        onNewReport={this.onNewReport} onActionError={this.onActionError}
        onNavigate={this.onNavigate} scheduleId={this.state.scheduleId}/>
    </div>;
  }

  private onPageChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.setState({page: Number(event.target.value)});
  };

  private onDelayChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.delay = Math.max(0, Number(event.target.value));
  };

  private onFailQueries = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failQueries = event.target.checked;
  };

  private onFailDeletions = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failDeletions = event.target.checked;
  };

  private onFailRetries = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failRetries = event.target.checked;
  };

  private onFailCreation = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failCreation = event.target.checked;
  };

  private onFailEdits = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failEdits = event.target.checked;
  };

  private onEmpty = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.empty = event.target.checked;
  };

  private onNewReport = () => {
    this.setState({page: ReportController.Page.CREATE});
  };

  private onNavigate = (page: ReportController.Page) => {
    this.setState({page});
  };

  private onActionError = (error: unknown) => {
    console.error(error);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/"]');
    if(link) {
      event.preventDefault();
      const path = link.getAttribute('href');
      if(path.startsWith('/reports/edit/')) {
        this.setState({page: ReportController.Page.EDIT_SCHEDULED,
          scheduleId: decodeURIComponent(path.slice('/reports/edit/'.length))});
      } else {
        console.log('Open Report', path);
      }
    }
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

function makeSchedule(value: ReportFormTemplate.Value):
    Omit<ScheduledReportsModel.Schedule, 'id'> {
  const report = definitions.find(entry => entry.id === value.reportType);
  const date = value.scheduleDateTime.date;
  return {type: report.name,
    parameters: report.parameters.map(parameter => ({label: parameter.label,
      value: formatParameter(parameter.type,
        value.parameters[parameter.name])})),
    repeats: value.repeats, runDate: {
      value: `${String(date.year).padStart(4, '0')}-` +
        `${String(date.month).padStart(2, '0')}-` +
        String(date.day).padStart(2, '0'),
      label: new Date(date.year, date.month - 1, date.day).
        toLocaleDateString('en-US', {
          month: 'short', day: '2-digit', year: 'numeric'})}};
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    display: 'flex',
    flexDirection: 'column',
    width: '100%',
    height: '100%'
  },
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};

function makeReports(count: number): ReportTable.Report[] {
  const today = new Date();
  return Array.from({length: count}, (_, index) => {
    const date = new Date(today.getFullYear(), today.getMonth(),
      today.getDate() - index);
    return {id: String(index + 1),
      type: ['Profit and Loss', 'Trading Volume'][index % 2],
      parameters: [`Alpha Group ${index + 1}`, 'Canada', 'Month to Date'],
      url: `/reports/${index + 1}`,
      dateCreated: new Beam.Date(date.getFullYear(), date.getMonth() + 1,
        date.getDate())};
  });
}

function makeActivities(count: number): ActivityTable.Activity[] {
  return makeReports(count).map((report, index) => ({id: report.id,
    type: report.type, parameters: report.parameters,
    dateModified: report.dateCreated,
    status: (() => {
      if(index % 2 === 0) {
        return ReportActivityStatusTag.Status.GENERATING;
      }
      return ReportActivityStatusTag.Status.FAILED;
    })()}));
}

function formatParameter(type: string, value: ReportParameterValue): string {
  if(value == null) {
    return '';
  }
  switch(type) {
    case 'DirectoryEntryList':
      return (value as Beam.DirectoryEntry[]).
        map(entry => entry.name).join(', ');
    case 'DateRange': {
      const range = value as DateRange;
      return `${range.start?.toString() ?? ''} - ` +
        `${range.end?.toString() ?? ''}`;
    }
    case 'Scope':
      return getScopeLabel(value as Nexus.Scope);
    case 'Currency':
      return Nexus.buildCurrencyDatabase().
        fromCurrency(value as Nexus.Currency).code;
    default:
      return value.toString();
  }
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

const settings = {delay: 750, failQueries: false, failDeletions: false,
  empty: false, failRetries: false, failCreation: false, failEdits: false};
const definitions = makeDefinitions();
const configurations = makeConfigurations(105);
const scheduledModel = new DelayedScheduledReportsModel(
  Array.from(configurations, ([id, value]) => ({...makeSchedule(value), id})));
const activityModel = new DelayedReportActivityModel(makeActivities(105));
const accounts = new LocalAccountGroupQueryModel([
  Beam.DirectoryEntry.makeAccount(1, 'Alice'),
  Beam.DirectoryEntry.makeAccount(2, 'Bob'),
  Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]);
const scopes = new ScopeQueryModel(new LocalTickerQueryModel([
  Nexus.Ticker.parse('ABX.TSX'), Nexus.Ticker.parse('BMO.TSX')]));
const createModel = new DemoCreateReportModel(definitions, accounts, scopes);
const editModel = new DemoEditScheduledReportModel(definitions, accounts,
  scopes, configurations);
const model = new CompositeReportModel(scheduledModel,
  new DelayedGeneratedReportsModel(makeReports(105), accounts),
  activityModel, createModel, editModel);
ReactDOM.render(<App/>, document.getElementById('main'));
