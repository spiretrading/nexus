import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, PageWrapper, AccountEntry } from '../../..';
import { GroupMemberEntry } from './group_member_entry';

interface Properties {

  displaySize: DisplaySize;

  members: AccountEntry[];

  lastUpdate?: Beam.DateTime;
}

interface State {
  dateTimeToDisplay: string;
}

export class GroupInfoPage extends React.Component<Properties> {
  public static readonly defaultProps = {
  }

  public render(): JSX.Element {
    const {displaySize, members} = this.props;
    const membersCount = (() => {
      if(displaySize !== DisplaySize.SMALL) {
        return (
          <span style={GroupInfoPage.STYLE.membersInfo}>
            Total members: {members.length}
          </span>);
      } else {
        return null;
      }
    })();
    const containerDimension = (() => {
      if(displaySize === DisplaySize.SMALL) {
        return GroupInfoPage.STYLE.smallContainer;
      } else if(displaySize === DisplaySize.MEDIUM) {
        return GroupInfoPage.STYLE.mediumContainer;
      } else {
        return GroupInfoPage.STYLE.largeContainer; 
      }
    })();
    const content = (() => {
      if(members.length === 0) {
        return <div>{GroupInfoPage.GROUP_EMPTY_MESSAGE}</div>;
      } else {
        const entries = [];
        for(const account of members) {
          entries.push(<GroupMemberEntry account={account}/>);
        }
        return entries;
      } 
    })();
    return(
      <PageWrapper>
        <div style={GroupInfoPage.STYLE.pageMargins}>
          {membersCount}
          <div style={{...containerDimension, 
              ... GroupInfoPage.STYLE.border}}>
            {content}
          </div>
        </div>
      </PageWrapper>);
  }

  private static readonly STYLE = {
    pageMargins: {
      marginTop: '18px',
      marginBottom: '60px',
      marginLeft: '18px',
      marginRight: '18px',
    } as React.CSSProperties,
    smallContainer: {
      minWidth: '284px',
      maxWidth: '424px',
      overflowY: 'auto'
    } as React.CSSProperties,
    mediumContainer: {
      width: '372px',
      height: '342px',
      overflowY: 'auto'
    } as React.CSSProperties,
    largeContainer: {
      width: '1000x',
      height: '342px',
      overflowY: 'auto'
    } as React.CSSProperties,
    border: {
      borderColor: '#C8C8C8',
      borderWidth: '1px',
      borderRadius: '1px'
    } as React.CSSProperties,
    membersInfo: {
      textAlign: 'right',
      width: '100%',
      color: '#333333',
      font: '400 14px Roboto',
    }as React.CSSProperties,
    noEntries: {
      width: '100%',
      height: '100%',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      color: '#333333',
      font: '400 14px Roboto',
    } as React.CSSProperties
  }
  private static readonly GROUP_EMPTY_MESSAGE = 'Group is empty.';
}
