import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';

interface Properties {

  /** The role the icon respresents */
  icon: Nexus.AccountRoles.Role;

  /** Determines if the role is set of not. */
  isSet: boolean;

  /** Called when icon is clicked on. */
  onClick?: () => void;
}

interface State {
  showToolTip: boolean;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesIcon extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      showToolTip: false
    };
    this.showToolTip = this.showToolTip.bind(this);
    this.hideToolTip = this.hideToolTip.bind(this);
    this.setSource = this.setSource.bind(this);
    this.setText = this.setText.bind(this);
    this.setSource(this.props.icon);
    this.setText(this.props.icon);
  }

  public render(): JSX.Element {
    const getIconColour = () => {
      if(this.props.isSet) {
        return 'purple';
      }
      return 'grey';
    };
    return (
        <div style={RolesIcon.STYLE.iconBox}>
          <img src={`${this.source}${getIconColour()}.svg`}
            style={RolesIcon.STYLE.icon}
            width={RolesIcon.IMAGE_SIZE}
            height={RolesIcon.IMAGE_SIZE}
            onClick={() =>
              this.props.onClick()}
            onMouseEnter={() =>
              this.showToolTip()}
            onMouseLeave={() =>
              this.hideToolTip()}/>
          <Transition in={this.state.showToolTip}
              timeout={RolesIcon.TIMEOUT}>
            {(state) => (
              <div style={{...RolesIcon.STYLE.animationBase,
                    ...(RolesIcon.ANIMATION_STYLE as any)[state]}}>
                  <div style={RolesIcon.STYLE.imageTooltip}>
                    {this.tooltipText}
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

  private setText(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        this.tooltipText = RolesIcon.TRADER_TOOLTIP_TEXT;
        break;
      case Nexus.AccountRoles.Role.MANAGER:
        this.tooltipText = RolesIcon.MANAGER_TOOLTIP_TEXT;
        break;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        this.tooltipText = RolesIcon.ADMINISTRATOR_TOOLTIP_TEXT;
        break;
      case Nexus.AccountRoles.Role.SERVICE:
        this.tooltipText = RolesIcon.SERVICE_TOOLTIP_TEXT;
        break;
    }
  }

  private setSource(role: Nexus.AccountRoles.Role) {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        this.source = 'resources/account/trader-';
        break;
      case Nexus.AccountRoles.Role.MANAGER:
        this.source = 'resources/account/manager-';
        break;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        this.source = 'resources/account/admin-';
        break;
      case Nexus.AccountRoles.Role.SERVICE:
        this.source = 'resources/account/service-';
        break;
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
  private source = '';
  private tooltipText = '';
  private static readonly IMAGE_SIZE = '20px';
  private static readonly TIMEOUT = 100;
  private static readonly TRADER_TOOLTIP_TEXT = 'Trader';
  private static readonly MANAGER_TOOLTIP_TEXT  = 'Manager';
  private static readonly ADMINISTRATOR_TOOLTIP_TEXT  = 'Admin';
  private static readonly SERVICE_TOOLTIP_TEXT = 'Service';
}
