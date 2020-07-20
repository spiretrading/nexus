import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const account = Beam.DirectoryEntry.makeAccount(124,
  'Group_name_goes_here');

ReactDOM.render(
  <WebPortal.GroupPage
    displaySize={WebPortal.DisplaySize.getDisplaySize()}
    account={account}
    subPage={WebPortal.GroupSubPage.GROUP}/>,
  document.getElementById('main'));
