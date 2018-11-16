import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Center, DisplaySize, HBoxLayout, Padding, VBoxLayout} from '../..';

interface Properties {

  /** The type of display to render on. */
  displaySize: DisplaySize;

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

enum MenuItem {
  ACCOUNT,
  RISK_CONTROLS,
  ENTITLEMENTS,
  COMPLIANCE,
  PROFIT_LOSS
}

interface State {
  selected: MenuItem;
  hovered: MenuItem;
}

/** Displays the horizontal menu used to navigate an account's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public static defaultProps = {
    onAccountClick: () => {},
    onRiskControlsClick: () => {},
    onEntitlementsClick: () => {},
    onComplianceClick: () => {},
    onProfitAndLossClick: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      selected: MenuItem.ACCOUNT,
      hovered: null
    };
    this.onIconMouseEnter = this.onIconMouseEnter.bind(this);
    this.onIconMouseLeave = this.onIconMouseLeave.bind(this);
    this.onIconClick = this.onIconClick.bind(this);
  }

  public render(): JSX.Element {
    const menuBarPadding = ((): JSX.Element => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <div className={css(MenuBar.STYLE.smallPadding)}/>;
      }
      return <Padding size='30px'/>;
    })();
    const menuIconContainerClassName = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return css([MenuBar.STYLE.base, MenuBar.STYLE.small]);
      }
      return css(MenuBar.STYLE.base);
    })();
    const menuBarWidth = (() => {
      if(this.props.displaySize !== DisplaySize.SMALL) {
        return '590px';
      }
    })();
    return (
      <HBoxLayout id='menu-bar' width={menuBarWidth}
          className={menuIconContainerClassName}>
        <VBoxLayout id='item-vbox'  height='40px'>
          <Item iconSrc={`resources/account/account` +
              `-${this.getIconColor(MenuItem.ACCOUNT)}.svg`} name='Account'
            isSelected={this.state.selected === MenuItem.ACCOUNT}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(MenuItem.ACCOUNT)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(MenuItem.ACCOUNT,
              this.props.onAccountClick)}/>
          <div className={this.getIconUnderlineClassName(MenuItem.ACCOUNT)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/risk-controls` +
            `-${this.getIconColor(MenuItem.RISK_CONTROLS)}.svg`}
            name='Risk Controls'
            isSelected={this.state.selected === MenuItem.RISK_CONTROLS}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(MenuItem.RISK_CONTROLS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(MenuItem.RISK_CONTROLS,
              this.props.onRiskControlsClick)}/>
          <div className={this.getIconUnderlineClassName(
            MenuItem.RISK_CONTROLS)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/entitlements` +
            `-${this.getIconColor(MenuItem.ENTITLEMENTS)}.svg`}
            name='Entitlements'
            isSelected={this.state.selected === MenuItem.ENTITLEMENTS}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(MenuItem.ENTITLEMENTS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(MenuItem.ENTITLEMENTS,
              this.props.onEntitlementsClick)}/>
          <div className={this.getIconUnderlineClassName(
            MenuItem.ENTITLEMENTS)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/compliance` +
            `-${this.getIconColor(MenuItem.COMPLIANCE)}.svg`} name='Compliance'
            isSelected={this.state.selected === MenuItem.COMPLIANCE}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(MenuItem.COMPLIANCE)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(MenuItem.COMPLIANCE,
              this.props.onComplianceClick)}/>
          <div className={this.getIconUnderlineClassName(MenuItem.COMPLIANCE)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={`resources/account/profit-loss` +
            `-${this.getIconColor(MenuItem.PROFIT_LOSS)}.svg`}
            name='Profit/Loss' displaySize={this.props.displaySize}
            isSelected={this.state.selected === MenuItem.PROFIT_LOSS}
            onMouseEnter={() => this.onIconMouseEnter(MenuItem.PROFIT_LOSS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(MenuItem.PROFIT_LOSS,
              this.props.onProfitAndLossClick)}/>
          <div className={this.getIconUnderlineClassName(
            MenuItem.PROFIT_LOSS)}/>
        </VBoxLayout>
      </HBoxLayout>);
  }

  private getIconColor(item: MenuItem) {
    if(this.state.selected === item || this.state.hovered === item) {
      return 'purple';
    }
    return 'grey';
  }

  private getIconUnderlineClassName(item: MenuItem) {
    if(item === this.state.selected) {
      return css(MenuBar.STYLE.selectedBorder);
    }
    return css(MenuBar.STYLE.unselectedBorder);
  }

  private onIconMouseEnter(item: MenuItem) {
    this.setState({hovered: item});
  }

  private onIconMouseLeave() {
    this.setState({hovered: null});
  }

  private onIconClick(item: MenuItem, onClick: () => void): void {
    onClick();
    this.setState({selected: item});
  }

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
  displaySize: DisplaySize;
  name: string;
  onMouseEnter?: () => void;
  onMouseLeave?: () => void;
  onClick?: () => void;
}

class Item extends React.Component<ItemProperties> {
  public static defaultProps = {
    isSelected: false,
    onMouseEnter: () => {},
    onMouseLeave: () => {},
    onClick: () => {}
  };

  public render(): JSX.Element {
    const itemClassName = (() => {
      if(this.props.isSelected) {
        return css([Item.STYLE.item, Item.STYLE.selectedItem]);
      }
      return css([Item.STYLE.item, Item.STYLE.unSelectedItem]);
    })();
    if(this.props.displaySize === DisplaySize.SMALL) {
      return (
        <VBoxLayout height='38px'>
          <Padding size='8px'/>
          <Center width='24px' height='24px' className={itemClassName}
              onMouseEnter={this.props.onMouseEnter}
              onMouseLeave={this.props.onMouseLeave}
              onClick={this.props.onClick}>
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
      font: '400 14px Roboto',
      whiteSpace: 'nowrap'
    }
  });
}
