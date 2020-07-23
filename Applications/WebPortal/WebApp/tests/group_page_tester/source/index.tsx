import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { DisplaySize, displaySizeRenderer, GroupPage, GroupSubPage,
  LoadingPage, LocalGroupModel } from 'web_portal';

interface Properties {
  displaySize: DisplaySize;
}

interface State {
  subPage: GroupSubPage;
  isLoaded: boolean;
}

class GroupPageTester extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      subPage: GroupSubPage.NONE,
      isLoaded: false
    };
    this.groupModel = new LocalGroupModel(
      Beam.DirectoryEntry.makeAccount(124, 'Group_name_goes_here'));
  }

  public async componentDidMount(): Promise<void> {
    await this.groupModel.load();
    this.setState({isLoaded: true});
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <GroupPage displaySize={this.props.displaySize}
        subPage={this.state.subPage} group={this.groupModel.group}
        onMenuClick={this.onMenuClick}/>);
  }

  private onMenuClick = (subPage: GroupSubPage) => {
    this.setState({subPage: subPage});
  }

  private groupModel: LocalGroupModel;
}

const ResponsiveGroupPageTester = displaySizeRenderer(GroupPageTester);

ReactDOM.render(<ResponsiveGroupPageTester/>, document.getElementById('main'));
