import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const account = Beam.DirectoryEntry.makeAccount(124,
  'username_can_be_very_long');
const roles = new Nexus.AccountRoles();
roles.set(Nexus.AccountRoles.Role.TRADER);
roles.set(Nexus.AccountRoles.Role.MANAGER);
roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);

const someDB = Nexus.buildDefaultCountryDatabase();
const ResponsivePage =
  WebPortal.displaySizeRenderer(WebPortal.CreateAccountPage);
ReactDOM.render(<ResponsivePage
  countryDatabase={someDB}/>,
  document.getElementById('main'));
