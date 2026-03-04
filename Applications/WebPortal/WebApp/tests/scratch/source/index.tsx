import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

ReactDOM.render(
  <WebPortal.IconLabelButton
    icon='resources/arrow-expand.svg'
    label='Expand'
    variant={WebPortal.IconLabelButton.Variant.ICON_LABEL}/>,
  document.getElementById('main'));
