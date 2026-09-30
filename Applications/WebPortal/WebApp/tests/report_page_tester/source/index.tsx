import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { ScheduledReportsPage } from 'web_portal';

interface State {
  submission: ScheduledReportsPage.Submission;
  response: ScheduledReportsPage.Response;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      submission: {filters: {query: ''}, pageIndex: 0},
      response: {
        status: ScheduledReportsPage.ResponseStatus.IN_PROGRESS,
        isEmpty: false, filteredCount: 0, schedules: []
      }
    };
    this.schedules = makeSchedules(105);
    this.timer = null;
  }

  public render(): JSX.Element {
    return <div onClick={this.onClick} style={STYLE.wrapper}>
      <ScheduledReportsPage {...this.state.submission}
        response={this.state.response} onSubmit={this.onSubmit}
        onNewReport={this.onNewReport} onRun={this.onRun}
        onDuplicate={this.onDuplicate} onDelete={this.onDelete}/>
    </div>;
  }

  public componentDidMount(): void {
    this.load(this.state.submission);
  }

  public componentWillUnmount(): void {
    window.clearTimeout(this.timer);
  }

  private load(submission: ScheduledReportsPage.Submission): void {
    window.clearTimeout(this.timer);
    this.setState({submission, response: {
      ...this.state.response,
      status: ScheduledReportsPage.ResponseStatus.IN_PROGRESS
    }});
    this.timer = window.setTimeout(() => {
      const query = submission.filters.query.toLowerCase();
      const matches = this.schedules.filter(schedule =>
        [schedule.type, schedule.runDate.label,
          ...schedule.parameters.flatMap(parameter =>
            [parameter.label, parameter.value])].some(value =>
          value.toLowerCase().includes(query)));
      const start = submission.pageIndex * ScheduledReportsPage.PAGE_SIZE;
      this.setState({response: {
        status: ScheduledReportsPage.ResponseStatus.READY,
        isEmpty: this.schedules.length === 0,
        filteredCount: matches.length,
        schedules: matches.slice(start, start + ScheduledReportsPage.PAGE_SIZE)
      }});
    }, 750);
  }

  private onSubmit = (submission: ScheduledReportsPage.Submission) => {
    console.log('Submit', submission);
    this.load(submission);
  };

  private onNewReport = () => {
    console.log('New Report');
  };

  private onRun = (id: string) => {
    console.log('Run Now', id);
  };

  private onDuplicate = (id: string) => {
    console.log('Duplicate', id);
  };

  private onDelete = (id: string) => {
    console.log('Delete', id);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as Element).closest('a[href^="/reports/edit/"]');
    if(link) {
      event.preventDefault();
      console.log('Edit Report', link.getAttribute('href'));
    }
  };

  private schedules: ScheduledReportsPage.Schedule[];
  private timer: number;
}

function makeSchedules(count: number): ScheduledReportsPage.Schedule[] {
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

ReactDOM.render(<App/>, document.getElementById('main'));
