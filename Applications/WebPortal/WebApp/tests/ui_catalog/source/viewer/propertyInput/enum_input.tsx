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
  return class extends React.Component<Properties> {
    constructor(props: Properties) {
      super(props);
      this._options = [];
      let i = 0;
      while(true) {
        const value = enumType[i];
        if(value !== undefined) {
          this._options.push(
            <option key={i} value={value}>
              {value}
            </option>);
        } else {
          break;
        }
        i += 1;
      }
    }

    public render(): JSX.Element {
      return (
        <select onChange={this.onChange} value={enumType[this.props.value]}>
          {this._options}
        </select>);
    }

    private onChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
      this.props.update(enumType[event.target.value]);
    }

    private _options: JSX.Element[];
  }
}
