import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const roles = new Nexus.AccountRoles(0);

ReactDOM.render(<WebPortal.DashboardPage roles={roles}/>,
  document.getElementById('main'));
