import * as React from 'react';
import { DisplaySize, LoadingPage } from '../../..'
import { GroupInfoModel } from './group_info_model';
import { GroupInfoPage } from './group_info_page';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The model representing the group. */
  model: GroupInfoModel;
}

interface State {
  isLoaded: boolean;
}

/** Controller for the GroupInfoPage */
export class GroupInfoController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false
    }
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return(
      <GroupInfoPage
        displaySize={this.props.displaySize}
        group={this.props.model.groupMembers}/>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }
}