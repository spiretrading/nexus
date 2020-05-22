import * as React from 'react';
import { AccountEntry, DisplaySize, PageWrapper } from '../../..';
import { GroupMemberEntry } from './group_member_entry';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The accounts that belong to the group. */
  group: AccountEntry[];
}

/** A page that displays information about a group and its members. */
export class GroupInfoPage extends React.Component<Properties> {

  public render(): JSX.Element {
    const {displaySize, group} = this.props;
    const membersCount = (() => {
      if(displaySize !== DisplaySize.SMALL) {
        return (
          <span style={GroupInfoPage.STYLE.membersInfo}>
            Total members: {group.length}
          </span>);
      } else {
        return null;
      }
    })();
    const pageWidth = (() => {
      if(displaySize === DisplaySize.SMALL) {
        return GroupInfoPage.STYLE.smallPageWidth;
      } else if(displaySize === DisplaySize.MEDIUM) {
        return GroupInfoPage.STYLE.mediumPageWidth;
      } else {
        return GroupInfoPage.STYLE.largePageWidth; 
      }
    })();
    const content = (() => {
      if(group.length === 0) {
        return (
          <div style={GroupInfoPage.STYLE.noEntries}>
            {GroupInfoPage.GROUP_EMPTY_MESSAGE}
          </div>);
      } else {
        const entries = [];
        for(const account of group) {
          entries.push(<GroupMemberEntry account={account}/>);
        }
        return entries;
      } 
    })();
    return(
      <PageWrapper>
        <div style={{...GroupInfoPage.STYLE.pageMargins, ...pageWidth}}>
          <div style={GroupInfoPage.STYLE.header}>
            <span>Last Update:</span>
            {membersCount}
          </div> 
          <div style={GroupInfoPage.STYLE.listContainer}>
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
      height: '100%',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center'
    } as React.CSSProperties,
    smallPageWidth: {
      boxSizing: 'border-box',
      minWidth: '284px',
      maxWidth: '424px',
      width: '100%',
    } as React.CSSProperties,
    mediumPageWidth: {
      boxSizing: 'border-box',
      width: '732px',
    } as React.CSSProperties,
    largePageWidth: {
      boxSizing: 'border-box',
      width: '1000px',
    } as React.CSSProperties,
    listContainer: {
      boxSizing: 'border-box',
      height: '342px',
      width: '100%',
      overflowY: 'auto',
      borderColor: '#C8C8C8',
      borderStyle: 'solid',
      borderWidth: '1px',
      borderRadius: '1px'
    } as React.CSSProperties,
    header: {
      display: 'flex',
      justifyContent: 'space-between',
      width: '100%',
      height: '16px',
      font: '400 14px Roboto',
      flexDirection: 'row',
      marginBottom: '30px',
    } as React.CSSProperties,
    membersInfo: {
      textAlign: 'right',
      height: '100$',
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
