import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: number;

  /** The callback to update the value. */
  update?: (newValue: number) => void;
}

/**
 * Uses a select combo box to display a list of enums as an input field.
 * @param enumType - The type constructor for the enum to display.
 * @return A React component class that will render the enum input field.
 */
export function EnumInput(enumType: any) {
  const options = Object.keys(enumType)
    .filter(key => isNaN(Number(key)))
    .map(key =>
      <option key={key} value={key}>
        {key}
      </option>);
  return class extends React.Component<Properties> {
    public render(): JSX.Element {
      return (
        <select onChange={this.onChange} value={enumType[this.props.value]}>
          {options}
        </select>);
    }

    private onChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
      this.props.update(enumType[event.target.value]);
    }
  }
}
