import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';

interface Properties {

  /** The role the icon respresents */
  role: Nexus.AccountRoles.Role;

  /** Determines if the role is set of not. */
  isSet: boolean;

  /** Called when icon is clicked on. */
  onClick?: () => void;
}

interface State {
  showToolTip: boolean;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RoleIcon extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      showToolTip: false
    };
    this.showToolTip = this.showToolTip.bind(this);
    this.hideToolTip = this.hideToolTip.bind(this);
    this.getSource = this.getSource.bind(this);
    this.getText = this.getText.bind(this);
  }

  public render(): JSX.Element {
    const iconColor = (() => {
      if(this.props.isSet) {
        return 'purple';
      }
      return 'grey';
    })();
    const source = (() => {
      return this.getSource(this.props.role);
    })();
    const tooltipText = (() => {
      return this.getText(this.props.role);
    })();
    return (
      <div style={RoleIcon.STYLE.iconBox}>
        <img src={`${source}${iconColor}.svg`}
          style={RoleIcon.STYLE.icon}
          width={RoleIcon.IMAGE_SIZE}
          height={RoleIcon.IMAGE_SIZE}
          onClick={this.props.onClick}
          onMouseEnter={() =>
            this.showToolTip()}
          onMouseLeave={() =>
            this.hideToolTip()}/>
        <Transition in={this.state.showToolTip} timeout={RoleIcon.TIMEOUT}>
          {(state) => (
            <div style={{...RoleIcon.STYLE.animationBase,
                  ...(RoleIcon.ANIMATION_STYLE as any)[state]}}>
              <div style={RoleIcon.STYLE.imageTooltip}>
                {tooltipText}
              </div>
            </div>)}
        </Transition>
      </div>);
  }

  private showToolTip() {
    this.setState({ showToolTip: true });
  }

  private hideToolTip() {
    this.setState({ showToolTip: false });
  }

  private getText(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        return RoleIcon.TRADER_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.MANAGER:
        return RoleIcon.MANAGER_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        return RoleIcon.ADMINISTRATOR_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.SERVICE:
        return RoleIcon.SERVICE_TOOLTIP_TEXT;
    }
  }

  private getSource(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        return 'resources/account/trader-';
      case Nexus.AccountRoles.Role.MANAGER:
        return 'resources/account/manager-';
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        return 'resources/account/admin-';
      case Nexus.AccountRoles.Role.SERVICE:
        return 'resources/account/service-';
    }
  }

  private static ANIMATION_STYLE = {
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    },
    exited: {
      display: 'none' as 'none'
    }
  };
  private static STYLE = {
    iconBox: {
      position: 'relative' as 'relative'
    },
    icon: {
      cursor: 'pointer'
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
      position: 'absolute' as 'absolute',
      zIndex: 5,
      top: '28px',
      left: '-2px',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      boxShadow: '0px 0px 2px #00000064'
    }
  };
  private static readonly IMAGE_SIZE = '20px';
  private static readonly TIMEOUT = 100;
  private static readonly TRADER_TOOLTIP_TEXT = 'Trader';
  private static readonly MANAGER_TOOLTIP_TEXT = 'Manager';
  private static readonly ADMINISTRATOR_TOOLTIP_TEXT = 'Admin';
  private static readonly SERVICE_TOOLTIP_TEXT = 'Service';
}
