import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { DirectoryEntry } from 'beam';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  members: WebPortal.AccountEntry[];
}

/** Displays a sample GroupInfoPage for testing. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      members: []
    };
  }

  public render(): JSX.Element {
    console.log(this.state.members);
    return(
    <div>
      <WebPortal.GroupInfoPage
        group={this.state.members}
        displaySize={this.props.displaySize}/>
    </div>);
  }

  public componentDidMount() {
    this.fillGroup();
  }

  private fillGroup = () => {
    console.log('fill???');
    let someGroup = [];
    someGroup.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(12, 'Bob'), 
      new Nexus.AccountRoles(2)));
    someGroup.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(18, 'Sue'), 
      new Nexus.AccountRoles(2)));
    someGroup.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(13, 'Carl'), 
      new Nexus.AccountRoles(2)));
    this.setState({members: someGroup});
  }
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
