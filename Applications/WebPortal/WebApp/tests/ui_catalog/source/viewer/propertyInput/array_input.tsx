import * as React from 'react';
import {PropertySchema} from '../../data';

interface Properties<T> {

  /** The value of the field. */
  value?: T[];

  /** The callback to update the value. */
  update?: (newValue: T[]) => void;
}

/**
 * Provides a way to manipulate a array. The length can be changed and each
 * element can be edited.
 * @param propertySchema - The schema for a individual element of the array.
 * @return A React component class that will render the array input field.
 */
export function ArrayInput<T>(propertySchema: PropertySchema) {
  return class extends React.Component<Properties<T>> {
    public render(): JSX.Element {
      return (
        <div style={this.STYLE.container}>
          <div style={this.STYLE.addRemoveButtons}>
            <button onClick={() => this.onChangeLength(1)}>Add item</button>
            <button onClick={() => this.onChangeLength(-1)}>Remove item</button>
          </div>
          {this.props.value.map((value, index) => this.renderInput(value, index))}
        </div>);
    }

    private renderInput = (value: any, index: number) => {
      return (
        <div style={this.STYLE.inputPadding} key={index}>
          <propertySchema.render value={value}
            onUpdate={(newValue: any) => this.updateElement(index, newValue)}
            update={(newValue: any) => this.updateElement(index, newValue)}/>
        </div>);
    }

    private updateElement = (index: number, value: any) => {
      const valueCopy = this.props.value.slice();
      valueCopy[index] = value;
      this.props.update(valueCopy);
    }

    private onChangeLength = (increment: number) => {
      const length = this.props.value.length + increment;
      if(increment > 0) {
        const valueCopy = this.props.value.slice();
        valueCopy.push(propertySchema.defaultValue);
        this.props.update(valueCopy);
      } else if(length >= 0) {
        this.props.update(this.props.value.slice(0, length));
      }
    }

    private readonly STYLE = {
      container: {
        width: '100%',
        display: 'flex',
        flexDirection: 'column'
      } as React.CSSProperties,
      addRemoveButtons: {
        display: 'flex',
        justifyContent: 'space-evenly',
        marginBottom: '15px'
      } as React.CSSProperties,
      inputPadding: {
        padding: '5px 0 5px 0'
      } as React.CSSProperties
    };
  }
}
