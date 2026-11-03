import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { CreateReportController, dateRuleToJson, DayOffsetDateRule,
  LocalAccountGroupQueryModel, LocalCreateReportModel, LocalReportDetailModel,
  LocalTickerQueryModel, ReportDetailController, ReportDetailModel,
  ReportDefinition, ReportFormTemplate, reportParameterValueToJson,
  ScopeQueryModel } from 'web_portal';

class DemoCreateReportModel extends LocalCreateReportModel {
  public async submit(value: ReportFormTemplate.Value): Promise<string> {
    const fail = settings.failCreation;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated report creation failure.');
    }
    const id = await super.submit(value);
    if(!value.scheduled) {
      const definition = this.reports.find(report =>
        report.id === value.reportType);
      const parameters = Object.fromEntries(Object.entries(value.parameters).
        map(([name, parameter]) =>
          [name, reportParameterValueToJson(parameter)]));
      const content = JSON.stringify(parameters, null, 2);
      const filePath = URL.createObjectURL(new Blob([content],
        {type: 'application/json'}));
      downloads.push(filePath);
      await details.load();
      details.add({id, title: definition.name, filePath, content,
        status: ReportDetailModel.Status.IN_PROGRESS,
        parameters: definition.parameters.map(parameter => ({
          label: parameter.label,
          value: JSON.stringify(parameters[parameter.name]) ?? ''}))});
    }
    console.log('Create Report', id, value);
    return id;
  }
}

class DemoReportDetailModel extends LocalReportDetailModel {
  public add(report: ReportDetailModel.Report): void {
    this.store(report);
    this.jobs.set(report.id, {ready: Date.now() + settings.generationDelay,
      failed: settings.failGeneration});
  }

  public async loadReport(id: string): Promise<ReportDetailModel.Report> {
    const report = await super.loadReport(id);
    const job = this.jobs.get(id);
    if(report.status === ReportDetailModel.Status.IN_PROGRESS) {
      if(Date.now() < job.ready) {
        return {...report, filePath: '', content: null};
      }
      report.status = (() => {
        if(job.failed) {
          return ReportDetailModel.Status.FAILED;
        }
        return ReportDetailModel.Status.READY;
      })();
      this.store(report);
    }
    return report;
  }

  private jobs = new Map<string, {ready: number; failed: boolean}>();
}

interface State {
  id: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {id: null};
  }

  public render(): JSX.Element {
    return <div style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailCreation}/>
          Fail creation</label>
        <label>Generation (ms) <input type='number'
          defaultValue={settings.generationDelay} min={0}
          onChange={this.onGenerationDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailGeneration}/>
          Fail generation</label>
        {this.state.id && <button onClick={this.onBack}>Back to form</button>}
      </div>
      {(() => {
        if(this.state.id) {
          return <ReportDetailController model={details} id={this.state.id}
            renderContent={renderContent} onError={this.onError}/>;
        }
        return <CreateReportController model={model}
          onCreated={this.onCreated} onError={this.onError}/>;
      })()}
    </div>;
  }

  public componentDidMount(): void {
    window.history.replaceState({reportId: null}, '');
    window.addEventListener('popstate', this.onPopState);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('popstate', this.onPopState);
    for(const url of downloads) {
      URL.revokeObjectURL(url);
    }
  }

  private onGenerationDelayChange =
      (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.generationDelay = Math.max(0, Number(event.target.value));
  };

  private onFailGeneration = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failGeneration = event.target.checked;
  };

  private onBack = () => {
    window.history.back();
  };

  private onPopState = (event: PopStateEvent) => {
    this.setState({id: event.state?.reportId ?? null});
  };

  private onDelayChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.delay = Math.max(0, Number(event.target.value));
  };

  private onFailCreation = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failCreation = event.target.checked;
  };

  private onCreated = (scheduled: boolean, id: string) => {
    if(scheduled) {
      console.log('Navigate to Scheduled Reports', id);
      return;
    }
    const url = `#report/${encodeURIComponent(id)}`;
    window.history.pushState({reportId: id}, '', url);
    this.setState({id});
    console.log('Open Report', url);
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
}

function renderContent(report: ReportDetailModel.Report): React.ReactNode {
  return <pre style={{whiteSpace: 'pre-wrap', overflowWrap: 'anywhere'}}>
    {report.content}
  </pre>;
}

function makeDefinitions(): ReportDefinition[] {
  const today = Beam.Date.today();
  const rule = dateRuleToJson(new DayOffsetDateRule(0));
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
          default: {start: today.toJson(), end: today.toJson(),
            rules: {start: rule, end: rule}}},
        {name: 'currency', label: 'Currency', type: 'Currency', required: true,
          default: 'USD'}],
      output: {media_type: 'text/csv', extension: 'csv'}}));
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failCreation: false, generationDelay: 2500,
  failGeneration: false};
const downloads: string[] = [];
const details = new DemoReportDetailModel([]);
const model = new DemoCreateReportModel(makeDefinitions(),
  new LocalAccountGroupQueryModel([
    Beam.DirectoryEntry.makeAccount(1, 'Alice'),
    Beam.DirectoryEntry.makeAccount(2, 'Bob'),
    Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]),
  new ScopeQueryModel(new LocalTickerQueryModel([
    Nexus.Ticker.parse('ABX.TSX'), Nexus.Ticker.parse('BMO.TSX')])));
ReactDOM.render(<App/>, document.getElementById('main'));
