import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';

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
  selected: string;
  hovered: string;
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
      breakpoint: MenuBar.getBreakpoint(),
      selected: 'Account',
      hovered: ''
    };
    this.onScreenResize = this.onScreenResize.bind(this);
    this.onIconMouseEnter = this.onIconMouseEnter.bind(this);
    this.onIconMouseLeave = this.onIconMouseLeave.bind(this);
    this.onIconClick = this.onIconClick.bind(this);
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
        return css([MenuBar.STYLE.base, MenuBar.STYLE.small]);
      }
      return css(MenuBar.STYLE.base);  
    })();
    const getIconColor = (name: string) => {
      if(this.state.selected === name || this.state.hovered === name) {
        return 'purple';
      }
      return 'grey';
    };
    const getIconUnderlineClassName = (name: string) => {
      if(name === this.state.selected) {
        return css(MenuBar.STYLE.selectedBorder);
      }
      return css(MenuBar.STYLE.unselectedBorder);
    };
    const menuBarWidth = (() => {
      if(this.state.breakpoint !== Breakpoint.SMALL) {
        return '590px';
      }
    })();
    return (
      <HBoxLayout id='menu-bar' width={menuBarWidth}
          className={menuIconContainerClassName}>
        <VBoxLayout id='item-vbox'  height='40px'>
          <Item iconSrc={`resources/account/account` +
              `-${getIconColor('Account')}.svg`} name='Account'
            isSelected={this.state.selected === 'Account'}
            breakpoint={this.state.breakpoint}
            onMouseEnter={() => this.onIconMouseEnter('Account')}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick('Account',
              this.props.onAccountClick)}/>
          <div className={getIconUnderlineClassName('Account')}/>
        </VBoxLayout>
        <MenuBarPadding/>
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/risk-controls` +
            `-${getIconColor('Risk Controls')}.svg`} name='Risk Controls'
            isSelected={this.state.selected === 'Risk Controls'}
            breakpoint={this.state.breakpoint}
            onMouseEnter={() => this.onIconMouseEnter('Risk Controls')}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick('Risk Controls',
              this.props.onRiskControlsClick)}/>
          <div className={getIconUnderlineClassName('Risk Controls')}/>
        </VBoxLayout>
        <MenuBarPadding/>
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/entitlements` +
            `-${getIconColor('Entitlements')}.svg`} name='Entitlements'
            isSelected={this.state.selected === 'Entitlements'}
            breakpoint={this.state.breakpoint}
            onMouseEnter={() => this.onIconMouseEnter('Entitlements')}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick('Entitlements',
              this.props.onEntitlementsClick)}/>
          <div className={getIconUnderlineClassName('Entitlements')}/>
        </VBoxLayout>
        <MenuBarPadding/>
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/compliance` +
            `-${getIconColor('Compliance')}.svg`} name='Compliance'
            isSelected={this.state.selected === 'Compliance'}
            breakpoint={this.state.breakpoint}
            onMouseEnter={() => this.onIconMouseEnter('Compliance')}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick('Compliance',
              this.props.onComplianceClick)}/>
          <div className={getIconUnderlineClassName('Compliance')}/>
        </VBoxLayout>
        <MenuBarPadding/>
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/profit-loss` +
            `-${getIconColor('Profit/Loss')}.svg`}
            name='Profit/Loss' breakpoint={this.state.breakpoint}
            isSelected={this.state.selected === 'Profit/Loss'}
            onMouseEnter={() => this.onIconMouseEnter('Profit/Loss')}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick('Profit/Loss',
              this.props.onProfitAndLossClick)}/>
          <div className={getIconUnderlineClassName('Profit/Loss')}/>
        </VBoxLayout>
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

  private onIconMouseEnter(name: string) {
    this.setState({hovered: name});
  }

  private onIconMouseLeave() {
    this.setState({hovered: ''});
  }

  private onIconClick(name: string, onClick: () => void): void {
    onClick();
    this.setState({selected: name});
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
      height: '40px',
      width: '590px'
    },
    small: {
      width: '55%',
      minWidth: '176px',
      maxWidth: '308px'
    },
    smallPadding: {
      width: '100%',
      maxWidth: '30px',
      height: '100%',
      flex: '1 1 auto'
    },
    selectedBorder: {
      width: '100%',
      height: '2px',
      backgroundColor: '#4B23A0'
    },
    unselectedBorder: {
      width: '100%',
      height: '2px',
      backgroundColor: 'rgba(255, 255, 255, 0)'
    }
  });
}

interface ItemProperties {
  iconSrc: string;
  isSelected: boolean;
  breakpoint: Breakpoint;
  name: string;
  onMouseEnter?: () => void;
  onMouseLeave?: () => void;
  onClick?: () => void;
}

class Item extends React.Component<ItemProperties> {
  public render(): JSX.Element {
    const itemClassName = (() => {
      if(this.props.isSelected) {
        return css([Item.STYLE.item, Item.STYLE.selectedItem]);
      }
      return css([Item.STYLE.item, Item.STYLE.unSelectedItem]);
    })();
    if(this.props.breakpoint === Breakpoint.SMALL) {
      return (
      <VBoxLayout height='38px'>
        <Padding size='8px'/>
        <Center width='24px' height='24px' className={itemClassName}
            onMouseEnter={this.props.onMouseEnter}
            onMouseLeave={this.props.onMouseLeave} onClick={this.props.onClick}>
          <img src={this.props.iconSrc} width='20px' height='20px'/>
        </Center>
        <Padding size='6px'/>
      </VBoxLayout>);
    }
    return (
      <VBoxLayout className={itemClassName}
          onMouseEnter={this.props.onMouseEnter}
          onMouseLeave={this.props.onMouseLeave}
          onClick={this.props.onClick}>
        <Padding size='12px'/>
        <div className={css(Item.STYLE.container)}>
          <div className={css(Item.STYLE.wrapper)}>
            <img src={this.props.iconSrc} width='16px' height='16px'/>
            <div className={css(Item.STYLE.padding)}/>
            <Center id='center-1' height='16px'>
              <span className={css(Item.STYLE.label)}>{this.props.name}</span>
            </Center>
          </div>
        </div>
        <Padding size='10px'/>
      </VBoxLayout>);
  }

  private static defaultProps = {
    isSelected: false,
    onMouseEnter: () => {},
    onMouseLeave: () => {},
    onClick: () => {}
  };

  private static STYLE = StyleSheet.create({
    item: {
      height: '38px',
      cursor: 'pointer',
      '-webkit-tap-highlight-color': 'transparent',
      ':hover': {
        color: '#4B23A0'
      }
    },
    selectedItem: {
      color: '#4B23A0'
    },
    unSelectedItem: {
      color: '#7D7E90'
    },
    container: {
      height: '16px',
      display: 'flex',
      flexWrap: 'wrap'
    },
    wrapper: {
      height: '16px',
      width: 'auto',
      display: 'flex'
    },
    padding: {
      height: '16px',
      width: '8px'
    },
    label: {
      color: 'inherit',
      font: '200 14px Roboto',
      whiteSpace: 'nowrap'
    }
  });
}
