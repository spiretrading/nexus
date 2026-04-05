import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Beam.Duration;

  /** The callback to update the value. */
  update?: (newValue: Beam.Duration) => void;
}

interface State {
  localValue: string;
}

/** A text input for editing Beam.Duration in HH:MM:SS format. */
export class BeamDurationInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      localValue: BeamDurationInput.format(this.props.value)
    };
  }

  public render(): JSX.Element {
    return <input type='text' placeholder='HH:MM:SS'
      value={this.state.localValue}
      onChange={this.onChange}
      onBlur={this.onBlur}/>;
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({
        localValue: BeamDurationInput.format(this.props.value)
      });
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({localValue: event.target.value});
  }

  private onBlur = () => {
    const parts = this.state.localValue.split(':');
    if(parts.length >= 2 && parts.length <= 3) {
      const hours = parseInt(parts[0]);
      const minutes = parseInt(parts[1]);
      const seconds = parts.length >= 3 ? parseInt(parts[2]) : 0;
      if(!isNaN(hours) && !isNaN(minutes) && !isNaN(seconds)) {
        this.props.update(
          Beam.Duration.HOUR.multiply(hours).add(
          Beam.Duration.MINUTE.multiply(minutes)).add(
          Beam.Duration.SECOND.multiply(seconds)));
        return;
      }
    }
    this.setState({
      localValue: BeamDurationInput.format(this.props.value)
    });
  }

  private static format(duration: Beam.Duration): string {
    const time = duration.split();
    const hours = String(time.hours).padStart(2, '0');
    const minutes = String(time.minutes).padStart(2, '0');
    const seconds = String(Math.floor(time.seconds)).padStart(2, '0');
    return `${hours}:${minutes}:${seconds}`;
  }
}
