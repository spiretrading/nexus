import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { IntegerInputBox } from './integer_input_box';

enum TimeUnit {
  HOURS,
  MINUTES,
  SECONDS
}

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Duration;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange: (value: Beam.Duration) => void;
}

/** A component that displays and lets a user edit a duration. */
export class DurationInputField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    onChange: () => {}
  };

  public render(): JSX.Element {
    const splitTransitionTime = this.props.value.split();
    const wrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationInputField.STYLE.wrapperSmall;
      } else {
        return DurationInputField.STYLE.wrapperLarge;
      }
    })();
    const integerInputStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationInputField.STYLE.integerBoxSmall;
      } else {
        return DurationInputField.STYLE.integerBoxLarge;
      }
    })();
    return (
      <div style={wrapperStyle}>
        <div style={DurationInputField.STYLE.inner}>
          <IntegerInputBox
            min={0} max={59}
            value={splitTransitionTime.hours}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            onChange={this.onChange.bind(this, TimeUnit.HOURS)}
            padding={2}/>
          <div style={DurationInputField.STYLE.colon}>{':'}</div>
          <IntegerInputBox
            min={0} max={59}
            value={splitTransitionTime.minutes}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            onChange={this.onChange.bind(this, TimeUnit.MINUTES)}
            padding={2}/>
          <div style={DurationInputField.STYLE.colon}>{':'}</div>
          <IntegerInputBox
            min={0} max={59}
            value={splitTransitionTime.seconds}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={integerInputStyle}
            onChange={this.onChange.bind(this, TimeUnit.SECONDS)}
            padding={2}/>
          </div>
          <div style={DurationInputField.STYLE.placeholder}>
            {'Hr : Min : Sec'}
          </div>
      </div>);
  }

  private onChange(timeUnit: TimeUnit, value: number) {
    const oldDuration = this.props.value.split();
    const newValue = (() => {
      switch(timeUnit) {
        case TimeUnit.HOURS:
          return Beam.Duration.HOUR.multiply(value).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.MINUTES:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(value)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.SECONDS:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(value));
      }
    })();
    this.props.onChange(newValue);
  }

  private static readonly STYLE = {
    wrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '184px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
    },
    wrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
    },
    inner: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      justifyContent: 'flex-start' as 'flex-start',
      marginLeft: '9px' 
    },
    integerBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '16px',
      maxWidth: '16px',
      width: '100%',
      height: '34px',
      flexGrow: 1,
      flexShrink: 1,
      border: '0px solid #ffffff',
    },
    integerBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '16px',
      maxWidth: '16px',
      height: '34px',
      border: '0px solid #ffffff',
    },
    colon: {
      width: '10px',
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center'
    },
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        ouline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
}
