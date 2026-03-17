import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value: any;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

interface State {
  localValue: string;
}

/** A css input field. */
export class CSSInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      localValue: JSON.stringify(this.props.value)
    };
  }

  public render(): JSX.Element {
    return <textarea
      value={this.state.localValue}
      onChange={this.onChange}
      onBlur={this.onBlur}/>;
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({localValue: JSON.stringify(this.props.value)});
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLTextAreaElement>) => {
    this.setState({localValue: event.target.value});
  }

  private onBlur = () => {
    try {
      const css = JSON.parse(this.state.localValue);
      this.props.update(css);
    } catch (error) {
      this.setState({localValue: JSON.stringify(this.props.value)});
    }
  }
}
