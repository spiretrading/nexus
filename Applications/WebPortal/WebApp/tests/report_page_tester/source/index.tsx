import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { CompositeReportModel, GeneratedReportsModel,
  LocalAccountGroupQueryModel, LocalGeneratedReportsModel,
  LocalScheduledReportsModel, ReportController, ReportTable,
  ScheduledReportsModel } from 'web_portal';

class DelayedScheduledReportsModel extends LocalScheduledReportsModel {
  public async loadSchedules(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    await new Promise(resolve => window.setTimeout(resolve, 750));
    return super.loadSchedules(submission);
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

interface State {
  page: ReportController.Page;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {page: ReportController.Page.GENERATED};
  }

  public render(): JSX.Element {
    return <div onClick={this.onClick} style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Test page <select value={this.state.page}
            onChange={this.onPageChange}>
          <option value={ReportController.Page.GENERATED}>Generated</option>
          <option value={ReportController.Page.SCHEDULED}>Scheduled</option>
        </select></label>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <label><input type='checkbox' onChange={this.onFailDeletions}/>
          Fail deletions</label>
        <label><input type='checkbox' onChange={this.onEmpty}/>
          Empty results</label>
      </div>
      <ReportController model={model} page={this.state.page}
        onNewReport={this.onNewReport} onActionError={this.onActionError}/>
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

  private onEmpty = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.empty = event.target.checked;
  };

  private onNewReport = () => {
    console.log('New Report');
  };

  private onActionError = (error: unknown) => {
    console.error(error);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/"]');
    if(link) {
      event.preventDefault();
      console.log('Open Report', link.getAttribute('href'));
    }
  };
}

function makeSchedules(count: number): ScheduledReportsModel.Schedule[] {
  const date = new Date();
  date.setDate(date.getDate() + 17);
  const runDate = {
    value: `${date.getFullYear()}-` +
      `${String(date.getMonth() + 1).padStart(2, '0')}-` +
      String(date.getDate()).padStart(2, '0'),
    label: date.toLocaleDateString('en-US', {
      month: 'short', day: '2-digit', year: 'numeric'
    })
  };
  return Array.from({length: count}, (_, index) => ({
    id: String(index + 1),
    type: ['Profit and Loss', 'Trading Volume'][index % 2],
    parameters: [
      {label: 'Account / Group', value: `Alpha Group ${index + 1}`},
      {label: 'Scope', value: 'Canada'},
      {label: 'Date Range', value: 'Month to Date'}
    ],
    repeats: index % 2 === 0,
    runDate
  }));
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

const settings = {delay: 750, failQueries: false, failDeletions: false,
  empty: false};
const model = new CompositeReportModel(
  new DelayedScheduledReportsModel(makeSchedules(105)),
  new DelayedGeneratedReportsModel(makeReports(105),
    new LocalAccountGroupQueryModel([
      Beam.DirectoryEntry.makeAccount(1, 'Alice'),
      Beam.DirectoryEntry.makeAccount(2, 'Bob'),
      Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')])));
ReactDOM.render(<App/>, document.getElementById('main'));
