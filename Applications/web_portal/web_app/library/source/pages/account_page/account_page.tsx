import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';
import {AccountModel} from '.';

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
    const AccountHeaderPadding = (): JSX.Element => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return <Padding/>;
      }
      return <Padding size='30px'/>;
    };
    const accountHeaderWidth = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return '100%';
      } else if(this.state.breakpoint === Breakpoint.MEDIUM) {
        return '786px';
      }
      return '1036px';
    })();
    const menuIconContainerClassName = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css(AccountPage.ACCOUNT_HEADER_STYLE.smallMenuIconContainer);
      }
    })();
    const AccountHeader = ():JSX.Element => {
      return (
        <HBoxLayout className={menuIconContainerClassName}>
          <MenuItem isSelected={false}
            src='resources/account/account-grey.svg' label='Account'
            breakpoint={this.state.breakpoint}/>
          <AccountHeaderPadding/>
          <MenuItem isSelected={false}
            src='resources/account/risk-controls-grey.svg'
            label='Risk Controls' breakpoint={this.state.breakpoint}/>
          <AccountHeaderPadding/>
          <MenuItem isSelected={false}
            src='resources/account/entitlements-grey.svg'
            label='Entitlements' breakpoint={this.state.breakpoint}/>
          <AccountHeaderPadding/>
          <MenuItem isSelected={false}
            src='resources/account/compliance-grey.svg' label='Compliance'
            breakpoint={this.state.breakpoint}/>
          <AccountHeaderPadding/>
          <MenuItem isSelected={false}
            src='resources/account/profit-loss-grey.svg' label='Profit/Loss'
            breakpoint={this.state.breakpoint}/>
        </HBoxLayout>);
    };
    if(this.state.breakpoint === Breakpoint.LARGE) {
      return (
        <VBoxLayout width='100%' height='100%'>
          <HBoxLayout width='100%' height='40px'
            className={css(AccountPage.STYLE.underlinedBox)}>
          <Padding/>
          <HBoxLayout width={accountHeaderWidth}
              className={accountHeaderClassName}>
          <Padding size='18px'/>
          <AccountHeader/>
          <Padding/>
          <HBoxLayout>
            <Center className={css(AccountPage.STYLE.username)}>
              {this.props.model.account.name}
            </Center>
            <Padding size='10px'/>
            <VBoxLayout width='68px' height='40px'>
              <Padding/>
              <HBoxLayout width='68px' height='14px'>
                <img src='resources/account/trader-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/manager-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/admin-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/service-grey.svg' width='14px'
                  height='14px'/>
              </HBoxLayout>
              <Padding/>
            </VBoxLayout>
          </HBoxLayout>
          <Padding size='18px'/>
          </HBoxLayout>
          <Padding/>
        </HBoxLayout>
      </VBoxLayout>);
    }
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='40px'
            className={css(AccountPage.STYLE.underlinedBox)}>
          <Padding/>
          <HBoxLayout width={accountHeaderWidth}
              className={accountHeaderClassName}>
            <Padding size='18px'/>
            <AccountHeader/>
            <Padding/>
            <VBoxLayout width='68px' height='40px'>
              <Padding/>
              <HBoxLayout width='68px' height='14px'>
                <img src='resources/account/trader-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/manager-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/admin-grey.svg' width='14px'
                  height='14px'/>
                <Padding size='4px'/>
                <img src='resources/account/service-grey.svg' width='14px'
                  height='14px'/>
              </HBoxLayout>
              <Padding/>
            </VBoxLayout>
            <Padding size='18px'/>
          </HBoxLayout>
          <Padding/>
        </HBoxLayout>
        <HBoxLayout height='30px' width='100%'
          className={css(AccountPage.STYLE.underlinedBox)}>
          <Padding/>
          <Center className={css(AccountPage.STYLE.username)}>
            {this.props.model.account.name}
          </Center>
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
      width: '100%',
      minWidth: '320px',
      maxWidth: '460px'
    },
    smallMenuIconContainer: {
      width: 'calc(90% - 104px)',
      maxWidth: '240px',
      minWidth: '176px'
    }
  });
  private static STYLE = StyleSheet.create({
    underlinedBox: {
      '-webkit-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      '-moz-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      boxShadow: '0px 1px 1px 0px rgba(149,152,154,0.16)'
    },
    username: {
      height: '40px',
      font: '500 14px Roboto',
      color: '#4B23A0',
      whiteSpace: 'nowrap'
    }
  });
}

class MenuItem extends React.Component<MenuItemProperties> {
  public render(): JSX.Element {
    if(this.props.breakpoint === Breakpoint.SMALL) {
      return (
      <VBoxLayout height='40px'>
        <Padding size='8px'/>
        <img src={this.props.src} width='24px' height='24px'/>
        <Padding size='8px'/>
      </VBoxLayout>);
    }
    return (
      <VBoxLayout height='40px'>
        <Padding/>
        <HBoxLayout height='16px'>
          <img src={this.props.src} width='16px' height='16px'/>
          <Padding size='8px'/>
          <VBoxLayout height='16px'>
            <Padding/>
            <span className={css(MenuItem.STYLE.label)}>
              {this.props.label}
            </span>
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
        <Padding/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    label: {
      font: '200 14px Roboto',
      whiteSpace: 'nowrap'
    }
  });
}
