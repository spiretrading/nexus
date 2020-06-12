import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const roles = new Nexus.AccountRoles(0);

ReactDOM.render(
  <Router.BrowserRouter>
    <WebPortal.DashboardPage roles={roles}/>
  </Router.BrowserRouter>,
  document.getElementById('main'));
