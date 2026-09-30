import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { LocalReportModel, LocalScheduledReportsModel, ReportController,
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

class App extends React.Component {
  public render(): JSX.Element {
    return <div onClick={this.onClick} style={STYLE.wrapper}>
      <ReportController model={model} onNewReport={this.onNewReport}
        onActionError={this.onActionError}/>
    </div>;
  }

  private onNewReport = () => {
    console.log('New Report');
  };

  private onActionError = (error: unknown) => {
    console.error(error);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/edit/"]');
    if(link) {
      event.preventDefault();
      console.log('Edit Report', link.getAttribute('href'));
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
    width: '100%',
    height: '100%'
  }
};

const model = new LocalReportModel(
  new DelayedScheduledReportsModel(makeSchedules(105)));
ReactDOM.render(<App/>, document.getElementById('main'));
