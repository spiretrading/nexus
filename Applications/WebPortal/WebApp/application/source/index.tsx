import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const serviceUrl = new URL('/api/administration_service/websocket',
  (window.location.protocol === 'https:' ? 'wss://' : 'ws://') +
  window.location.host);
const model =
  new WebPortal.HttpApplicationModel(
    new Nexus.HttpServiceClients(serviceUrl));

ReactDOM.render(
  <Router.BrowserRouter>
    <WebPortal.ApplicationController model={model}/>
  </Router.BrowserRouter>, document.getElementById('main'));
