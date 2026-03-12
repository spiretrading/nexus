import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

type ButtonAttributes = React.ButtonHTMLAttributes<HTMLButtonElement>;

interface Properties extends ButtonAttributes {

  /** The full path for the icon. */
  icon: string;

  /** The text label to display inside the button. */
  label: string;

  /** The visual representation of the button. Defaults to ICON. */
  variant?: IconLabelButton.Variant;

  /** The location of the icon relative to the label. Defaults to LEADING. */
  iconPlacement?: IconLabelButton.Placement;
}

interface State {
  isHovered: boolean;
}

/** Displays a button with an icon and/or label. */
export class IconLabelButton extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isHovered: false
    };
  }

  public render(): JSX.Element {
    const { icon, label, variant: variantProp, iconPlacement: placementProp,
      ...buttonProps } = this.props;
    const variant = variantProp ?? IconLabelButton.Variant.ICON;
    const iconPlacement = placementProp ??
      IconLabelButton.Placement.LEADING;
    const fillColor = (() => {
      if(buttonProps.disabled) {
        return '#C8C8C8';
      }
      if(this.state.isHovered) {
        return '#4B23A0';
      }
      return '#684BC7';
    })();
    const iconStyle = {
      width: '1.25rem',
      height: '1.25rem',
      backgroundColor: fillColor,
      WebkitMaskImage: `url(${icon})`,
      maskImage: `url(${icon})`,
      WebkitMaskSize: 'contain',
      maskSize: 'contain',
      WebkitMaskRepeat: 'no-repeat',
      maskRepeat: 'no-repeat'
    };
    if(variant === IconLabelButton.Variant.ICON_LABEL) {
      const isTrailing =
        iconPlacement === IconLabelButton.Placement.TRAILING;
      const labelStyle = {
        ...IconLabelButton.INLINE_STYLES.label,
        color: fillColor
      };
      return (
        <button {...buttonProps} className={css(IconLabelButton.STYLES.button,
            IconLabelButton.STYLES.iconLabelButton)}
            onMouseEnter={this.onMouseEnter}
            onMouseLeave={this.onMouseLeave}>
          <div className={css(IconLabelButton.STYLES.content)}>
            {isTrailing &&
              <>
                <span style={labelStyle}>{label}</span>
                <div className={css(IconLabelButton.STYLES.spacer)}/>
              </>}
            <div aria-hidden style={iconStyle}/>
            {!isTrailing &&
              <>
                <div className={css(IconLabelButton.STYLES.spacer)}/>
                <span style={labelStyle}>{label}</span>
              </>}
          </div>
        </button>);
    }
    return (
      <button {...buttonProps} className={css(IconLabelButton.STYLES.button,
            IconLabelButton.STYLES.iconButton)}
          onMouseEnter={this.onMouseEnter}
          onMouseLeave={this.onMouseLeave}>
        <div className={css(IconLabelButton.STYLES.content)}>
          <div aria-hidden style={iconStyle}/>
        </div>
      </button>);
  }

  private onMouseEnter = () => {
    this.setState({isHovered: true});
  }

  private onMouseLeave = () => {
    this.setState({isHovered: false});
  }

  private static readonly INLINE_STYLES = {
    label: {
      fontSize: '0.875rem',
      fontFamily: 'Roboto',
      fontWeight: 500,
      whiteSpace: 'nowrap'
    }
  };

  private static readonly STYLES = StyleSheet.create({
    button: {
      display: 'inline-flex',
      alignItems: 'center',
      justifyContent: 'flex-start',
      height: '34px',
      padding: 0,
      border: '1px solid transparent',
      borderRadius: '1px',
      backgroundColor: '#FFFFFF',
      cursor: 'pointer',
      boxSizing: 'border-box',
      outline: 'none',
      ':hover': {
        backgroundColor: '#F8F8F8'
      },
      ':focus-visible': {
        borderColor: '#684BC7'
      }
    },
    iconButton: {
      width: '34px',
      justifyContent: 'center'
    },
    iconLabelButton: {
      padding: '0 11px'
    },
    content: {
      display: 'flex',
      alignItems: 'center'
    },
    spacer: {
      width: '8px',
      flexShrink: 0
    }
  });
}

export namespace IconLabelButton {

  /** The visual representation of the button. */
  export enum Variant {

    /** Displays only the icon. */
    ICON,

    /** Displays both the icon and the label. */
    ICON_LABEL
  }

  /** The location of the icon relative to the label. */
  export enum Placement {

    /** The icon comes before the label. */
    LEADING,

    /** The icon comes after the label. */
    TRAILING
  }
}
