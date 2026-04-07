import * as React from 'react';

interface Properties<T> {

  /** The value of the field. */
  value?: T;

  /** The callback to update the value. */
  update: (newValue: T) => void;
}

interface State<T>{
  localValue: T;
  isUndefined: boolean;
}

/** 
 * Wraps am existing input. It allows the input inside it to be toggled between
 * undefined and its current value.
 * @param PropertyInput - The input to use.
 * @return A React component class that will render a input field with a
 *  optional value.
 */
export function OptionalInput<T>(PropertyInput: typeof React.Component) {
  return class extends React.Component<Properties<T>, State<T>> {
    constructor(props: Properties<T>) {
      super(props);
      this.state = {
        localValue: this.props.value,
        isUndefined: false
      };
    }

    public render(): JSX.Element {
      return (
        <div style={this.STYLE.container}>
          <div style={this.STYLE.row}>
            <div>Is undefined?</div>
            <input type='checkbox'
              checked={this.state.isUndefined}
              onChange={this.toggleUndefined}/>
          </div>
          <PropertyInput
            value={this.state.localValue}
            update={this.updateLocal}/>
        </div>);
    }

    public componentDidUpdate(prevProps: Properties<T>) {
      if(this.props.value !== prevProps.value && !this.state.isUndefined) {
        this.setState({localValue: this.props.value});
      }
    }

    private updateLocal = (value: any) => {
      this.setState({localValue: value});
      if(!this.state.isUndefined) {
        this.props.update(value);
      }
    }

    private toggleUndefined = (event: React.ChangeEvent<HTMLInputElement>) => {
      const isUndefined = event.target.checked;
      this.setState({isUndefined});
      if(isUndefined) {
        this.props.update(undefined);
      } else {
        this.props.update(this.state.localValue);
      }
    }

    private readonly STYLE = {
      container: {
        width: '100%',
        display: 'flex',
        flexDirection: 'column'
      } as React.CSSProperties,
      row: {
        display: 'flex',
        flexDirection: 'row',
        justifyContent: 'space-between',
        fontSize: '14px',
        fontWeight: 400,
        fontFamily: 'Roboto',
        color: '#000000',
        paddingBottom: '5px'
      } as React.CSSProperties,
    };
  }
}
