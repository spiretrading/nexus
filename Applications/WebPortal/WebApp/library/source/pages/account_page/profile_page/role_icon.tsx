import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize } from '../../..';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The role the icon represents. */
  role: Nexus.AccountRoles.Role;

  /** Whether the roles can be changed. */
  readonly?: boolean;

  /** Determines if the role is set. */
  isSet: boolean;

  /** Determines if the mobile tooltip should be shown. */
  isTouchTooltipShown: boolean;

  /** Called when the icon is clicked on. */
  onClick?: () => void;

  /** Called when the icon is touched. */
  onTouch?: () => void;
}

interface State {
  isMouseTooltipShown: boolean;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RoleIcon extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    onClick: () => {},
    onTouch: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isMouseTooltipShown: false
    };
    this.showTooltipMouse = this.showTooltipMouse.bind(this);
    this.hideToolTipMouse = this.hideToolTipMouse.bind(this);
    this.onTouch = this.onTouch.bind(this);
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const iconColor = (() => {
      if(this.props.isSet) {
        return 'purple';
      }
      return 'grey';
    })();
    const iconStyle = (() => {
      if(this.props.readonly) {
        return RoleIcon.STYLE.readonly;
      } else {
        return RoleIcon.STYLE.clickable;
      }
    })();
    const iconWrapper = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return RoleIcon.STYLE.iconWrapperSmall;
      } else {
        return RoleIcon.STYLE.iconWrapperLarge;
      }
    })();
    return (
      <div style={{...iconWrapper, ...iconStyle}}
          onClick={this.onClick}
          onTouchStart={this.onTouch}
          onMouseEnter={this.showTooltipMouse}
          onMouseLeave={this.hideToolTipMouse}>
        <img src={`${this.getSource(this.props.role)}${iconColor}.svg`}
          style={iconStyle}
          width={RoleIcon.IMAGE_SIZE}
          height={RoleIcon.IMAGE_SIZE}/>
        <div style={RoleIcon.STYLE.tooltipAnchor}>
          <Transition timeout={RoleIcon.TIMEOUT_MOBILE_TOOLTIP}
              in={this.props.isTouchTooltipShown}>
            {(state) => (
              <div style={{...RoleIcon.STYLE.animationBase,
                  ...RoleIcon.STYLE.imageTooltip,
                  ...(RoleIcon.ANIMATION_STYLE as any)[state]}}>
                {this.getText(this.props.role)}
              </div>)}
          </Transition>
          <Transition timeout={RoleIcon.TIMEOUT_TOOLTIP}
              in={this.state.isMouseTooltipShown &&
                !this.props.isTouchTooltipShown}>
            {(state) => (
              <div style={{...RoleIcon.STYLE.animationBase,
                  ...RoleIcon.STYLE.imageTooltip,
                  ...(RoleIcon.ANIMATION_STYLE as any)[state]}}>
                {this.getText(this.props.role)}
              </div>)}
          </Transition>
        </div>
      </div>);
  }

  private showTooltipMouse() {
    if(!this.state.isMouseTooltipShown && !this.props.isTouchTooltipShown) {
      this.setState({isMouseTooltipShown: true});
    }
  }

  private hideToolTipMouse() {
    this.setState({isMouseTooltipShown: false});
  }

  private onClick() {
    if(!this.props.readonly && !this.props.isTouchTooltipShown) {
      this.props.onClick();
    }
  }

  private onTouch() {
    if(!this.props.readonly) {
      this.props.onClick();
    }
    this.props.onTouch();
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
        return '/resources/account/trader-';
      case Nexus.AccountRoles.Role.MANAGER:
        return '/resources/account/manager-';
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        return '/resources/account/admin-';
      case Nexus.AccountRoles.Role.SERVICE:
        return '/resources/account/service-';
    }
  }

  private static readonly ANIMATION_STYLE = {
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
  private static readonly STYLE = {
    iconWrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      height: '24px',
      width: '24px',
      padding: '2px',
      outline: 0
    },
    iconWrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      height: '34px',
      width: '34px',
      padding: '7px',
      outline: 0
    },
    clickable: {
      cursor: 'pointer'
    },
    readonly: {
      cursor: 'inherit'
    },
    tooltipAnchor: {
      position: 'relative' as 'relative',
      height: 0,
      width: 0
    },
    animationBase: {
      opacity: 0,
      transition: 'opacity 200ms ease-in-out'
    },
    imageTooltip: {
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      font: '400 12px Roboto',
      paddingLeft: '15px',
      paddingRight: '15px',
      height: '22px',
      backgroundColor: '#4B23A0',
      color: '#FFFFFF',
      position: 'absolute' as 'absolute',
      top: '16px',
      left: '-20px',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      boxShadow: '0px 0px 2px #00000064',
      tabFocus: 0
    }
  };
  private static readonly TIMEOUT_TOOLTIP = {
    enter: 100,
    entered: 200,
    exit: 200,
    exited: 1
  };
  private static readonly TIMEOUT_MOBILE_TOOLTIP = {
    enter: 1,
    entered: 200,
    exit: 200,
    exited: 1
  };
  private static readonly IMAGE_SIZE = '20px';
  private static readonly TRADER_TOOLTIP_TEXT = 'Trader';
  private static readonly MANAGER_TOOLTIP_TEXT = 'Manager';
  private static readonly ADMINISTRATOR_TOOLTIP_TEXT = 'Admin';
  private static readonly SERVICE_TOOLTIP_TEXT = 'Service';
}
