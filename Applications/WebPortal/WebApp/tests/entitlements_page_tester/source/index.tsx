import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const buttontest = (
  <div>
    <WebPortal.CheckMarkButton size='50px' isReadOnly={false} isChecked/>
    <WebPortal.DropDownButton size='50px'/>
  </div>);

ReactDOM.render(buttontest,
  document.getElementById('main'));
