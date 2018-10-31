import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {AccountPage, LocalAccountModel} from 'web_portal';

const model = new LocalAccountModel(
  Beam.DirectoryEntry.makeAccount(124, 'username_can_be_very_long'),
  new Nexus.AccountRoles(
    Nexus.AccountRoles.Role.TRADER |
    Nexus.AccountRoles.Role.MANAGER |
    Nexus.AccountRoles.Role.ADMINISTRATOR));

ReactDOM.render(<AccountPage model={model}/>, document.getElementById('main'));
