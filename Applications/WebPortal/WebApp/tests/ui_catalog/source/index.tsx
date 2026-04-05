import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';
import { componentSections } from './data/component_schemas';
import { ComponentSection } from './data/schemas';
import { UICatalog } from './ui_catalog';
import { Viewport } from './viewport';
import { viewportRenderer } from './viewport_renderer';

interface Properties {
  viewport: Viewport;
  sections: ComponentSection[];
}

class App extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <Router.BrowserRouter>
        <UICatalog sections={this.props.sections}/>
      </Router.BrowserRouter>);
  }
}

const ResponsivePage = viewportRenderer(App);
ReactDOM.render(<ResponsivePage sections={componentSections}/>,
  document.getElementById('main'));
