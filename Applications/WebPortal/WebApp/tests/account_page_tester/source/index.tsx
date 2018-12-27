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

ReactDOM.render(<WebPortal.AccountPage displaySize={WebPortal.DisplaySize.LARGE}
  account={account} roles={roles}
  subPage={WebPortal.SubPage.ENTITLEMENTS}/>
,
  document.getElementById('main'));
