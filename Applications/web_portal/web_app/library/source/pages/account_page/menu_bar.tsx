import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {HBoxLayout, Padding, VBoxLayout} from '../..';

interface Properties {

  /** Indicates the account item was clicked. */
  onAccountClick?: () => void;

  /** Indicates the risk controls item was clicked. */
  onRiskControlsClick?: () => void;

  /** Indicates the entitlements item was clicked. */
  onEntitlementsClick?: () => void;

  /** Indicates the compliance item was clicked. */
  onComplianceClick?: () => void;

  /** Indicates the profit and loss item was clicked. */
  onProfitAndLossClick?: () => void;
}

interface State {
  breakpoint: Breakpoint;
}

enum Breakpoint {
  SMALL,
  MEDIUM,
  LARGE
}

/** Displays the horizontal menu used to navigate an account's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {
      breakpoint: MenuBar.getBreakpoint()
    }
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public componentDidMount() {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
    const MenuBarPadding = (): JSX.Element => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return <div className={css(MenuBar.STYLE.smallPadding)}/>;
      }
      return <Padding size='30px'/>;
    };
    const menuIconContainerClassName = (() => {
      if(this.state.breakpoint === Breakpoint.SMALL) {
        return css(MenuBar.STYLE.small);
      }
    })();
    return (
      <HBoxLayout className={menuIconContainerClassName}>
        <Item iconSrc='resources/account/account-grey.svg' name='Account'
          breakpoint={this.state.breakpoint}
          onClick={this.props.onAccountClick}/>
        <MenuBarPadding/>
        <Item iconSrc='resources/account/risk-controls-grey.svg'
          name='Risk Controls' breakpoint={this.state.breakpoint}
          onClick={this.props.onRiskControlsClick}/>
        <MenuBarPadding/>
        <Item iconSrc='resources/account/entitlements-grey.svg'
          name='Entitlements' breakpoint={this.state.breakpoint}
          onClick={this.props.onEntitlementsClick}/>
        <MenuBarPadding/>
        <Item iconSrc='resources/account/compliance-grey.svg' name='Compliance'
          breakpoint={this.state.breakpoint}
          onClick={this.props.onComplianceClick}/>
        <MenuBarPadding/>
        <Item iconSrc='resources/account/profit-loss-grey.svg'
          name='Profit/Loss' breakpoint={this.state.breakpoint}
          onClick={this.props.onProfitAndLossClick}/>
      </HBoxLayout>);
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
    const newBreakpoint = MenuBar.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }

  private static defaultProps = {
    onAccountClick: () => {},
    onRiskControlsClick: () => {},
    onEntitlementsClick: () => {},
    onComplianceClick: () => {},
    onProfitAndLossClick: () => {}
  };
  private static STYLE = StyleSheet.create({
    base: {
      height: '40px'
    },
    small: {
      width: '100%',
      minWidth: '356px',
      maxWidth: '496px'
    },
    smallPadding: {
      width: '100%',
      maxWidth: '30px',
      height: '100%',
      flex: '1 1 auto'
    }
  });
}

interface ItemProperties {
  iconSrc: string;
  breakpoint: Breakpoint;
  name: string;
  onClick?: () => void;
}

class Item extends React.Component<ItemProperties> {
  public render(): JSX.Element {
    if(this.props.breakpoint === Breakpoint.SMALL) {
      return (
      <VBoxLayout height='40px'>
        <Padding size='8px'/>
        <img src={this.props.iconSrc} width='24px' height='24px'/>
        <Padding size='8px'/>
      </VBoxLayout>);
    }
    return (
      <VBoxLayout height='40px'>
        <Padding/>
        <HBoxLayout height='16px'>
          <img src={this.props.iconSrc} width='16px' height='16px'/>
          <Padding size='8px'/>
          <VBoxLayout height='16px'>
            <Padding/>
            <span className={css(Item.STYLE.label)}>
              {this.props.name}
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
