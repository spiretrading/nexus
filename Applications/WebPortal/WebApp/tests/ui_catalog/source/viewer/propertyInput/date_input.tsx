import * as React from 'react'
import { format } from 'date-fns'

interface Properties {

  /** The value of the field. */
  value?: Date;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

interface State {
  localValue: Date;
}

/** A date input field. */
export class DateInput extends React.Component<Properties, State> {
  public static readonly defaultProps: Partial<Properties> = {
    value: new Date()
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      localValue: this.props.value
    };
  }

  public render(): JSX.Element {
    return <input type='date'
      value={format(this.state.localValue, 'yyyy-MM-dd')}
      onChange={this.onChange}/>;
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({localValue: this.props?.value});
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(new Date(event.target.value));
  }
}
