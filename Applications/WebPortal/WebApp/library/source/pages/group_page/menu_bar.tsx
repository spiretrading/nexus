import { css, StyleSheet } from 'aphrodite';
import { Center, HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { DisplaySize } from '../..';
import { GroupSubPage } from './group_sub_page';

interface Properties {

  /** The sub-page currently selected. */
  selected: GroupSubPage;

  /** The type of display to render on. */
  displaySize: DisplaySize;

  /** Indicates a menu item was clicked.
   * @param subPage - The sub-page item that was clicked.
   */
  onClick?: (subPage: GroupSubPage) => void;
}

interface State {
  hovered: GroupSubPage;
}

/** Displays the horizontal menu used to navigate a group's properties. */
export class MenuBar extends React.Component<Properties, State> {
  public static defaultProps = {
    onClick: () => {},
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      hovered: GroupSubPage.NONE
    };
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
        return '343px';
      }
    })();
    return (
      <HBoxLayout id='menu-bar' width={menuBarWidth}
          style={menuIconContainerClassName}>
        <VBoxLayout id='item-vbox'  height='40px'>
          <Item iconSrc={'resources/group_page/group' +
              `-${this.getIconColor(GroupSubPage.GROUP)}.svg`} name='Group'
            isSelected={this.props.selected === GroupSubPage.GROUP}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(GroupSubPage.GROUP)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(GroupSubPage.GROUP)}/>
          <div style={this.getIconUnderlineStyle(GroupSubPage.GROUP)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/group_page/compliance' +
              `-${this.getIconColor(GroupSubPage.COMPLIANCE)}.svg`}
            name='Compliance'
            isSelected={this.props.selected === GroupSubPage.COMPLIANCE}
            displaySize={this.props.displaySize}
            onMouseEnter={() => this.onIconMouseEnter(GroupSubPage.COMPLIANCE)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(GroupSubPage.COMPLIANCE)}/>
          <div style={this.getIconUnderlineStyle(GroupSubPage.COMPLIANCE)}/>
        </VBoxLayout>
        {menuBarPadding}
        <VBoxLayout height='40px'>
          <Item iconSrc={'resources/group_page/profit-loss' +
              `-${this.getIconColor(GroupSubPage.PROFIT_LOSS)}.svg`}
            name='Profit/Loss' displaySize={this.props.displaySize}
            isSelected={this.props.selected === GroupSubPage.PROFIT_LOSS}
            onMouseEnter={
              () => this.onIconMouseEnter(GroupSubPage.PROFIT_LOSS)}
            onMouseLeave={this.onIconMouseLeave}
            onClick={() => this.onIconClick(GroupSubPage.PROFIT_LOSS)}/>
          <div style={this.getIconUnderlineStyle(GroupSubPage.PROFIT_LOSS)}/>
        </VBoxLayout>
        {menuBarPadding}
      </HBoxLayout>);
  }

  private getIconColor = (subPage: GroupSubPage) => {
    if(this.props.selected === subPage || this.state.hovered === subPage) {
      return 'purple';
    }
    return 'grey';
  }

  private getIconUnderlineStyle = (subPage: GroupSubPage) => {
    if(subPage === this.props.selected) {
      return MenuBar.STYLE.selectedBorder;
    }
    return MenuBar.STYLE.unselectedBorder;
  }

  private onIconMouseEnter = (subPage: GroupSubPage) => {
    this.setState({hovered: subPage});
  }

  private onIconMouseLeave = () => {
    this.setState({hovered: GroupSubPage.NONE});
  }

  private onIconClick = (subPage: GroupSubPage) => {
    if(this.props.selected !== subPage) {
      this.props.onClick(subPage);
    }
  }

  private static readonly STYLE = {
    base: {
      height: '40px',
      width: '343px'
    },
    small: {
      width: '55%',
      minWidth: '114px',
      maxWidth: '162px'
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
  };
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
