import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value: string;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

interface State {
  localValue: string;
}

/** A color input field. */
export class ColorInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      localValue: this.props.value
    };
  }

  public render(): JSX.Element {
    return (
      <div style={ColorInput.STYLE.container}>
        <input
          value={this.state.localValue}
          onChange={this.onChange}
          onBlur={this.onBlur}/>
        <div style={{backgroundColor: this.props.value,
          ...ColorInput.STYLE.colorPreview}}/>
      </div>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({localValue: this.props.value});
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({localValue: event.target.value});
    if(ColorInput.validate(event.target.value)) {
      this.props.update(event.target.value);
    }
  }

  private onBlur = () => {
    if(ColorInput.validate(this.state.localValue)) {
      this.props.update(this.state.localValue);
    } else {
      this.setState({localValue: this.props.value});
    }
  }

  private static validate(color: string): boolean {
    const style = new Option().style;
    style.color = color;
    return Boolean(style.color);
  }

  private static readonly STYLE = {
    container: {
      width: '100%',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'space-between'
    } as React.CSSProperties,
    colorPreview: {
      height: '21.5px',
      width:'20px' 
    }
  };
}
