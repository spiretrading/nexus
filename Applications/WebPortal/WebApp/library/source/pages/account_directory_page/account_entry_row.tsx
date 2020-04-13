import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, RolePanel, AccountEntry} from '../..';
import { AccountDirectoryController } from './account_directory_controller';

interface Properties {
  displaySize: DisplaySize;
  account?: AccountEntry;
  filter?: string;
  isOpen: boolean;
  onDirectoryEntryClick?: (entry: Beam.DirectoryEntry) => void;
}

interface State {
  afterFirstRender: boolean;
  isOpen: boolean;
}

export class AccountEntryRow extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    filter: '',
    account: null as AccountEntry,
    onDirectoryEntryClick: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      afterFirstRender: false,
      isOpen: false
    }
  }

  public render(): JSX.Element {
    console.log('render');
    const accountsLabelStyle = (() => {
      switch(this.props.displaySize) {
        case(DisplaySize.SMALL):
          return null;
        case(DisplaySize.MEDIUM):
          return AccountEntryRow.STYLE.accountLabelMedium;
        case(DisplaySize.LARGE):
          return AccountEntryRow.STYLE.accountLabelLarge;
      }
    })();
    console.log();
    const id = (() => {
      if(this.props.account) {
        return this.props.account.account.id;
      } else {
        return 0;
      }
    })();
    console.log('id');
    const dynamic_style = (() => {
      if(this.props.account){
        return AccountEntryRow.DYNAMIC_STYLE.accountBox;
      } else {
        return null;
      }
    })();
    console.log('dynamic style');
    const animation_style = (() => {
      if(!this.props.filter) {
        return AccountEntryRow.accountLabelAnimationStyle;
      }else {
        return AccountEntryRow.noAnimationStyle;
      }
    })();
    console.log('animation style');
    const text = (() => {
      if(this.props.filter !== '' && this.props.account) {
        return (
          <div style={{...accountsLabelStyle,
              ...AccountEntryRow.STYLE.accountLabelText}}>
            <div style={AccountEntryRow.STYLE.highlightedText}>
              {this.props.account.account.name.slice(0, this.props.filter.length)}
            </div>
            {this.props.account.account.name.slice(this.props.filter.length)}
          </div>);
      } else if(this.props.account) {
        return (
          <div style={{...accountsLabelStyle,
              ...AccountEntryRow.STYLE.accountLabelText}}>
            {this.props.account.account.name.toString()}
          </div>);
      } else {
        return (
          <div style={{...accountsLabelStyle,
              ...AccountEntryRow.STYLE.accountLabelText}}>
            Empty
          </div>);
      }
    })();
    console.log('content stuffs');
    const roles = (() =>{
      if(this.props.account) {
        return ( 
          <div style={AccountEntryRow.STYLE.rolesWrapper}>
            <RolePanel roles={this.props.account.roles}/>
          </div>);
      } else {
        return null;
      }
    })();
    console.log('roles');
    return (
      <Transition in={this.state.isOpen}
          appear={true}
          key={id}
          timeout={AccountEntryRow.TIMEOUTS}>
        {(state) => (
          <div key={id}
              className={css( dynamic_style,
                (animation_style as any)[state])}
              onClick={() =>
                this.props.onDirectoryEntryClick(this.props.account.account)}>
            {text}
            {roles}
          </div>
        )}
      </Transition>);
  }
  

  public componentDidMount() {
    this.setState({afterFirstRender: true});
  }

  static getDerivedStateFromProps(props: Properties, state: State) {
    if(state.afterFirstRender) {
      if(props.isOpen && !state.isOpen) {
        return {isOpen: true};
      } else if(!props.isOpen && state.isOpen) {
        return {isOpen: false};
      } else {
        return null;
      }
    }
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
    emptyLabelText: {
      font: '400 14px Roboto',
      color: '#8C8C8C',
      paddingLeft: '10px',
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
      width: '100%',
      maxHeight: 0,
      transform: 'scaleY(0)'
    },
    entered: {
      width: '100%',
      maxHeight: '34px',
      transform: 'scaleY(1)',
      transitionProperty: 'max-height, transform',
      transitionDuration: '200ms',
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    },
    exiting: {
      width: '100%',
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `200ms`,
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    },
    exited: {
      width: '100%',
      maxHeight: 0,
      transform: 'scaleY(0)',
      transformOrigin: 'top' as 'top',
      overflow: 'hidden' as 'hidden'
    }
  });
  private static readonly noAnimationStyle = StyleSheet.create({
    entering: {
      width: '100%'
    },
    entered: {
      width: '100%'

    },
    exiting: {
      width: '100%'
    },
    exited: {
      width: '100%'
    }
  });
  private static readonly TIMEOUTS = {
    enter: 1,
    entered: 200,
    exit: 200,
    exited:  200
  };
}
