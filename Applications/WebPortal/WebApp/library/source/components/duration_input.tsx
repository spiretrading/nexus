import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';

enum TimeUnit {
  HOURS,
  MINUITES,
  SECONDS
}

interface Properties { 
  display: DisplaySize;
  duration: Beam.Duration;
  onChange:() => {}
}

export class DurationInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={DurationInput.STYLE.wrapper}>
      </div>);
  }

  private onChange() {
    
  }

  private static readonly STYLE = {
    wrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row'  as 'row'
    }
  };
}