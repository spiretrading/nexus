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
  showToolTip: boolean;
  showTraderText: boolean;
  showManagerText: boolean;
  showAdminText: boolean;
  showServiceText: boolean;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      showToolTip: false,
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
      if (this.props.roles.test(role)) {
        return 'purple';
      }
      return 'grey';
    };
    const traderImageSrc = (() => {
      if (this.props.roles.test(Nexus.AccountRoles.Role.TRADER)) {
        return 'resources/account/trader-purple.svg';
      } else {
        return 'resources/account/trader-grey.svg';
      }
    })();
    const serviceImageSrc = (() => {
      if (this.props.roles.test(Nexus.AccountRoles.Role.TRADER)) {
        return 'resources/account/service-purple.svg';
      } else {
        return 'resources/account/service-grey.svg';
      }
    })();
    return (
      <VBoxLayout>
        <HBoxLayout width='68px' height={RolesField.IMAGE_SIZE}>
          <div className={css(RolesField.STYLE.box)}>
            <img src={traderImageSrc}
              width={RolesField.IMAGE_SIZE}
              height={RolesField.IMAGE_SIZE}
              onClick={() =>
                this.props.onClick(Nexus.AccountRoles.Role.TRADER)}
              onMouseEnter={() =>
                this.showToolTip(Nexus.AccountRoles.Role.TRADER)}
              onMouseLeave={() =>
                this.hideToolTip(Nexus.AccountRoles.Role.TRADER)} />
            <Transition in={this.state.showTraderText}
              mountOnEnter={true}
              timeout={RolesField.TIMEOUT}>
              {(state) => (
                <div 
                  className={css(RolesField.STYLE.base,
                    (RolesField.ANIMATION_STYLE as any)[state])}>
                    <div className={css(RolesField.STYLE.imageTooltip)}>
                      TRADER
                    </div>
                </div>)}
            </Transition>
          </div>
          <Padding size={RolesField.IMAGE_PADDING} />
          <div className={css(RolesField.STYLE.box)}>
            <img src={`resources/account/manager-${
              getIconColour(Nexus.AccountRoles.Role.MANAGER)}.svg`}
              width={RolesField.IMAGE_SIZE}
              height={RolesField.IMAGE_SIZE}
              onClick={() =>
                this.props.onClick(Nexus.AccountRoles.Role.MANAGER)}
              onMouseEnter={() =>
                this.showToolTip(Nexus.AccountRoles.Role.MANAGER)}
              onMouseLeave={() =>
                this.hideToolTip(Nexus.AccountRoles.Role.MANAGER)} />
            <Transition in={this.state.showManagerText}
              timeout={RolesField.TIMEOUT}>
              {(state) => (
                <div
                  className={css(RolesField.STYLE.base,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                    <div className={css(RolesField.STYLE.imageTooltip)}>
                      Manager
                    </div>
                </div>)}
            </Transition>
          </div>
          <Padding size={RolesField.IMAGE_PADDING} />
          <div className={css(RolesField.STYLE.box)}>
            <img src={`resources/account/admin-${
              getIconColour(Nexus.AccountRoles.Role.ADMINISTRATOR)}.svg`}
              width={RolesField.IMAGE_SIZE}
              height={RolesField.IMAGE_SIZE}
              onClick={() =>
                this.props.onClick(Nexus.AccountRoles.Role.ADMINISTRATOR)}
              onMouseEnter={() =>
                this.showToolTip(Nexus.AccountRoles.Role.ADMINISTRATOR)}
              onMouseLeave={() =>
                this.hideToolTip(Nexus.AccountRoles.Role.ADMINISTRATOR)} />
            <Transition in={this.state.showAdminText}
              timeout={RolesField.TIMEOUT}>
              {(state) => (
                <div
                  className={css(RolesField.STYLE.base,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                    <div className={css(RolesField.STYLE.imageTooltip)}>
                      Admin
                    </div>
                </div>)}
            </Transition>
          </div>
          <Padding size={RolesField.IMAGE_PADDING} />
          <div className={css(RolesField.STYLE.box)}>
            <img src={serviceImageSrc}
              width={RolesField.IMAGE_SIZE}
              height={RolesField.IMAGE_SIZE}
              onClick={() =>
                this.props.onClick(Nexus.AccountRoles.Role.SERVICE)}
              onMouseEnter={() =>
                this.showToolTip(Nexus.AccountRoles.Role.SERVICE)}
              onMouseLeave={() =>
                this.hideToolTip(Nexus.AccountRoles.Role.SERVICE)} />
            <Transition in={this.state.showServiceText}
              timeout={RolesField.TIMEOUT}>
              {(state) => (
                <div
                  className={css(RolesField.STYLE.base,
                  (RolesField.ANIMATION_STYLE as any)[state])}>
                    <div className={css(RolesField.STYLE.imageTooltip)}>
                      Service
                    </div>
                </div>)}
            </Transition>
          </div>
        </HBoxLayout>
      </VBoxLayout>);
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
    box: {
      position: 'relative' as 'relative'
    },
    base: {
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
      visibility: 'visible' as 'visible',
      /* Position the tooltip text - see examples below! */
      position: 'absolute',
      zIndex: 5,
      top: '28px',
      left: '-2px',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      boxShadow: '0px 0px 2px #000000CC'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  });
  private static readonly IMAGE_SIZE = '24px';
  private static readonly IMAGE_PADDING = '18px';
  private static readonly TIMEOUT = 100;
}
