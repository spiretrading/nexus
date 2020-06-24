import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, PageWrapper } from '../../..';
import { AccountEntry } from './account_entry';
import { CreateGroupModal } from './create_group_modal';
import { FilterBar } from './filter_bar';
import { GroupCard } from './group_card';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** The roles of the user looking at the directory page. */
  roles: Nexus.AccountRoles;

  /** The groups to display. */
  groups: Beam.DirectoryEntry[];

  /** The groups that are open and the accounts that belong to those groups. */
  openedGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>

  /** The filter used to return a subset of groups. */
  filter : string;

  /** The accounts that match the current filter. */
  filteredGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>

  /** The error message from when a new group was created. */
  createGroupStatus?: string;

  /** Called when the filter value changes. */
  onFilterChange?: (filter: string) => void;

  /** Called when a card is clicked on. */
  onCardClick?: (group: Beam.DirectoryEntry) => void;

  /** Called when the user wants to make a new group. 
   * @param name - The name of the group.
   */
  onCreateGroup?: (name: string) => void;

  /** Called when the user wants to make a new account. */
  onNewAccountClick?: () => void;
}

interface State {
  isCreateGroupModalOpen: boolean;
}

/** Displays a directory of accounts. */
export class AccountDirectoryPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onFilterChange: () => {},
    onCardClick: () => {},
    onCreateGroup: () => {},
    onNewAccountClick: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isCreateGroupModalOpen: false
    };
  }

  public render(): JSX.Element {
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return AccountDirectoryPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return AccountDirectoryPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return AccountDirectoryPage.STYLE.contentLarge;
      }
    })();
    const headerBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.STYLE.verticalHeaderBox;
      } else {
        return AccountDirectoryPage.STYLE.horizontalHeaderBox;
      }
    })();
    const buttonBoxStyle = (() => {
      if(this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return AccountDirectoryPage.STYLE.buttonBoxSmall;
        } else {
          return AccountDirectoryPage.STYLE.buttonBox;
        }
      } else {
        return AccountDirectoryPage.STYLE.hidden;
      }
    })();
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.DYNAMIC_STYLE.buttonSmall;
      } else {
        return AccountDirectoryPage.DYNAMIC_STYLE.button;
      }
    })();
    const horizontalButtonVisibility = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const verticalButtonVisibility = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return AccountDirectoryPage.STYLE.hidden;
      }
    })();
    const createGroupModal = (() => {
      if(this.state.isCreateGroupModalOpen) {
        return <CreateGroupModal 
          displaySize={this.props.displaySize}
          errorStatus={this.props.createGroupStatus}
          isOpen={this.state.isCreateGroupModalOpen}
          onClose={this.onCloseCreateGroupModal}
          onCreateGroup={this.props.onCreateGroup}/>;
      } else {
        return null;
      }
    })();
    const cards = [];
    for (const group of this.props.groups) {
      const accounts = (() => {
        if(this.props.filter && !this.props.openedGroups.get(group)) {
          return this.props.filteredGroups.get(group) || [];
        } else {
          return this.props.openedGroups.get(group) || [];
        }
      })();
      cards.push(
        <GroupCard key={group.id}
          displaySize={this.props.displaySize}
          group={group}
          accounts={accounts}
          filter={this.props.filter}
          isOpen={this.props.openedGroups.get(group) !== undefined}
          onDropDownClick={() => this.props.onCardClick(group)}/>);
    }
    return (
      <PageWrapper>
        {createGroupModal}
        <div style={AccountDirectoryPage.STYLE.page}>
          <div style={contentWidth}>
            <div style={headerBoxStyle}>
              <div style={verticalButtonVisibility}>
                <div style={buttonBoxStyle}>
                  <button className={css(buttonStyle)}
                      onClick={this.props.onNewAccountClick}>
                    New Account
                  </button>
                  <div style={AccountDirectoryPage.STYLE.spacing}/>
                  <button onClick={this.onCreateGroupClick}
                      className={css(buttonStyle)}>
                    New Group
                  </button>
                </div>
                <div style={AccountDirectoryPage.STYLE.spacing}/>
              </div>
              <FilterBar value={this.props.filter} 
                onChange={this.props.onFilterChange}/>
              <div style={{...buttonBoxStyle, ...horizontalButtonVisibility}}>
                <div style={AccountDirectoryPage.STYLE.spacing}/>
                <button className={css(buttonStyle)}
                    onClick={this.props.onNewAccountClick}>
                  New Account
                </button>
                <div style={AccountDirectoryPage.STYLE.spacing}/>
                <button onClick={this.onCreateGroupClick}
                    className={css(buttonStyle)}>
                  New Group
                </button>
              </div>
            </div>
            <div style={AccountDirectoryPage.STYLE.spacing}/>
            {cards}
          </div>
        </div>
      </PageWrapper>);
  }

  private onCreateGroupClick = () => {
    this.setState({isCreateGroupModalOpen: true});
  }

  private onCloseCreateGroupModal = () => {
    this.setState({isCreateGroupModalOpen: false});
  }

  private static readonly STYLE = {
    page: {
      boxSizing: 'border-box' as 'border-box',
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'center',
      padding: '18px',
      paddingBottom: '40px'
    },
    contentSmall: {
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      width: '732px'
    },
    contentLarge: {
      width: '1000px'
    },
    verticalHeaderBox: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'flex-end' as 'flex-end'
    },
    horizontalHeaderBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    buttonBoxSmall: {
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    buttonBox: {
      boxSizing: 'border-box' as 'border-box',
      flexBasis: '298px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'flex-end' as 'flex-end'
    },
    spacing: {
      width: '18px',
      height: '18px',
      flexBias: '18px',
      flexGrow: 0,
      flexShrink: 0
    },
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
  private static DYNAMIC_STYLE = StyleSheet.create({
    button: {
      boxSizing: 'border-box' as 'border-box',
      width: '140px',
      flexBasis: '140px',
      flexGrow: 0,
      flexShrink: 0,
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      borderRadius: 1,
      cursor: 'pointer' as 'pointer',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      ':active' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      '::-moz-focus-inner': {
        border: 0
      }
    },
    buttonSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '140px',
      flexBasis: '140px',
      flexGrow: 1,
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      cursor: 'pointer' as 'pointer',
      borderRadius: 1,
      ':active' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      '::-moz-focus-inner': {
        border: 0
      }
    }
  });
}
