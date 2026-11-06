import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { LocalReportDetailModel, ReportDetailController,
  ReportDetailModel, ReportViewer } from 'web_portal';
import SAMPLE_REPORT from './sample_report.pdf';

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
          <option value='4'>Sample PDF</option>
          <option value='5'>Missing PDF</option>
          <option value='6'>Invalid PDF</option>
          <option value='7'>Quoted CSV</option>
          <option value='8'>Large CSV</option>
          <option value='9'>Empty CSV</option>
          <option value='10'>Invalid CSV</option>
          <option value='11'>Missing CSV</option>
          <option value='12'>Markdown report</option>
          <option value='13'>JSON report</option>
          <option value='14'>Plain text report</option>
          <option value='15'>Invalid JSON</option>
          <option value='16'>Empty text</option>
          <option value='17'>Empty Markdown</option>
          <option value='18'>JSON scalar</option>
          <option value='19'>Missing Markdown</option>
          <option value='20'>Missing JSON</option>
          <option value='21'>Missing text</option>
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
  const reports = ['Profit and Loss', 'Trading Volume', 'Binary download'].
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
      return {id: String(index + 1), title, filePath, mediaType: file.type,
        status: ReportDetailModel.Status.READY,
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
  const invalid = URL.createObjectURL(new Blob(['Not a PDF.'],
    {type: 'application/pdf'}));
  const sample = URL.createObjectURL(new Blob([
    Uint8Array.from(atob(SAMPLE_REPORT.split(',')[1]),
      character => character.charCodeAt(0))], {type: 'application/pdf'}));
  downloads.push(invalid, sample);
  for(const [id, title, filePath] of [
      ['4', 'Sample PDF', sample],
      ['5', 'Missing PDF', 'missing-report.pdf'],
      ['6', 'Invalid PDF', invalid]]) {
    reports.push({id, title, filePath, mediaType: 'application/pdf',
      status: ReportDetailModel.Status.READY, parameters: [], content: null});
  }
  const large = ['Trade,Account,Ticker,Quantity,Price,Amount'];
  for(let i = 1; i <= 50000; ++i) {
    large.push(`${i},Account ${i % 100},ABC.TSX,${i % 1000},12.50,` +
      `${(i % 1000 * 12.5).toFixed(2)}`);
  }
  for(const [id, title, content] of [
      ['7', 'Quoted CSV', '\uFEFFName,Amount,Account,Notes,Value,Value,\r\n' +
        'Zulu,10,0007,"Comma, inside",first,second,\r\n' +
        'Alpha,2.5,0008,"Two\r\nlines",third,fourth,\r\n' +
        'Beta,-3,0009,"A ""quoted"" value",fifth,sixth,\r\n' +
        '<img src=x onerror=alert(1)>,,0010,=1+1,seventh,eighth,'],
      ['8', 'Large CSV', large.join('\n')],
      ['9', 'Empty CSV', ''],
      ['10', 'Invalid CSV', 'Name,Value\n"Unclosed,1']]) {
    const filePath = URL.createObjectURL(new Blob([content], {type: 'text/csv'}));
    downloads.push(filePath);
    reports.push({id, title, filePath, mediaType: 'text/csv',
      status: ReportDetailModel.Status.READY, parameters: [], content: null});
  }
  reports.push({id: '11', title: 'Missing CSV', filePath: 'missing-report.csv',
    mediaType: 'text/csv', status: ReportDetailModel.Status.READY,
    parameters: [], content: null});
  for(const [id, title, mediaType, content] of [
      ['12', 'Markdown report', 'text/markdown',
        '# Account summary\n\nTrading results for **Alpha Group**.\n\n' +
        '| Ticker | Quantity | Profit |\n| --- | ---: | ---: |\n' +
        '| ABC.TSX | 100 | $125.50 |\n| XYZ.TSX | 25 | -$30.00 |\n\n' +
        '## Notes\n\n- Values are in CAD.\n- Includes ~~pending~~ ' +
        '*completed* trades.\n\n- [x] Reconciled\n- [ ] Reviewed\n\n' +
        '> Prepared for the account owner.\n\n' +
        '```json\n{"account": "Alpha Group", "trades": 125}\n```\n\n' +
        '[Spire](https://spiretrading.com)\n\n' +
        '<script>alert("script")</script>\n' +
        '<span style="position:fixed;inset:0" onclick="alert(1)">' +
        'This text is safe.</span>\n\n[Unsafe link](javascript:alert(1))'],
      ['13', 'JSON report', 'application/json',
        '{"account":"Alpha Group","id":900719925474099312345,' +
        '"amount":123.4567890123456789,"negativeZero":-0,' +
        '"trades":[{"ticker":"ABC.TSX","quantity":100},' +
        '{"ticker":"XYZ.TSX","quantity":25}],"complete":true,' +
        '"error":null,"empty":{},"notes":"<img src=x onerror=alert(1)>"}'],
      ['14', 'Plain text report', 'text/plain',
        'Account statement\n\nAccount\tAlpha Group\n' +
        'Ticker      Quantity      Profit\n' +
        'ABC.TSX          100      125.50\n' +
        'XYZ.TSX           25      -30.00\n\n' +
        '<b>This is literal text.</b>\nUnicode: caf\u00e9 \u65e5\u672c\n'],
      ['15', 'Invalid JSON', 'application/json', '{"account":'],
      ['16', 'Empty text', 'text/plain', ''],
      ['17', 'Empty Markdown', 'text/markdown', ''],
      ['18', 'JSON scalar', 'application/json', 'null']]) {
    const filePath =
      URL.createObjectURL(new Blob([content], {type: mediaType}));
    downloads.push(filePath);
    reports.push({id, title, filePath, mediaType,
      status: ReportDetailModel.Status.READY, parameters: [], content: null});
  }
  for(const [id, title, mediaType, extension] of [
      ['19', 'Missing Markdown', 'text/markdown', 'md'],
      ['20', 'Missing JSON', 'application/json', 'json'],
      ['21', 'Missing text', 'text/plain', 'txt']]) {
    reports.push({id, title, mediaType, filePath: 'missing-report.' + extension,
      status: ReportDetailModel.Status.READY, parameters: [], content: null});
  }
  return reports;
}

function renderReportContent(report: ReportDetailModel.Report):
    React.ReactNode {
  return <ReportViewer report={report}/>;
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
