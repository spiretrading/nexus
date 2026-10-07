import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { LocalReportDetailModel, ReportDetailController,
  ReportDetailModel } from 'web_portal';

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
        <label>Report <select value={this.state.id}
            onChange={this.onReportChange}>
          <option value='1'>Profit and Loss</option>
          <option value='2'>Trading Volume</option>
          <option value='3'>Binary download</option>
        </select></label>
        <label><input type='checkbox' onChange={this.onFailQueries}/>
          Fail queries</label>
        <button onClick={this.onReload}>Reload</button>
      </div>
      <ReportDetailController key={this.state.revision} model={model}
        id={this.state.id} renderContent={renderReportContent}
        onError={this.onError}/>
    </div>;
  }

  public componentWillUnmount(): void {
    for(const url of downloads) {
      URL.revokeObjectURL(url);
    }
  }

  private onReload = () => {
    this.setState(state => ({revision: state.revision + 1}));
  };

  private onDelayChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.delay = Math.max(0, Number(event.target.value));
  };

  private onFailQueries = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failQueries = event.target.checked;
  };

  private onReportChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.setState({id: event.target.value});
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function makeDetails(): ReportDetailModel.Report[] {
  return ['Profit and Loss', 'Trading Volume', 'Binary download'].
    map((title, index) => {
      const content = `Report,Account,Scope,Period\n` +
        `${title},Alpha Group ${index + 1},Canada,Month to Date\n`;
      const file = (() => {
        if(index === 2) {
          return new Blob([new Uint8Array([1, 2, 3, 4])],
            {type: 'application/octet-stream'});
        }
        return new Blob([content], {type: 'text/csv'});
      })();
      const filePath = URL.createObjectURL(file);
      downloads.push(filePath);
      return {id: String(index + 1), title, filePath,
        parameters: [
          {label: 'Account / Group', value: `Alpha Group ${index + 1}`},
          {label: 'Scope', value: 'Canada'},
          {label: 'Date Range', value: 'Month to Date'}],
        content: (() => {
          if(index !== 2) {
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

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failQueries: false};
const downloads: string[] = [];
const model = new DemoReportDetailModel(makeDetails());
ReactDOM.render(<App/>, document.getElementById('main'));
