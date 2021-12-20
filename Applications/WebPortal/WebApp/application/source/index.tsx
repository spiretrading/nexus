import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const model =
  new WebPortal.HttpApplicationModel(new Nexus.HttpServiceClients());

ReactDOM.render(
  <Router.BrowserRouter>
    <WebPortal.ApplicationController model={model}/>
  </Router.BrowserRouter>, document.getElementById('main'));
