import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { HashRouter, Route, RouteComponentProps } from 'react-router-dom';
import { ActivityTable, CompositeReportModel, DateRange, GeneratedReportsModel,
  getScopeLabel, Interval, LocalCreateReportModel, LocalTickerQueryModel,
  LocalAccountGroupQueryModel, LocalEditScheduledReportModel,
  LocalGeneratedReportsModel, LocalReportDetailModel,
  LocalReportActivityModel, LocalScheduledReportsModel, ReportActivityModel,
  ReportActivityStatusTag, ReportController, ReportDefinition,
  ReportDetailModel,
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
    await detailModel.load();
    for(const id of ids) {
      detailModel.delete(id);
      URL.revokeObjectURL(downloads.get(id));
      downloads.delete(id);
    }
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

class DemoReportDetailModel extends LocalReportDetailModel {
  public async loadReport(id: string): Promise<ReportDetailModel.Report> {
    const fail = settings.failQueries;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated report retrieval failure.');
    }
    return super.loadReport(id);
  }
}

interface Location {
  page: ReportController.Page;
  scheduleId: string;
  reportId: string;
}

class App extends React.Component<RouteComponentProps> {
  public render(): JSX.Element {
    const location = parseLocation(this.props.location.pathname);
    return <div onClick={this.onClick} style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Test page <select value={location.page}
            onChange={this.onPageChange}>
          <option value={ReportController.Page.GENERATED}>Generated</option>
          <option value={ReportController.Page.SCHEDULED}>Scheduled</option>
          <option value={ReportController.Page.ACTIVITY}>Activity</option>
          <option value={ReportController.Page.CREATE}>Create</option>
          {location.page === ReportController.Page.EDIT_SCHEDULED &&
            <option value={ReportController.Page.EDIT_SCHEDULED}>
              Edit Scheduled Report
            </option>}
          {location.page === ReportController.Page.DETAIL &&
            <option value={ReportController.Page.DETAIL}>Report Detail</option>}
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
      <ReportController model={model} page={location.page}
        onNewReport={this.onNewReport} onActionError={this.onActionError}
        onNavigate={this.onNavigate} scheduleId={location.scheduleId}
        reportId={location.reportId}
        renderReportContent={renderReportContent}/>
    </div>;
  }

  public componentWillUnmount(): void {
    for(const url of downloads.values()) {
      URL.revokeObjectURL(url);
    }
  }

  private onPageChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.props.history.push(PAGE_PATHS[Number(event.target.value)]);
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
    this.props.history.push(PAGE_PATHS[ReportController.Page.CREATE]);
  };

  private onNavigate = (page: ReportController.Page) => {
    this.props.history.push(PAGE_PATHS[page]);
  };

  private onActionError = (error: unknown) => {
    console.error(error);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/"]');
    if(link) {
      event.preventDefault();
      this.props.history.push(link.getAttribute('href'));
    }
  };
}

function parseLocation(path: string): Location {
  const location: Location = {page: ReportController.Page.GENERATED,
    scheduleId: null, reportId: null};
  const page = Object.keys(PAGE_PATHS).find(key =>
    PAGE_PATHS[Number(key)] === path);
  if(page !== undefined) {
    location.page = Number(page);
  } else if(path.startsWith('/reports/edit/')) {
    location.page = ReportController.Page.EDIT_SCHEDULED;
    location.scheduleId =
      decodeURIComponent(path.slice('/reports/edit/'.length));
  } else if(path.startsWith('/reports/')) {
    location.page = ReportController.Page.DETAIL;
    location.reportId = decodeURIComponent(path.slice('/reports/'.length));
  }
  return location;
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

function makeDetails(reports: readonly ReportTable.Report[]):
    ReportDetailModel.Report[] {
  return reports.map((report, index) => {
    const content = `Report,Account,Scope,Period\n` +
      `${report.type},${report.parameters.join(',')}\n`;
    const downloadable = (() => {
      if(index % 3 === 2) {
        return new Blob([new Uint8Array([1, 2, 3, 4])],
          {type: 'application/octet-stream'});
      }
      return new Blob([content], {type: 'text/csv'});
    })();
    const filePath = URL.createObjectURL(downloadable);
    downloads.set(report.id, filePath);
    const parameters = ['Account / Group', 'Scope', 'Date Range'].
      map((label, i) => ({label, value: report.parameters[i]}));
    return {id: report.id, title: report.type, parameters, filePath,
      content: (() => {
        if(index % 3 !== 2) {
          return content;
        }
        return null;
      })()};
  });
}

function renderReportContent(report: ReportDetailModel.Report):
    React.ReactNode {
  if(report.content == null) {
    return null;
  }
  return <pre style={{margin: 0, whiteSpace: 'pre-wrap',
    overflowWrap: 'anywhere'}}>{report.content}</pre>;
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
const downloads = new Map<string, string>();
const generatedReports = makeReports(105);
const detailModel = new DemoReportDetailModel(makeDetails(generatedReports));
const model = new CompositeReportModel(scheduledModel,
  new DelayedGeneratedReportsModel(generatedReports, accounts),
  activityModel, createModel, editModel, detailModel);
const PAGE_PATHS: Record<number, string> = {
  [ReportController.Page.GENERATED]: '/reports',
  [ReportController.Page.SCHEDULED]: '/reports/scheduled',
  [ReportController.Page.ACTIVITY]: '/reports/activity',
  [ReportController.Page.CREATE]: '/reports/create'
};
ReactDOM.render(<HashRouter>
  <Route component={App}/>
</HashRouter>, document.getElementById('main'));
