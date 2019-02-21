import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const someDB = Nexus.buildDefaultCountryDatabase();
const thing = (username: string, groups: Beam.DirectoryEntry[],
    identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles) => {
     return 'boo';
    };

const ResponsivePage =
  WebPortal.displaySizeRenderer(WebPortal.CreateAccountPage);
ReactDOM.render(<ResponsivePage
  onSubmit={thing}
  countryDatabase={someDB}/>,
  document.getElementById('main'));
