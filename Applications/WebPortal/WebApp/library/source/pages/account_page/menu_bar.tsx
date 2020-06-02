import { css, StyleSheet } from 'aphrodite';
import { Center, HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { DisplaySize } from '../..';
import { SubPage } from './sub_page';

interface Properties {

  /** The sub-page currently selected. */
  selected: SubPage;

  /** The type of display to render on. */
  displaySize: DisplaySize;

  /** Indicates a menu item was clicked.
   * @param subPage - The sub-page item that was clicked.
   */
  onClick?: (subPage: SubPage) => void;
}

interface State {
  hovered: SubPage;
}

/** Displays the horizontal menu used to navigate an account's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public static defaultProps = {
    onClick: () => {},
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      hovered: SubPage.NONE
    };
    this.onIconMouseEnter = this.onIconMouseEnter.bind(this);
    this.onIconMouseLeave = this.onIconMouseLeave.bind(this);
    this.onIconClick = this.onIconClick.bind(this);
  }

  public render(): JSX.Element {
    const menuBarPadding = ((): JSX.Element => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <div style={MenuBar.STYLE.smallPadding}/>;
      }
      return <Padding size='30px'/>;
    })();
    const menuIconContainerClassName = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return {...MenuBar.STYLE.base, ...MenuBar.STYLE.small};
      }
      return MenuBar.STYLE.base;
    })();
    const menuBarWidth = (() => {
      if(this.props.displaySize !== DisplaySize.SMALL) {
        return '590px';
      }
    })();
    return (
      <HBoxLayout id='menu-bar' width={menuBarWidth}
          style={menuIconContainerClassName}>
        <VBoxLayout id='item-vbox'  height='40px'>
          <Item iconSrc={'resources/account/account' +
              `-${this.getIconColor(SubPage.PROFILE)}.svg`} name='Account'
            isSelected={this.props.selected === SubPage.PROFILE}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(SubPage.PROFILE)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(SubPage.PROFILE)}/>
          <div style={this.getIconUnderlineStyle(SubPage.PROFILE)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/account/risk-controls' +
              `-${this.getIconColor(SubPage.RISK_CONTROLS)}.svg`}
            name='Risk Controls'
            isSelected={this.props.selected === SubPage.RISK_CONTROLS}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(SubPage.RISK_CONTROLS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(SubPage.RISK_CONTROLS)}/>
          <div style={this.getIconUnderlineStyle(SubPage.RISK_CONTROLS)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/account/entitlements' +
              `-${this.getIconColor(SubPage.ENTITLEMENTS)}.svg`}
            name='Entitlements'
            isSelected={this.props.selected === SubPage.ENTITLEMENTS}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(SubPage.ENTITLEMENTS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(SubPage.ENTITLEMENTS)}/>
          <div style={this.getIconUnderlineStyle(SubPage.ENTITLEMENTS)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/account/compliance' +
              `-${this.getIconColor(SubPage.COMPLIANCE)}.svg`}
            name='Compliance'
            isSelected={this.props.selected === SubPage.COMPLIANCE}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(SubPage.COMPLIANCE)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(SubPage.COMPLIANCE)}/>
          <div style={this.getIconUnderlineStyle(SubPage.COMPLIANCE)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/account/profit-loss' +
              `-${this.getIconColor(SubPage.PROFIT_LOSS)}.svg`}
            name='Profit/Loss' displaySize={this.props.displaySize}
            isSelected={this.props.selected === SubPage.PROFIT_LOSS}
            onMouseEnter={() => this.onIconMouseEnter(SubPage.PROFIT_LOSS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(SubPage.PROFIT_LOSS)}/>
          <div style={this.getIconUnderlineStyle(SubPage.PROFIT_LOSS)}/>
        </VBoxLayout>
      </HBoxLayout>);
  }

  private getIconColor(subPage: SubPage) {
    if(this.props.selected === subPage || this.state.hovered === subPage) {
      return 'purple';
    }
    return 'grey';
  }

  private getIconUnderlineStyle(subPage: SubPage) {
    if(subPage === this.props.selected) {
      return MenuBar.STYLE.selectedBorder;
    }
    return MenuBar.STYLE.unselectedBorder;
  }

  private onIconMouseEnter(subPage: SubPage) {
    this.setState({hovered: subPage});
  }

  private onIconMouseLeave() {
    this.setState({hovered: SubPage.NONE});
  }

  private onIconClick(subPage: SubPage) {
    if(this.props.selected !== subPage) {
      this.props.onClick(subPage);
    }
  }

  private static readonly STYLE = {
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
  }
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
