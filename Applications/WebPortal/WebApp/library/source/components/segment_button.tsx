import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

let nextId = 0;

interface Properties {

  /** The unique id for the radio button. Auto-generated if not provided. */
  radioId?: string;

  /** The name of the radio button group. */
  name: string;

  /** The label displayed in the segment button. */
  label: string;

  /** The supplemental content displayed in a badge after the label.
   *  If empty or undefined, the badge is hidden. */
  badge?: string;

  /** Whether the segment is checked. */
  isChecked?: boolean;

  /** Whether the segment is disabled. */
  disabled?: boolean;

  /** Called when this segment is selected. */
  onChange?: () => void;
}

interface State {
  isFocusVisible: boolean;
  isHovered: boolean;
}

/** A single segment within a segmented button group. */
export class SegmentButton extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.radioId = props.radioId || `segment-button-${nextId++}`;
    this.state = {
      isFocusVisible: false,
      isHovered: false
    };
  }

  public render(): JSX.Element {
    const labelStyle = (() => {
      if(this.props.disabled && this.props.isChecked) {
        return STYLES.labelDisabledChecked;
      } else if(this.props.disabled) {
        return STYLES.labelDisabled;
      } else if(this.props.isChecked) {
        return STYLES.labelChecked;
      }
      return STYLES.labelDefault;
    })();
    const badgeStyle = (() => {
      if(this.props.disabled && this.props.isChecked) {
        return STYLES.badgeDisabledChecked;
      } else if(this.props.disabled) {
        return STYLES.badgeDisabled;
      } else if(this.props.isChecked && this.state.isHovered) {
        return STYLES.badgeCheckedHover;
      } else if(this.props.isChecked) {
        return STYLES.badgeChecked;
      } else if(this.state.isHovered) {
        return STYLES.badgeHover;
      }
      return STYLES.badgeDefault;
    })();
    return (
      <div className={css(STYLES.container)}>
        <input type='radio' id={this.radioId}
          name={this.props.name}
          checked={this.props.isChecked}
          disabled={this.props.disabled}
          onChange={this.onInputChange}
          onFocus={this.onFocus}
          onBlur={this.onBlur}
          className={css(STYLES.input)}/>
        <label htmlFor={this.radioId}
            onMouseEnter={this.onMouseEnter}
            onMouseLeave={this.onMouseLeave}
            className={css(STYLES.label, labelStyle,
              this.state.isFocusVisible && STYLES.focusVisible)}>
          {this.props.label}
          {this.props.badge &&
            <span className={css(STYLES.badge, badgeStyle)}>
              {this.props.badge}
            </span>}
        </label>
      </div>);
  }

  private onMouseEnter = () => {
    this.setState({isHovered: true});
  };

  private onMouseLeave = () => {
    this.setState({isHovered: false});
  };

  private onInputChange = () => {
    this.props.onChange?.();
  };

  private onFocus = (event: React.FocusEvent<HTMLInputElement>) => {
    if(event.currentTarget.matches(':focus-visible')) {
      this.setState({isFocusVisible: true});
    }
  };

  private onBlur = () => {
    if(this.state.isFocusVisible) {
      this.setState({isFocusVisible: false});
    }
  };

  private radioId: string;
}

const STYLES = StyleSheet.create({
  container: {
    display: 'inline-flex',
    position: 'relative' as 'relative'
  },
  input: {
    position: 'absolute' as 'absolute',
    width: '100%',
    height: '100%',
    appearance: 'none',
    WebkitAppearance: 'none',
    MozAppearance: 'none',
    outline: 'none',
    pointerEvents: 'none',
    border: 'none',
    margin: 0,
    ':focus-visible': {
      outline: 'none',
      border: 'none',
      boxShadow: 'none'
    }
  },
  label: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    textAlign: 'center' as 'center',
    padding: '0 9px',
    borderRadius: '1px',
    lineHeight: '34px',
    outlineOffset: '0px',
    outline: 'transparent solid 1px',
    userSelect: 'none',
    whiteSpace: 'nowrap',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    flexGrow: 1,
    gap: '8px'
  },
  labelDefault: {
    color: '#5D5E6D',
    backgroundColor: '#F8F8F8',
    cursor: 'pointer',
    ':hover': {
      backgroundColor: '#E8E8E8'
    }
  },
  labelChecked: {
    color: '#FFFFFF',
    backgroundColor: '#684BC7',
    cursor: 'pointer',
    ':hover': {
      backgroundColor: '#4B23A0'
    }
  },
  labelDisabled: {
    color: '#8C8C8C',
    backgroundColor: '#F8F8F8',
    cursor: 'not-allowed'
  },
  labelDisabledChecked: {
    color: '#FFFFFF',
    backgroundColor: '#C8C8C8',
    cursor: 'not-allowed'
  },
  focusVisible: {
    outlineColor: '#684BC7'
  },
  badge: {
    fontSize: '0.75rem',
    textAlign: 'center' as 'center',
    padding: '2px 4px',
    borderRadius: '4px',
    minWidth: '1.125rem',
    boxSizing: 'border-box',
    userSelect: 'none',
    lineHeight: 'normal'
  },
  badgeDefault: {
    color: '#333333',
    backgroundColor: '#E8E8E8'
  },
  badgeHover: {
    color: '#333333',
    backgroundColor: '#C8C8C8'
  },
  badgeChecked: {
    color: '#684BC7',
    backgroundColor: '#FFFFFF'
  },
  badgeCheckedHover: {
    color: '#4B23A0',
    backgroundColor: '#FFFFFF'
  },
  badgeDisabled: {
    color: '#8C8C8C',
    backgroundColor: '#EBEBEB'
  },
  badgeDisabledChecked: {
    color: '#C8C8C8',
    backgroundColor: '#FFFFFF'
  }
});
