import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { Transition } from 'react-transition-group';
import { AccountEntry, DisplaySize, RolePanel } from '../../..';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** Determines if the entry is visible and open. */
  isOpen: boolean;

  /** The account to display. */
  account?: AccountEntry;

  /** The filter. Used to determine highlighting. */
  filter?: string;
}

interface State {
  afterFirstRender: boolean;
  isOpen: boolean;
}

/** Displays a account entry or a empty entry. */
export class AccountEntryRow extends React.Component<Properties, State> {
  public static getDerivedStateFromProps(props: Properties, state: State) {
    if(state.afterFirstRender) {
      if(props.isOpen && !state.isOpen) {
        return {isOpen: true};
      } else if(!props.isOpen && state.isOpen) {
        return {isOpen: false};
      }
    }
    return null;
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      afterFirstRender: false,
      isOpen: false
    };
  }

  public render(): JSX.Element {
    const rowStyle = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return null;
        case DisplaySize.MEDIUM:
          return AccountEntryRow.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return AccountEntryRow.STYLE.contentLarge;
      }
    })();
    const id = (() => {
      if(this.props.account) {
        return this.props.account.account.id;
      } else {
        return 0;
      }
    })();
    const dynamic_style = (() => {
      if(this.props.account) {
        return AccountEntryRow.DYNAMIC_STYLE.entry;
      } else {
        return null;
      }
    })();
    const text = (() => {
      if(this.props.filter !== '' && this.props.account) {
        return (
          <div style={{...rowStyle, ...AccountEntryRow.STYLE.text}}>
            <div style={AccountEntryRow.STYLE.highlightedText}>
              {this.props.account.account.name.slice(
                0, this.props.filter.length)}
            </div>
            {this.props.account.account.name.slice(this.props.filter.length)}
          </div>);
      } else if(this.props.account) {
        return (
          <div style={{...rowStyle, ...AccountEntryRow.STYLE.text}}>
            {this.props.account.account.name.toString()}
          </div>);
      } else {
        return (
          <div style={{...rowStyle, ...AccountEntryRow.STYLE.emptyText}}>
            Empty
          </div>);
      }
    })();
    const roles = (() => {
      if(this.props.account) {
        return (
          <div style={AccountEntryRow.STYLE.rolesWrapper}>
            <RolePanel roles={this.props.account.roles}/>
          </div>);
      } else {
        return null;
      }
    })();
    const link = (() => {
      if(this.props.account) {
        return( 
          <Router.Link key={id} to={`/account/${id}`}
            className={css(dynamic_style)}>
          {text}
          {roles}
        </Router.Link>);
      } else {
        return (
          <div className={css(dynamic_style)}>
            {text}
            {roles}
          </div>);
      }
    })();
    return (
      <Transition in={this.state.isOpen}
          appear={true}
          key={id}
          timeout={AccountEntryRow.TIMEOUTS}>
        {(state) =>
          <div style={(AccountEntryRow.ANIMATION_STYLE as any)[state]}>
            {link}
          </div>}
      </Transition>);
  }

  public componentDidMount() {
    this.setState({afterFirstRender: true});
  }

  private static readonly defaultProps = {
    account: null as AccountEntry,
    filter: ''
  };
  private static readonly STYLE = {
    contentMedium: {
      marginLeft: '38px'
    },
    contentLarge: {
      marginLeft: '38px'
    },
    text: {
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 0,
      flexShrink: 0,
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    emptyText: {
      font: '400 14px Roboto',
      color: '#8C8C8C',
      paddingLeft: '10px',
      textDecoration: 'none',
      cursor: 'default' as 'default'
    },
    entryListWrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column-reverse' as 'column-reverse'
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
    }
  };
  private static readonly DYNAMIC_STYLE = StyleSheet.create({
    entry: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      paddingLeft: '10px',
      paddingRight: '10px',
      textDecoration: 'none',
      cursor: 'pointer' as 'pointer',
      ':hover' : {
        backgroundColor: '#F8F8F8'
      },
      ':active' : {
        backgroundColor: '#F8F8F8'
      }
    }
  });
  private static readonly ANIMATION_STYLE = {
    entering: {
      maxHeight: 0,
      transform: 'scaleY(0)'
    },
    entered: {
      maxHeight: '34px',
      transform: 'scaleY(1)',
      transitionProperty: 'max-height, transform',
      transitionDuration: '200ms',
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `200ms`,
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    }
  };
  private static readonly TIMEOUTS = {
    enter: 5,
    entered: 200,
    exit: 200,
    exited:  200
  };
}
