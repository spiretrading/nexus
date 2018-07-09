import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';
import {AccountModel} from '.';
import {MenuBar} from './menu_bar';
import {RolePanel} from './role_panel';

interface Properties {

  /** The account's model. */
  model: AccountModel;
}

interface State {
  isLoading: boolean;
  breakpoint?: Breakpoint;
}

enum Breakpoint {
  SMALL,
  MEDIUM,
  LARGE
}
interface MenuItemProperties {
  isSelected: boolean;
  src: string;
  label: string;
  breakpoint: Breakpoint;
}

/** Implements the container used to display account information. */
export class AccountPage extends React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {
      isLoading: true,
      breakpoint: AccountPage.getBreakpoint()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public componentWillMount() {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoading: false
      });
    });
  }

  public componentDidMount() {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
    const accountHeaderClassName = ((): string => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css([AccountPage.ACCOUNT_HEADER_STYLE.base,
          AccountPage.ACCOUNT_HEADER_STYLE.small]);
      }
      return css(AccountPage.ACCOUNT_HEADER_STYLE.base);
    })();
    const accountHeaderWidth = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return '100%';
      } else if(this.state.breakpoint === Breakpoint.MEDIUM) {
        return '786px';
      }
      return '1036px';
    })();
    if(this.state.breakpoint === Breakpoint.LARGE) {
      return (
        <VBoxLayout width='100%' height='100%'>
          <HBoxLayout width='100%' height='40px'
              className={css(AccountPage.STYLE.underlinedBox)}>
            <Padding/>
            <HBoxLayout width={accountHeaderWidth}
                className={accountHeaderClassName}>
              <Padding size='18px'/>
              <MenuBar/>
              <Padding/>
              <HBoxLayout>
                <Center height='40px' className={css(AccountPage.STYLE.username)}>
                  {this.props.model.account.name}
                </Center>
                <Padding size='10px'/>
                <RolePanel roles={this.props.model.roles}/>
              </HBoxLayout>
              <Padding size='18px'/>
            </HBoxLayout>
            <Padding/>
          </HBoxLayout>
        </VBoxLayout>);
    }
    const accountContentsClassName = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.smallContainer);
      } else {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.mediumContainer);
      }
    })();
    const accountContentsPaddingClassName = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.smallContainerPadding);
      } else {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.mediumContainerPadding);
      }
    })();
    const accountHeaderPaddingClassName = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.smallPadding);
      } else {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.mediumPadding);
      }
    })();
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='40px'
            className={css(AccountPage.STYLE.underlinedBox)}>
          <div className={accountHeaderPaddingClassName}/>
          <Padding size='18px'/>
          <HBoxLayout height='40px' className={accountContentsClassName}>
            <MenuBar/>
            <div className={accountContentsPaddingClassName}/>
            <RolePanel roles={this.props.model.roles}/>
          </HBoxLayout>
          <Padding size='18px'/>
        </HBoxLayout>
        <HBoxLayout height='30px' width='100%'
            className={css(AccountPage.STYLE.underlinedBox)}>
          <Padding/>
          <Padding size='18px'/>
          <Center height='30px' className={css(AccountPage.STYLE.username)}>
            {this.props.model.account.name}
          </Center>
          <Padding size='18px'/>
        </HBoxLayout>
      </VBoxLayout>);
  }

  private static getBreakpoint(): Breakpoint {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return Breakpoint.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return Breakpoint.MEDIUM;
    } else {
      return Breakpoint.LARGE;
    }
  }

  private onScreenResize(): void {
    const newBreakpoint = AccountPage.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }
  private static ACCOUNT_HEADER_STYLE = StyleSheet.create({
    base: {
      height: '40px'
    },
    small: {
      width: '100%'
    },
    smallPadding: {
      width: 'calc(20% - 18px)'
    },
    mediumPadding: {
      width: 'calc(50% - 393px)',
      height: '100%'
    },
    smallContainer: {
      width: '60%',
      minWidth: '320px',
      maxWidth: '460px'
    },
    mediumContainer: {
      width: '768px'
    },
    smallContainerPadding: {
      width: 'calc(45% - 68px)',
      height: '100%'
    },
    mediumContainerPadding: {
      width: '100%',
      height: '100%',
      flex: '1 1 auto'
    }
  });
  private static STYLE = StyleSheet.create({
    underlinedBox: {
      '-webkit-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      '-moz-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      boxShadow: '0px 1px 1px 0px rgba(149,152,154,0.16)'
    },
    username: {
      font: '500 14px Roboto',
      color: '#4B23A0',
      whiteSpace: 'nowrap'
    }
  });
}
