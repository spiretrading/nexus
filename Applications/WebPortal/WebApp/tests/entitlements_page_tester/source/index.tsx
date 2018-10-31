import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { EntitlementsPage } from 'web_portal';
import { CheckMarkButton } from 'web_portal';
import { DropDownButton } from 'web_portal';

const buttontest = (
  <div>
    <CheckMarkButton size='50px'isReadOnly={true}/>
    <DropDownButton size='50px'/>
  </div>);

ReactDOM.render(buttontest,
  document.getElementById('main'));
