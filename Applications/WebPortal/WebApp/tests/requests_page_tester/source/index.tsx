import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

ReactDOM.render(
  <WebPortal.NavigationTab
    icon='resources/account/account-grey.svg'
    highlightedIcon='resources/account/account-purple.svg'
    href='account/123'
    label='Account'
    variant={WebPortal.NavigationTab.Variant.ICON_LABEL}/>,
  document.getElementById('main'));
