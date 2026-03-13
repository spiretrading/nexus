import * as Beam from 'beam';
import * as React from 'react';
import { Select } from '..';
import { DateInput } from './date_input';
import { DurationInput } from './duration_input';

enum Periods {
  AM,
  PM
}

interface Properties {

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
    return (
      <div style={DateTimeField.STYLE.outerWrapper}>
        <DateInput
          value={this.value.date}
          readonly={this.props.readonly}
          onChange={this.onDateChange}/>
        <div style={DateTimeField.STYLE.filler}/>
        <div style={DateTimeField.STYLE.durationWrapper}>
          <DurationInput
            value={this.state.displayedTime}
            readonly={this.props.readonly}
            maxHourValue={12}
            minHourValue={1}
            onChange={this.onTimeChange}/>
          <Select
              onChange={this.onPeriodChange}
              value={String(this.state.period)}
              readonly={this.props.readonly}
              style={DateTimeField.STYLE.select}>
            <option value={String(Periods.AM)}>AM</option>
            <option value={String(Periods.PM)}>PM</option>
          </Select>
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

  private onPeriodChange = (value: string) => {
    const period = parseInt(value);
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
      marginLeft: '10px',
      width: '64px'
    } as React.CSSProperties,
    filler: {
      height: '10px',
      width: '100%'
    } as React.CSSProperties
  };
}
