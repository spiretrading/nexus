import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { ActivityTable, LocalReportActivityModel, ReportActivityController,
  ReportActivityModel, ReportActivityStatusTag } from 'web_portal';

class DelayedReportActivityModel extends LocalReportActivityModel {
  public async query(submission: ReportActivityModel.Submission):
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
    return super.query(submission);
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

interface State {
  revision: number;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {revision: 0};
  }

  public render(): JSX.Element {
    return <div style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <label><input type='checkbox' onChange={this.onEmpty}/>
          Empty results</label>
        <label><input type='checkbox' onChange={this.onFailActions}/>
          Fail cancellations</label>
        <label><input type='checkbox' onChange={this.onFailRetries}/>
          Fail retries</label>
      </div>
      <ReportActivityController key={this.state.revision} model={model}
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
    settings.failDeletions = event.target.checked;
  };

  private onFailRetries = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failRetries = event.target.checked;
  };

  private onNewReport = () => {
    console.log('New Report');
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function makeActivities(count: number): ActivityTable.Activity[] {
  const today = new Date();
  return Array.from({length: count}, (_, index) => {
    const date = new Date(today.getFullYear(), today.getMonth(),
      today.getDate() - index);
    return {id: String(index + 1),
      type: ['Profit and Loss', 'Trading Volume'][index % 2],
      parameters: [`Alpha Group ${index + 1}`, 'Canada', 'Month to Date'],
      dateModified: new Beam.DateTime(new Beam.Date(
        date.getFullYear(), date.getMonth() + 1, date.getDate())),
      status: (() => {
        if(index % 2 === 0) {
          return ReportActivityStatusTag.Status.GENERATING;
        }
        return ReportActivityStatusTag.Status.FAILED;
      })()};
  });
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failQueries: false, failDeletions: false,
  failRetries: false, empty: false};
const model = new DelayedReportActivityModel(makeActivities(105));
ReactDOM.render(<App/>, document.getElementById('main'));
