import * as Beam from 'beam';
import { VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, DropDownButton, HLine } from '../..';
import { AccountEntry } from './account_entry';
import { AccountEntryRow } from './account_entry_row';

interface Properties {

  /** Determines the size to display the component at. */
  displaySize: DisplaySize;

  /** The group the card belongs to. */
  group: Beam.DirectoryEntry;

  /** The accounts in the group. */
  accounts: AccountEntry[];

  /** The current filter used on the accounts. */
  filter: string;

  /**  Determines if the card is opened. */
  isOpen: boolean;

  /** Called when the dropdown button is clicked. */
  onDropDownClick?: () => void;

  /** Called when a directory is clicked on.
   * @param entry - The account or directory entry.
   */
  onDirectoryEntryClick?: (entry: Beam.DirectoryEntry) => void;
}

interface State {
  isHeaderHovered: boolean;
  isOpen: boolean;
  localAccounts: AccountEntry[];
}

/** A card that displays a group and the accounts associated with it. */
export class GroupCard extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onDropDownClick: () => {},
    onDirectoryEntryClick: () => {}
  }

  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isHeaderHovered: false,
      isOpen: false,
      localAccounts: []
    };
    this.onGroupMouseEnter = this.onGroupMouseEnter.bind(this);
    this.onGroupMouseLeave = this.onGroupMouseLeave.bind(this);
  }

  public render(): JSX.Element {
    const headerTextStyle = (() => {
      if(this.state.isOpen) {
        return GroupCard.STYLE.headerTextOpen;
      } else {
        return GroupCard.STYLE.headerText;
      }
    })();
    const headerMouseOverStyle = (() => {
      if(this.state.isHeaderHovered) {
        return GroupCard.STYLE.mouseOverStyle;
      } else {
        return null;
      }
    })();
    const dropDownButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <DropDownButton size='20px'
            isExpanded={this.state.isOpen}
            onClick={this.props.onDropDownClick}/>);
      } else {
        return (
          <div style={GroupCard.STYLE.dropDownButtonWrapper}
              onClick={this.props.onDropDownClick}>
            <DropDownButton size='16px'
              isExpanded={this.state.isOpen}/>
          </div>);
      }
    })();
    const topAccountPadding = (() => {
      if(this.state.localAccounts.length === 0) {
        return '14px';
      } else {
        return '10px';
      }
    })();
    const lineWhenOpen = (() => {
      if(this.state.isOpen) {
        return (
          <div>
            <HLine color='#E6E6E6'/>
            <div style={{height: topAccountPadding}}/>
          </div>);
      } else {
        return null;
      }
    })();
    const accounts: JSX.Element[] = [];
    if(this.state.localAccounts.length > 0) {
      for(const account of this.state.localAccounts) {
        if(account.account.name.indexOf(this.props.filter) === 0 &&
            this.props.filter) {
          accounts.push(
            <AccountEntryRow 
              displaySize={this.props.displaySize}
              account={account}
              filter={this.props.filter}
              isOpen={this.state.isOpen}
              onDirectoryEntryClick={this.props.onDirectoryEntryClick}/>);
          if(!this.state.isOpen && this.state.localAccounts.indexOf(account) ===
              this.state.localAccounts.length - 1) {
            accounts.push(<div style={{height: topAccountPadding}}/>);
            accounts.push(<HLine key={this.props.group.id} color='#E6E6E6'/>);
          }
        } else {
          accounts.push(
            <AccountEntryRow 
              displaySize={this.props.displaySize}
              account={account}
              isOpen={this.state.isOpen}
              onDirectoryEntryClick={this.props.onDirectoryEntryClick}/>);
        }
      }
    } else {
      accounts.push(
        <AccountEntryRow 
          displaySize={this.props.displaySize}
          isOpen={this.state.isOpen}/>);
    }
    return (
      <VBoxLayout width='100%'>
        <div style={{...GroupCard.STYLE.header, ...headerMouseOverStyle}}>
          {dropDownButton}
          <div style={headerTextStyle}
              onClick={() => this.props.onDirectoryEntryClick(this.props.group)}
              onMouseEnter={this.onGroupMouseEnter}
              onMouseLeave={this.onGroupMouseLeave}>
            {this.props.group.name}
          </div>
        </div>
        {lineWhenOpen}
        <div style={GroupCard.STYLE.entryListWrapper}>
          {accounts}
        </div>
        <Transition in={this.state.isOpen} timeout={GroupCard.TIMEOUTS}>
          {(state) => (
            <div style= {(GroupCard.animationStyle as any)[state]}>
              <div style={{height:'20px'}}/>
            </div>
          )}
        </Transition>
      </VBoxLayout>);
  }

  public static getDerivedStateFromProps(props: Properties, state: State) {
    if(!props.isOpen && !state.isOpen && props.accounts.length !==
        state.localAccounts.length) {
      return {localAccounts: props.accounts};
    } else if(props.isOpen && !state.isOpen) {
      return {isOpen: true, localAccounts: props.accounts};
    } else if(!props.isOpen && state.isOpen) {
      return {isOpen: false};
    } else {
      return null;
    }
  }

  private onGroupMouseEnter() {
    this.setState({isHeaderHovered: true});
  }

  private onGroupMouseLeave() {
    this.setState({isHeaderHovered: false});
  }

  private static readonly STYLE = {
    headerTextOpen: {
      marginLeft: '18px',
      font: '500 14px Roboto',
      color: '#4B23A0',
      cursor: 'pointer' as 'pointer',
      flexGrow: 1,
      height: '34px',
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'flex-start' as 'flex-start'
    },
    headerText: {
      marginLeft: '18px',
      font: '400 14px Roboto',
      color: '#000000',
      cursor: 'pointer' as 'pointer',
      flexGrow: 1,
      height: '34px',
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'flex-start' as 'flex-start'
    },
    entryListWrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column-reverse' as 'column-reverse'
    },
    dropDownButtonWrapper: {
      width: '20px',
      height: '20px',
      flexGrow: 0,
      flexShrink: 0,
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center'
    },
    mouseOverStyle: {
      backgroundColor: '#F8F8F8'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      paddingLeft: '10px',
      paddingRight: '10px'
    }
  };
  private static readonly animationStyle = {
    entering: {
      maxHeight: 0,
      transform: 'scaleY(1)'
    },
    entered: {
      maxHeight: '20px',
      transform: 'scaleY(1)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `200ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `200ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    }
  };
  private static readonly TIMEOUTS = {
    enter: 1,
    entered: 200,
    exit: 200,
    exited:  200
  };
}
