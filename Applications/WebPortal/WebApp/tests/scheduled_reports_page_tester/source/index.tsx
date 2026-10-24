import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { LocalScheduledReportsModel, ScheduledReportsController,
  ScheduledReportsModel } from 'web_portal';

class DemoScheduledReportsModel extends LocalScheduledReportsModel {
  public async query(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    const fail = settings.failQueries;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated query failure.');
    }
    if(settings.empty) {
      return {status: ScheduledReportsModel.ResponseStatus.READY,
        isEmpty: true, filteredCount: 0, schedules: []};
    }
    return super.query(submission);
  }

  public async duplicate(id: string): Promise<ScheduledReportsModel.Schedule> {
    const fail = settings.failActions;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated duplication failure.');
    }
    const copy = await super.duplicate(id);
    console.log('Duplicate', id, copy.id);
    return copy;
  }

  public async delete(id: string): Promise<void> {
    const fail = settings.failActions;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated deletion failure.');
    }
    await super.delete(id);
    console.log('Delete', id);
  }

  public async run(id: string): Promise<void> {
    console.log('Run Now', id);
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
        <label><input type='checkbox' onChange={this.onFailActions}/>
          Fail actions</label>
      </div>
      <ScheduledReportsController key={this.state.revision} model={model}
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

  private onFailActions = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failActions = event.target.checked;
  };

  private onNewReport = () => {
    console.log('New Report');
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/edit/"]');
    if(link) {
      event.preventDefault();
      console.log('Edit Scheduled Report', link.getAttribute('href'));
    }
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function makeSchedules(count: number): ScheduledReportsModel.Schedule[] {
  const date = new Date();
  date.setDate(date.getDate() + 17);
  const runDate = {value: `${date.getFullYear()}-` +
      `${String(date.getMonth() + 1).padStart(2, '0')}-` +
      String(date.getDate()).padStart(2, '0'),
    label: date.toLocaleDateString('en-US', {
      month: 'short', day: '2-digit', year: 'numeric'})};
  return Array.from({length: count}, (_, index) => ({id: String(index + 1),
    type: ['Profit and Loss', 'Trading Volume'][index % 2],
    repeats: index % 2 === 0, runDate,
    parameters: [
      {label: 'Account / Group', value: `Alpha Group ${index + 1}`},
      {label: 'Scope', value: 'Canada'},
      {label: 'Date Range', value: 'Month to Date'}]}));
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failQueries: false, failActions: false,
  empty: false};
const model = new DemoScheduledReportsModel(makeSchedules(105));
ReactDOM.render(<App/>, document.getElementById('main'));
