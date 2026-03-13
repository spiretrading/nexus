import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '..';
import { DateInput } from './date_input';
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
  onChange?: (value: Beam.DateTime) => void;
}

interface State {
  displayedTime: Beam.Duration;
  period: Periods;
}

/** A component that displays both date and time. */
export class DateTimeField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      period: Periods.AM,
      displayedTime: this.value.timeOfDay
    };
  }

  private get value(): Beam.DateTime {
    return this.props.value ?? DateTimeField.today();
  }

  private static today(): Beam.DateTime {
    const now = new Date();
    return new Beam.DateTime(
      new Beam.Date(now.getFullYear(), now.getMonth() + 1, now.getDate()),
      new Beam.Duration(0));
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
        <DateInput
          value={this.value.date}
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
            <option value={Periods.AM}>AM</option>
            <option value={Periods.PM}>PM</option>
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

  public componentDidUpdate(prevProps: Properties) {
    if(!this.value.equals(prevProps.value)) {
      this.setState({
        period: this.getPeriod(),
        displayedTime: this.getTimeIn12HourFormat()
      });
    }
  }

  private getPeriod = () => {
    const sourceTime = this.value.timeOfDay.split();
    if(sourceTime.hours === 0 || sourceTime.hours === 24) {
      return Periods.AM;
    } else if(sourceTime.hours >= 12) {
      return Periods.PM;
    } else {
      return Periods.AM;
    }
  }

  private getTimeIn12HourFormat = () => {
    const sourceTime = this.value.timeOfDay.split();
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
      return this.value.timeOfDay;
    }
  }

  private getTimeIn24HourFormat = (
      displayedTime: Beam.Duration, period: Periods) => {
    const sourceTime = displayedTime.split();
    if(period === Periods.PM) {
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

  private onPeriodChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const period = parseInt(event.target.value);
    this.setState({
      period: period,
      displayedTime: this.getTimeIn12HourFormat()
    });
    this.props.onChange?.(new Beam.DateTime(this.value.date,
      this.getTimeIn24HourFormat(this.state.displayedTime, period)));
  }

  private onDateChange = (date: Beam.Date) => {
    this.props.onChange?.(new Beam.DateTime(date, this.value.timeOfDay));
  }

  private onTimeChange = (time: Beam.Duration) => {
    this.setState({displayedTime: time});
    this.props.onChange?.(new Beam.DateTime(this.value.date,
      this.getTimeIn24HourFormat(time, this.state.period)));
  }

  private static readonly STYLE = {
    outerWrapper: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'column',
      height: '78px'
    } as React.CSSProperties,
    durationWrapper: {
      display: 'flex',
      flexDirection: 'row'
    } as React.CSSProperties,
    select: {
      boxSizing: 'border-box',
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
      MozAppearance: 'none',
      WebkitAppearance: 'none',
      appearance: 'none'
    } as React.CSSProperties,
    selectReadonly: {
      boxSizing: 'border-box',
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
      MozAppearance: 'none',
      WebkitAppearance: 'none',
      appearance: 'none'
    } as React.CSSProperties,
    filler: {
      height: '10px',
      width: '100%'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
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
