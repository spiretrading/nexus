import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

let nextId = 0;

interface Properties {

  /** The unique id for the checkbox. Auto-generated if not provided. */
  checkboxId?: string;

  /** The label for the filter chip. */
  label: string;

  /** Whether the chip is checked. */
  isChecked?: boolean;

  /** Whether the chip is disabled. */
  disabled?: boolean;

  /** Called when the checked state changes. */
  onChange?: (isChecked: boolean) => void;
}

interface State {
  isFocusVisible: boolean;
}

/** A toggleable filter chip. */
export class FilterChip extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.checkboxId = props.checkboxId || `filter-chip-${nextId++}`;
    this.state = {
      isFocusVisible: false
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
    return (
      <div className={css(STYLES.container)}>
        <input type='checkbox' id={this.checkboxId}
          checked={this.props.isChecked}
          disabled={this.props.disabled}
          onChange={this.onInputChange}
          onFocus={this.onFocus}
          onBlur={this.onBlur}
          className={css(STYLES.input)}/>
        <label htmlFor={this.checkboxId}
            className={css(STYLES.label, labelStyle,
              this.state.isFocusVisible && STYLES.focusVisible)}>
          {this.props.label}
        </label>
      </div>);
  }

  private onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.onChange?.(event.target.checked);
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

  private checkboxId: string;
}

const STYLES = StyleSheet.create({
  container: {
    display: 'inline-flex',
    position: 'relative'
  },
  input: {
    position: 'absolute',
    width: '100%',
    height: '100%',
    appearance: 'none',
    WebkitAppearance: 'none',
    MozAppearance: 'none',
    outline: 'none',
    pointerEvents: 'none',
    border: 'none',
    margin: 0
  },
  label: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    padding: '0 17px',
    borderRadius: '17px',
    lineHeight: '34px',
    outlineOffset: '2px',
    outline: 'transparent solid 2px',
    userSelect: 'none',
    whiteSpace: 'nowrap'
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
  }
});
