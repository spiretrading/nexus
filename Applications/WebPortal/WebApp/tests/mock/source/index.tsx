import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';
import { MockApplicationModel } from './mock_application_model';

const model = new MockApplicationModel();

ReactDOM.render(
  <Router.HashRouter>
    <WebPortal.ApplicationController model={model}/>
  </Router.HashRouter>,
  document.getElementById('main'));
