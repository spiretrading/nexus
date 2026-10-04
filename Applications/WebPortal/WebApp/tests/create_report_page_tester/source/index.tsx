import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { CreateReportController, LocalAccountGroupQueryModel,
  LocalCreateReportModel, LocalTickerQueryModel, ReportDefinition,
  ReportFormTemplate, ScopeQueryModel } from 'web_portal';

class DemoCreateReportModel extends LocalCreateReportModel {
  public async submit(value: ReportFormTemplate.Value): Promise<string> {
    const fail = settings.failCreation;
    await new Promise(resolve => window.setTimeout(resolve, settings.delay));
    if(fail) {
      throw new Error('Simulated report creation failure.');
    }
    const id = await super.submit(value);
    console.log('Create Report', id, value);
    return id;
  }
}

class App extends React.Component {
  public render(): JSX.Element {
    return <div style={STYLE.wrapper}>
      <div style={STYLE.controls}>
        <label>Delay (ms) <input type='number' defaultValue={settings.delay}
          min={0} onChange={this.onDelayChange}/></label>
        <label><input type='checkbox' onChange={this.onFailCreation}/>
          Fail creation</label>
      </div>
      <CreateReportController model={model}
        onCreated={this.onCreated}
        onError={this.onError}/>
    </div>;
  }

  private onDelayChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.delay = Math.max(0, Number(event.target.value));
  };

  private onFailCreation = (event: React.ChangeEvent<HTMLInputElement>) => {
    settings.failCreation = event.target.checked;
  };

  private onCreated = (scheduled: boolean, id: string) => {
    console.log('Created', scheduled, id);
  };

  private onError = (error: unknown) => {
    console.error(error);
  };
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

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {display: 'flex', flexDirection: 'column', width: '100%',
    height: '100%', fontFamily: 'Roboto, system-ui, sans-serif'},
  controls: {display: 'flex', flexWrap: 'wrap', gap: '12px', padding: '8px',
    backgroundColor: '#EEEEEE', font: '12px sans-serif'}
};
const settings = {delay: 750, failCreation: false};
const model = new DemoCreateReportModel(makeDefinitions(),
  new LocalAccountGroupQueryModel([
    Beam.DirectoryEntry.makeAccount(1, 'Alice'),
    Beam.DirectoryEntry.makeAccount(2, 'Bob'),
    Beam.DirectoryEntry.makeDirectory(3, 'Alpha Group')]),
  new ScopeQueryModel(new LocalTickerQueryModel([
    Nexus.Ticker.parse('ABX.TSX'), Nexus.Ticker.parse('BMO.TSX')])));
ReactDOM.render(<App/>, document.getElementById('main'));
