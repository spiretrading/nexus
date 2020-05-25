import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '..';
import { DateField } from './date_field';
import { DurationField } from './duration_field';

enum Periods {
  AM,
  PM
}

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.DateTime;

  /** Determines if the field is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange: (value: Beam.DateTime) => void;
}

interface State {
  displayedTime: Beam.Duration;
  period: Periods;
}

/** A component that displays both date and time. */
export class DateTimeField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.DateTime(new Beam.Date(1, 1, 1990), new Beam.Duration(0)),
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      period: Periods.AM,
      displayedTime: this.props.value.timeOfDay()
    };
    this.onPeriodChange = this.onPeriodChange.bind(this);
    this.onTimeChange = this.onTimeChange.bind(this);
    this.onDateChange = this.onDateChange.bind(this);
  }

  public render(): JSX.Element {
    const selectionStyle = (() => {
      if(this.props.readonly) {
        return DateTimeField.STYLE.selectReadonly;
      } else {
        return DateTimeField.STYLE.select;
      }
    })();
    return (
      <div style={DateTimeField.STYLE.outerWrapper}>
        <DateField
          displaySize={this.props.displaySize}
          value={this.props.value.date()}
          readonly={this.props.readonly}
          onChange={this.onDateChange}/>
        <div style={DateTimeField.STYLE.filler}/>
        <div style={DateTimeField.STYLE.durationWrapper}>
          <DurationField 
            displaySize={this.props.displaySize}
            value={this.state.displayedTime}
            readonly={this.props.readonly}
            maxHourValue={12}
            minHourValue={1}
            onChange={this.onTimeChange}/>
          <select style={selectionStyle}
              onChange={this.onPeriodChange}
              value={this.state.period}
              disabled={this.props.readonly}
              className={css(DateTimeField.EXTRA_STYLE.effects)}>
            <option value={Periods.AM}>{'AM'}</option>
            <option value={Periods.PM}>{'PM'}</option>
          </select>
        </div>
      </div>);
  }
  
  public componentDidMount() {
    this.setState({
      period: this.getPeriod(),
      displayedTime: this.getTimeIn12HourFormat()
    });
  }

  private getPeriod() {
    const sourceTime = this.props.value.timeOfDay().split();
    if(sourceTime.hours === 0 || sourceTime.hours === 24) {
      return Periods.AM;
    } else if(sourceTime.hours >= 12) {
      return Periods.PM;
    } else {
      return Periods.AM;
    }
  }

  private getTimeIn12HourFormat() {
    const sourceTime = this.props.value.timeOfDay().split();
    if(sourceTime.hours === 0 || sourceTime.hours === 24) {
      return Beam.Duration.HOUR.multiply(12).add(
        Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
        Beam.Duration.SECOND.multiply(sourceTime.seconds));
    } else if(sourceTime.hours > 12) {
      return Beam.Duration.HOUR.multiply(sourceTime.hours - 12).add(
        Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
        Beam.Duration.SECOND.multiply(sourceTime.seconds));
    } else if(sourceTime.hours === 12) {
      return Beam.Duration.HOUR.multiply(12).add(
        Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
        Beam.Duration.SECOND.multiply(sourceTime.seconds));
    } else {
      return this.props.value.timeOfDay();
    }
  }

  private getTimeIn24HourFormat() {
    const sourceTime = this.state.displayedTime.split();
    if(this.state.period === Periods.PM) {
      if(sourceTime.hours === 12) {
        return Beam.Duration.HOUR.multiply(sourceTime.hours).add(
          Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
          Beam.Duration.SECOND.multiply(sourceTime.seconds));
      } else {
        return Beam.Duration.HOUR.multiply(sourceTime.hours + 12).add(
          Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
          Beam.Duration.SECOND.multiply(sourceTime.seconds));
      }
    } else {
      if(sourceTime.hours === 12) {
        return Beam.Duration.HOUR.multiply(0).add(
          Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
          Beam.Duration.SECOND.multiply(sourceTime.seconds));
      } else {
        return Beam.Duration.HOUR.multiply(sourceTime.hours).add(
          Beam.Duration.MINUTE.multiply(sourceTime.minutes)).add(
          Beam.Duration.SECOND.multiply(sourceTime.seconds));
      }
    }
  }

  private onPeriodChange(event: React.ChangeEvent<HTMLSelectElement>): void {
    const period = parseInt(event.target.value);
    this.setState({
      period: period,
      displayedTime: this.getTimeIn12HourFormat()
    });
    this.props.onChange(new Beam.DateTime(this.props.value.date(),
      this.getTimeIn24HourFormat()));
  }

  private onDateChange(date: Beam.Date) {
    this.props.onChange(new Beam.DateTime(date, this.props.value.timeOfDay()));
  }

  private onTimeChange(time: Beam.Duration) {
    this.setState({displayedTime: time});
    this.props.onChange(new Beam.DateTime(this.props.value.date(),
      this.getTimeIn24HourFormat()));
  }

  private static readonly STYLE = {
    outerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      height: '78px'
    },
    durationWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row'
    },
    select: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      marginLeft: '10px',
      paddingLeft: '7px',
      width: '64px',
      height: '34px',
      flexGrow: 0,
      flexShrink: 0,
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage: 'url(resources/components/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none'
    },
    selectReadonly: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      marginLeft: '10px',
      paddingLeft: '7px',
      width: '64px',
      height: '34px',
      flexGrow: 0,
      flexShrink: 0,
      color: '#333333',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#FFFFFF',
      backgroundImage: 'url(resources/components/arrow-down-grey.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none'
    },
    filler: {
      height: '10px',
      width: '100%'
    }
  };
  public static readonly EXTRA_STYLE = StyleSheet.create({
    focusEffects: {
      outlineColor: 'transparent',
      outlineStyle: 'none',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    },
    effects: {
      ':focus': {
        outline: 0,
        outlineColor: 'transparent',
        outlineStyle: 'none',
        border: '1px solid #684BC7',
        borderRadius: '1px'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      },
      '-webkit-user-select': 'text',
      '-moz-user-select': 'text',
      '-ms-user-select': 'text',
      'user-select': 'text'
    }
  });
}
