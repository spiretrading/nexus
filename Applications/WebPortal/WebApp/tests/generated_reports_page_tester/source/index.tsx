import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { GeneratedReportsController, GeneratedReportsModel,
  LocalAccountGroupQueryModel, LocalGeneratedReportsModel,
  ReportTable } from 'web_portal';

class DelayedGeneratedReportsModel extends LocalGeneratedReportsModel {
  public async query(submission: GeneratedReportsModel.Submission):
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
    return super.query(submission);
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
  revision: number;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {revision: 0};
  }

  public render(): JSX.Element {
    return <div style={STYLE.wrapper} onClick={this.onClick}>
      <div style={STYLE.controls}>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <label><input type='checkbox' onChange={this.onEmpty}/>
          Empty results</label>
        <label><input type='checkbox' onChange={this.onFailDeletions}/>
          Fail deletions</label>
      </div>
      <GeneratedReportsController key={this.state.revision} model={model}
        onNewReport={this.onNewReport}
        onActionError={this.onError}/>
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

  private onEmpty = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.empty = event.target.checked;
    this.reload();
  };

  private onFailDeletions = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failDeletions = event.target.checked;
  };

  private onNewReport = () => {
    console.log('New Report');
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/"]');
    if(link) {
      event.preventDefault();
      console.log('Open Report', link.getAttribute('href'));
    }
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function makeReports(count: number): ReportTable.Report[] {
  const today = new Date();
  return Array.from({length: count}, (_, index) => {
    const date = new Date(today.getFullYear(), today.getMonth(),
      today.getDate() - index);
    return {id: String(index + 1),
      type: ['Profit and Loss', 'Trading Volume'][index % 2],
      parameters: [`Alpha Group ${index + 1}`, 'Canada', 'Month to Date'],
      url: `/reports/${index + 1}`,
      dateCreated: new Beam.DateTime(new Beam.Date(
        date.getFullYear(), date.getMonth() + 1, date.getDate()))};
  });
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failQueries: false, failDeletions: false,
  empty: false};
const model = new DelayedGeneratedReportsModel(makeReports(105),
  new LocalAccountGroupQueryModel([
    Beam.DirectoryEntry.makeAccount(1, 'Alice'),
    Beam.DirectoryEntry.makeAccount(2, 'Bob'),
    Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]));
ReactDOM.render(<App/>, document.getElementById('main'));
