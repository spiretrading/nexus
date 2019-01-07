import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const model = new WebPortal.WebApplicationModel(new Nexus.WebServiceClients());

ReactDOM.render(<WebPortal.ApplicationController model={model}/>,
  document.getElementById('main'));
