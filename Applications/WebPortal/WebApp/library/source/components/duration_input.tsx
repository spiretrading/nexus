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
export class DurationInput extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    onChange: () => {}
  };

  public render(): JSX.Element {
    const splitTransitionTime = this.props.value.split();
    const wrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationInput.STYLE.wrapperSmall;
      } else {
        return DurationInput.STYLE.wrapperLarge;
      }
    })();
    const integerInputStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return DurationInput.STYLE.integerBoxSmall;
      } else {
        return DurationInput.STYLE.integerBoxLarge;
      }
    })();
    return (
      <div style={wrapperStyle}>
        <IntegerInputBox
          min={0} max={59}
          value={splitTransitionTime.hours}
          className={css(DurationInput.EXTRA_STYLE.effects)}
          style={integerInputStyle}
          onChange={this.onChange.bind(this, TimeUnit.HOURS)}
          padding={2}/>
        <div style={DurationInput.STYLE.colon}>{':'}</div>
        <IntegerInputBox
          min={0} max={59}
          value={splitTransitionTime.minutes}
          className={css(DurationInput.EXTRA_STYLE.effects)}
          style={integerInputStyle}
          onChange={this.onChange.bind(this, TimeUnit.MINUTES)}
          padding={2}/>
        <div style={DurationInput.STYLE.colon}>{':'}</div>
        <IntegerInputBox
          min={0} max={59}
          value={splitTransitionTime.seconds}
          className={css(DurationInput.EXTRA_STYLE.effects)}
          style={integerInputStyle}
          onChange={this.onChange.bind(this, TimeUnit.SECONDS)}
          padding={2}/>
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
      flexGrow: 1
    },
    wrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      maxWidth: '246px'
    },
    integerBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      minWidth: '44px',
      maxWidth: '64px',
      width: '100%',
      height: '34px',
      flexGrow: 1,
      flexShrink: 1
    },
    integerBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      width: '64px',
      height: '34px'
    },
    colon: {
      width: '26px',
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center'
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
