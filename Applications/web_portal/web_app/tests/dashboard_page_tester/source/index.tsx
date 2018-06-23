import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {DashboardPage, LocalDashboardModel} from 'web_portal';

const model = new LocalDashboardModel(
  Beam.DirectoryEntry.makeAccount(123, 'demo'),
  new Nexus.AccountRoles(0));

ReactDOM.render(<DashboardPage model={model}/>,
  document.getElementById('main'));
