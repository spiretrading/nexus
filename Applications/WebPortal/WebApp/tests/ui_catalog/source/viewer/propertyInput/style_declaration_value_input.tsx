import {StyleDeclarationValue, StyleSheet} from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value: StyleDeclarationValue;

  /** The callback to update the value. */
  update?: (newValue: StyleDeclarationValue) => void;
}

interface State {
  localValue: string;
}

/**
 * An input field for building aphrodite StyleDeclarationValues from a string.
 */
export class StyleDeclarationValueInput extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      localValue: StyleDeclarationValueInput.getString(this.props.value)
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
      this.setState({
        localValue: StyleDeclarationValueInput.getString(this.props.value)
      });
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLTextAreaElement>) => {
    this.setState({localValue: event.target.value});
  }

  private onBlur = () => {
    try {
      this.props.update(StyleSheet.create(
        {
          demo: JSON.parse(this.state.localValue)
        }).demo);
    } catch (error) {
      this.setState({
        localValue: StyleDeclarationValueInput.getString(this.props.value)
      });
    }
  }

  private static getString(value: StyleDeclarationValue): string {
    try {
      return JSON.stringify((value as any)._definition);
    } catch {
      return '';
    }
  }
}
