import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { HBoxLayout, Padding, VBoxLayout } from '../../..';

interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;

  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

interface State {
  showTraderText: boolean;
  showManagerText: boolean;
  showAdminText: boolean;
  showServiceText: boolean;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onClick: (_: string) => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      showTraderText: false,
      showAdminText: false,
      showServiceText: false,
      showManagerText: false
    };
    this.showToolTip = this.showToolTip.bind(this);
    this.hideToolTip = this.hideToolTip.bind(this);
  }

  public render(): JSX.Element {
    const getIconColour = (role: Nexus.AccountRoles.Role) => {
      if(this.props.roles.test(role)) {
        return 'purple';
      }
      return 'grey';
    };
    return (
      <HBoxLayout width={RolesField.COMPONENT_WIDTH}
          height={RolesField.IMAGE_SIZE}>
        <div className={css(RolesField.STYLE.imageBox)}>
          <img src={`resources/account/trader-${
              getIconColour(Nexus.AccountRoles.Role.TRADER)}.svg`}
            width={RolesField.IMAGE_SIZE}
            height={RolesField.IMAGE_SIZE}
            onClick={() =>
              this.props.onClick(Nexus.AccountRoles.Role.TRADER)}
            onMouseEnter={() =>
              this.showToolTip(Nexus.AccountRoles.Role.TRADER)}
            onMouseLeave={() =>
              this.hideToolTip(Nexus.AccountRoles.Role.TRADER)}/>
          <Transition in={this.state.showTraderText}
              timeout={RolesField.TIMEOUT}>
            {(state) => (
              <div className={css(RolesField.STYLE.animationBase,
                    (RolesField.ANIMATION_STYLE as any)[state])}>
                  <div className={css(RolesField.STYLE.imageTooltip)}>
                    {RolesField.TRADER_TOOLTIP_TEXT}
                  </div>
              </div>)}
          </Transition>
        </div>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <div className={css(RolesField.STYLE.imageBox)}>
          <img src={`resources/account/manager-${
              getIconColour(Nexus.AccountRoles.Role.MANAGER)}.svg`}
            width={RolesField.IMAGE_SIZE}
            height={RolesField.IMAGE_SIZE}
            onClick={() =>
              this.props.onClick(Nexus.AccountRoles.Role.MANAGER)}
            onMouseEnter={() =>
              this.showToolTip(Nexus.AccountRoles.Role.MANAGER)}
            onMouseLeave={() =>
              this.hideToolTip(Nexus.AccountRoles.Role.MANAGER)}/>
          <Transition in={this.state.showManagerText}
              timeout={RolesField.TIMEOUT}>
            {(state) => (
              <div className={css(RolesField.STYLE.animationBase,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                <div className={css(RolesField.STYLE.imageTooltip)}>
                  {RolesField.MANAGER_TOOLTIP_TEXT}
                </div>
              </div>)}
          </Transition>
        </div>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <div className={css(RolesField.STYLE.imageBox)}>
          <img src={`resources/account/admin-${
              getIconColour(Nexus.AccountRoles.Role.ADMINISTRATOR)}.svg`}
            width={RolesField.IMAGE_SIZE}
            height={RolesField.IMAGE_SIZE}
            onClick={() =>
              this.props.onClick(Nexus.AccountRoles.Role.ADMINISTRATOR)}
            onMouseEnter={() =>
              this.showToolTip(Nexus.AccountRoles.Role.ADMINISTRATOR)}
            onMouseLeave={() =>
              this.hideToolTip(Nexus.AccountRoles.Role.ADMINISTRATOR)}/>
          <Transition in={this.state.showAdminText}
            timeout={RolesField.TIMEOUT}>
            {(state) => (
              <div className={css(RolesField.STYLE.animationBase,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                <div className={css(RolesField.STYLE.imageTooltip)}>
                  {RolesField.ADMINISTRATOR_TOOLTIP_TEXT}
                </div>
              </div>)}
          </Transition>
        </div>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <div className={css(RolesField.STYLE.imageBox)}>
          <img src={`resources/account/service-${
              getIconColour(Nexus.AccountRoles.Role.SERVICE)}.svg`}
            width={RolesField.IMAGE_SIZE}
            height={RolesField.IMAGE_SIZE}
            onClick={() =>
              this.props.onClick(Nexus.AccountRoles.Role.SERVICE)}
            onMouseEnter={() =>
              this.showToolTip(Nexus.AccountRoles.Role.SERVICE)}
            onMouseLeave={() =>
              this.hideToolTip(Nexus.AccountRoles.Role.SERVICE)}/>
          <Transition in={this.state.showServiceText}
            timeout={RolesField.TIMEOUT}>
            {(state) => (
              <div className={css(RolesField.STYLE.animationBase,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                <div className={css(RolesField.STYLE.imageTooltip)}>
                  {RolesField.SERVICE_TOOLTIP_TEXT}
                </div>
              </div>)}
          </Transition>
        </div>
      </HBoxLayout>);
  }

  private showToolTip(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        this.setState({ showTraderText: true });
        break;
      case Nexus.AccountRoles.Role.MANAGER:
        this.setState({ showManagerText: true });
        break;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        this.setState({ showAdminText: true });
        break;
      case Nexus.AccountRoles.Role.SERVICE:
        this.setState({ showServiceText: true });
        break;
    }
  }

  private hideToolTip(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        this.setState({ showTraderText: false });
        break;
      case Nexus.AccountRoles.Role.MANAGER:
        this.setState({ showManagerText: false });
        break;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        this.setState({ showAdminText: false });
        break;
      case Nexus.AccountRoles.Role.SERVICE:
        this.setState({ showServiceText: false });
        break;
    }
  }

  private static ANIMATION_STYLE = StyleSheet.create({
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    },
    exited: {
      display: 'none' as 'none'
    }
  });
  private static STYLE = StyleSheet.create({
    imageBox: {
      position: 'relative' as 'relative'
    },
    animationBase: {
      opacity: 0,
      transition: 'opacity 100ms ease-in-out'
    },
    imageTooltip: {
      font: '400 12px Roboto',
      paddingLeft: '15px',
      paddingRight: '15px',
      paddingTop: '30%',
      height: '22px',
      backgroundColor: '#4B23A0',
      color: '#FFFFFF',
      position: 'absolute',
      zIndex: 5,
      top: '28px',
      left: '-2px',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      boxShadow: '0px 0px 2px #00000064'
    }
  });
  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_PADDING = '14px';
  private static readonly COMPONENT_WIDTH = '68px';
  private static readonly TIMEOUT = 100;
  private static readonly TRADER_TOOLTIP_TEXT = 'Trader';
  private static readonly MANAGER_TOOLTIP_TEXT  = 'Manager';
  private static readonly ADMINISTRATOR_TOOLTIP_TEXT  = 'Admin';
  private static readonly SERVICE_TOOLTIP_TEXT = 'Service';
}
