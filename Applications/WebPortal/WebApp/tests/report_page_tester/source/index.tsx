import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { HashRouter, Route, RouteComponentProps } from 'react-router-dom';
import { ReportPage } from 'web_portal';

class App extends React.Component<RouteComponentProps> {
  public render(): JSX.Element {
    const current = (() => {
      switch(this.props.location.pathname) {
        case '/activity':
          return ReportPage.Tab.ACTIVITY;
        case '/schedules':
          return ReportPage.Tab.SCHEDULES;
        default:
          return ReportPage.Tab.GENERATED;
      }
    })();
    return <div style={{display: 'flex', height: '100%'}}>
      <ReportPage current={current} onNavigate={this.onNavigate}/>
    </div>;
  }

  private onNavigate = (tab: ReportPage.Tab) => {
    const path = PATHS[tab];
    if(path !== this.props.location.pathname) {
      this.props.history.push(path);
    }
    console.log('Navigate', path);
  };
}

const PATHS: Record<ReportPage.Tab, string> = {
  [ReportPage.Tab.GENERATED]: '/generated',
  [ReportPage.Tab.ACTIVITY]: '/activity',
  [ReportPage.Tab.SCHEDULES]: '/schedules'
};
ReactDOM.render(<HashRouter>
  <Route component={App}/>
</HashRouter>, document.getElementById('main'));
