import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import { VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, DropDownButton, HLine } from '../..';
import { RolePanel } from '../account_page/role_panel';
import { AccountEntry } from '.';

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
   * @param entry - A accounty or directory entry.
   */
  onDirectoryEntryClick?: (entry: Beam.DirectoryEntry) => void;
}

interface State {
  isHeaderHovered: boolean;
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
      isHeaderHovered: false
    };
    this.onGroupMouseEnter = this.onGroupMouseEnter.bind(this);
    this.onGroupMouseLeave = this.onGroupMouseLeave.bind(this);
  }

  public render(): JSX.Element {
    const headerTextStyle = (() => {
      if(this.props.isOpen) {
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
    const accountsLableStyle = (() => {
      switch(this.props.displaySize) {
        case(DisplaySize.SMALL):
          return null;
        case(DisplaySize.MEDIUM):
          return GroupCard.STYLE.accountLabelMedium;
        case(DisplaySize.LARGE):
          return GroupCard.STYLE.accountLabelLarge;
      }
    })();
    const dropDownButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <DropDownButton size='20px'
            isExpanded={this.props.isOpen}
            onClick={this.props.onDropDownClick}/>);
      } else {
        return (
          <div style={GroupCard.STYLE.dropDownButtonWrapper}
              onClick={this.props.onDropDownClick}>
            <DropDownButton size='16px'
              isExpanded={this.props.isOpen}/>
          </div>);
      }
    })();
    const accounts: JSX.Element[] = [];
    if(this.props.accounts.length > 0) {
      let noLine = true;
      for(const account of this.props.accounts) {
        if(account.account.name.indexOf(this.props.filter) === 0
            && this.props.filter) {
          if(!this.props.isOpen && noLine) {
            accounts.push(
              <HLine key={this.props.group.id} color='#E6E6E6'/>);
            noLine = false;
          }
          accounts.push(
           <div className={css(GroupCard.DYNAMIC_STYLE.accountBox)}
              key={account.account.id}
              onClick={() =>
                this.props.onDirectoryEntryClick(account.account)}>
            <div style={{...accountsLableStyle,
                ...GroupCard.STYLE.accountLabelText}}>
              <div style={GroupCard.STYLE.highlightedText}>
                {account.account.name.slice(0, this.props.filter.length)}
              </div>
              {account.account.name.slice(this.props.filter.length)}
            </div>
            <div style={GroupCard.STYLE.rolesWrapper}>
              <RolePanel roles={account.roles}/>
            </div>
          </div>);
        } else {
          accounts.push(
            <Transition in={this.props.isOpen}
                appear={true}
                key={account.account.id}
                timeout={GroupCard.TIMEOUTS}>
              {(state) => (
                <div className={css(GroupCard.DYNAMIC_STYLE.accountBox,
                  (GroupCard.accountLabelAnimationStyle as any)[state])}
                    key={account.account.id}
                    onClick={() =>
                      this.props.onDirectoryEntryClick(account.account)}>
                  <div style={{...accountsLableStyle,
                      ...GroupCard.STYLE.accountLabelText}}>
                    {account.account.name.toString()}
                  </div>
                  <div style={GroupCard.STYLE.rolesWrapper}>
                    <RolePanel roles={account.roles}/>
                  </div>
                </div>
              )}
            </Transition>);
        }
      }
    } else {
      accounts.push(
        <Transition in={this.props.isOpen}
            timeout={GroupCard.TIMEOUTS}
            key={this.props.group.id}>
          {(state) => (
            <div key={this.props.group.id} style={{...accountsLableStyle,
                ...GroupCard.STYLE.emptyLableText,
                ...(GroupCard.emptyLabelAnimationStyle as any)[state]}}>
              Empty
            </div>
          )}
        </Transition>);
    }
    const topAccountPadding = (() => {
      if(this.props.accounts.length === 0) {
        return '14px';
      } else {
        return '10px';
      }
    })();
    return (
      <VBoxLayout width='100%'>
        <div style={{...GroupCard.STYLE.header, ...headerMouseOverStyle}}>
          {dropDownButton}
          <div style={headerTextStyle}
              onClick={() =>
                this.props.onDirectoryEntryClick(this.props.group)}
              onMouseEnter={this.onGroupMouseEnter}
              onMouseLeave={this.onGroupMouseLeave}>
            {this.props.group.name}
          </div>
        </div>
        <Transition in={this.props.isOpen}
            timeout={GroupCard.TIMEOUTS}>
          {(state) => (
            <div>
              <div style={(GroupCard.topPaddingAnimationStyle as any)[state]}>
                <HLine color='#E6E6E6'/>
                <div style={{height: topAccountPadding}}/>
              </div>
              {accounts}
              <div style={
                    (GroupCard.bottomPaddingAnimationStyle as any)[state]}>
                <div style={{height:'20px'}}/>
              </div>
            </div>
          )}
        </Transition>
      </VBoxLayout>);
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
    accountLabelMedium: {
      marginLeft: '38px'
    },
    accountLabelLarge: {
      marginLeft: '38px'
    },
    accountLabelText: {
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 0,
      flexShrink: 0,
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    emptyLableText: {
      font: '400 14px Roboto',
      color: '#8C8C8C',
      paddingLeft: '10px',
      cursor: 'default' as 'default'
    },
    highlightedText: {
      font: '400 14px Roboto',
      color: '#000000',
      backgroundColor: '#FFF7C4',
      flexGrow: 0,
      flexShrink: 0
    },
    rolesWrapper: {
      width: '80px',
      flexGrow: 0,
      flexShrink: 0
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
  private static DYNAMIC_STYLE = StyleSheet.create({
    accountBox: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      paddingLeft: '10px',
      paddingRight: '10px',
      cursor: 'pointer' as 'pointer',
      ':hover' : {
        backgroundColor: '#F8F8F8'
      },
      ':active' : {
        backgroundColor: '#F8F8F8'
      }
    }
  });
  private static readonly accountLabelAnimationStyle = StyleSheet.create({
    entering: {
      maxHeight: 0,
      transform: 'scaleY(0)'
    },
    entered: {
      maxHeight: '34px',
      transform: 'scaleY(1)',
      transitionProperty: 'max-height, transform',
      transitionDuration: '200ms',
      transformOrigin: 'top' as 'top'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `200ms`,
      transformOrigin: 'top' as 'top'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transformOrigin: 'top' as 'top'
    }
  });
  private static readonly emptyLabelAnimationStyle = {
    entering: {
      maxHeight: 0,
      transform: 'scaleY(0)'
    },
    entered: {
      maxHeight: '34px',
      transform: 'scaleY(1)',
      transitionProperty: 'max-height, transform',
      transitionDuration: '200ms',
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
  private static readonly  topPaddingAnimationStyle = {
    entering: {
      maxHeight: 0,
      transform: 'scaleY(0)'
    },
    entered: {
      maxHeight: '15px',
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
  private static readonly  bottomPaddingAnimationStyle = {
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
    enter: 0,
    entered: 200,
    exit: 200,
    exited:  200
  };
}
