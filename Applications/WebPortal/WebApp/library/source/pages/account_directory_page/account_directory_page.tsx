import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../..';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';
import { FilterBar } from './filter_bar';
import { GroupCard } from './group_card';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Model that contains information about the accounts. */
  model: AccountDirectoryModel;

  /** The roles of the user looking at the directory page. */
  roles: Nexus.AccountRoles;

  /** Called when the user wants to make a new group. */
  onNewGroupClick?: () => void;

  /** Called when the user wants to make a new account. */
  onNewAccountClick?: () => void;
}

interface State {
  filter: string;
  accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  openedGroups: Beam.Set<Beam.DirectoryEntry>;
}

/** Displays a directory of accounts. */
export class AccountDirectoryPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onNewGroupClick: () => {},
    onNewAccountClick: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      filter: '',
      accounts: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      openedGroups: new Beam.Set<Beam.DirectoryEntry>()
    };
    this.onChange = this.onChange.bind(this);
    this.onCardClick = this.onCardClick.bind(this);
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
    const cards = [];
    for (const group of this.props.model.groups) {
      const accounts = this.state.accounts.get(group) || [];
      cards.push(
        <GroupCard key={group.id}
          displaySize={this.props.displaySize}
          group={group}
          accounts={accounts}
          filter={this.state.filter}
          isOpen={this.state.openedGroups.test(group)}
          onDropDownClick={() => this.onCardClick(group)}/>);
    }
    return (
      <div style={AccountDirectoryPage.STYLE.page}>
        <div style={contentWidth}>
          <div id='header' style={headerBoxStyle}>
            <div style={verticalButtonVisibility}>
              <div style={buttonBoxStyle}>
                <button className={css(buttonStyle)}
                    onClick={this.props.onNewAccountClick}>
                  New Account
                </button>
                <div style={AccountDirectoryPage.STYLE.spacing}/>
                <button onClick={this.props.onNewGroupClick}
                    className={css(buttonStyle)}>
                  New Group
                </button>
              </div>
              <div style={AccountDirectoryPage.STYLE.spacing}/>
            </div>
            <FilterBar value={this.state.filter} onChange={this.onChange}/>
            <div style={{...buttonBoxStyle, ...horizontalButtonVisibility}}>
              <div style={AccountDirectoryPage.STYLE.spacing}/>
              <button className={css(buttonStyle)}
                  onClick={this.props.onNewAccountClick}>
                New Account
              </button>
              <div style={AccountDirectoryPage.STYLE.spacing}/>
              <button onClick={this.props.onNewGroupClick}
                  className={css(buttonStyle)}>
                New Group
              </button>
            </div>
          </div>
          <div style={AccountDirectoryPage.STYLE.spacing}/>
          <div id='group_cards'>
            {cards}
          </div>
        </div>
    </div>);
  }

  private onChange(newFilter: string) {
    clearTimeout(this.timerId);
    this.setState({filter: newFilter});
    if(newFilter !== '') {
      this.timerId = setTimeout(
        async () => {
          const newAccounts =
            await this.props.model.loadFilteredAccounts(newFilter);
          this.setState({
            openedGroups: new Beam.Set<Beam.DirectoryEntry>(),
            accounts: newAccounts
          });
        }, 400);
    }
  }

  private async onCardClick(group: Beam.DirectoryEntry) {
    if(this.state.openedGroups.test(group)) {
      this.state.openedGroups.remove(group);
    } else {
      const accounts = await this.props.model.loadAccounts(group);
      this.state.accounts.set(group, accounts);
      this.state.openedGroups.add(group);
    }
    this.setState({
      openedGroups: this.state.openedGroups,
      accounts: this.state.accounts
    });
  }

  private static readonly STYLE = {
    page: {
      boxSizing: 'border-box' as 'border-box',
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      overflowY: 'auto' as 'auto',
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
  private timerId: NodeJS.Timeout;
}
