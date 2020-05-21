import * as React from 'react';
import { DisplaySize } from '../../..'
import { GroupInfoModel } from './group_info_page_model';
import { GroupInfoPage } from './group_info_page';

interface Properties {
  displaySize: DisplaySize;
  model: GroupInfoModel;
}

export class GroupInfoController extends React.Component<Properties> {

  public render(): JSX.Element {
    return(
      <GroupInfoPage
        displaySize={this.props.displaySize}
        members={this.props.model.groupMembers}/>);
  }
}