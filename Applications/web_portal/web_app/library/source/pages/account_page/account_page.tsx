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
  src: string;
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
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout height='40px' width='100%'
            className={css(AccountPage.STYLE.underlinedBox)}>
          <Padding size='18px'/>
          <HBoxLayout>
            <MenuItem src='resources/account/account-grey.svg'
              breakpoint={this.state.breakpoint}/>
            <Padding size='14px'/>
            <MenuItem src='resources/account/risk-controls-grey.svg'
              breakpoint={this.state.breakpoint}/>
            <Padding size='14px'/>
            <MenuItem src='resources/account/entitlements-grey.svg'
              breakpoint={this.state.breakpoint}/>
            <Padding size='14px'/>
            <MenuItem src='resources/account/compliance-grey.svg'
              breakpoint={this.state.breakpoint}/>
            <Padding size='14px'/>
            <MenuItem src='resources/account/profit-loss-grey.svg'
              breakpoint={this.state.breakpoint}/>
          </HBoxLayout>
        </HBoxLayout>
        <HBoxLayout height='30px' width='100%'
            className={css(AccountPage.STYLE.underlinedBox)}>
        </HBoxLayout>
      </VBoxLayout>
      );
  }

  private static getBreakpoint(): Breakpoint {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 559) {
      return Breakpoint.SMALL;
    } else if(screenWidth >= 560 && screenWidth <= 1039) {
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

  private static STYLE = StyleSheet.create({
    underlinedBox: {
      '-webkit-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      '-moz-box-shadow': '0px 1px 1px 0px rgba(149,152,154,0.16)',
      boxShadow: '0px 1px 1px 0px rgba(149,152,154,0.16)'
    }
  });
}

class MenuItem extends React.Component<MenuItemProperties> {
  render(): JSX.Element {
    return (
      <VBoxLayout height='40px'>
        <Padding size='8px'/>
        <img src={this.props.src} width='24px' height='24px'/>
        <Padding size='8px'/>
      </VBoxLayout>);
  }
}
