import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Beam.Duration;

  /** The callback to update the value. */
  update?: (newValue: Beam.Duration) => void;
}

/** A time input for editing a Beam.Duration representing a time of day. */
export class BeamTimeOfDayInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const time = this.props.value.split();
    const hours = String(time.hours).padStart(2, '0');
    const minutes = String(time.minutes).padStart(2, '0');
    const seconds = String(Math.floor(time.seconds)).padStart(2, '0');
    return <input type='time' step='1'
      value={`${hours}:${minutes}:${seconds}`}
      onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const parts = event.target.value.split(':');
    if(parts.length >= 2) {
      const hours = parseInt(parts[0]);
      const minutes = parseInt(parts[1]);
      const seconds = parts.length >= 3 ? parseInt(parts[2]) : 0;
      this.props.update(
        Beam.Duration.HOUR.multiply(hours).add(
        Beam.Duration.MINUTE.multiply(minutes)).add(
        Beam.Duration.SECOND.multiply(seconds)));
    }
  }
}
