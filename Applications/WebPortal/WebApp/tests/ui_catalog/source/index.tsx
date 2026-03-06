import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';
import { componentsList } from './data/component_schemas';
import { ComponentSchema } from './data/schemas';
import { UICatalog } from './ui_catalog';
import { Viewport } from './viewport';
import { viewportRenderer } from './viewport_renderer';

interface Properties {
  viewport: Viewport;
  components: ComponentSchema[];
}

class App extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <Router.BrowserRouter>
        <UICatalog componentList={this.props.components}/>
      </Router.BrowserRouter>);
  }
}

const ResponsivePage = viewportRenderer(App);
ReactDOM.render(<ResponsivePage components={componentsList}/>,
  document.getElementById('main'));
