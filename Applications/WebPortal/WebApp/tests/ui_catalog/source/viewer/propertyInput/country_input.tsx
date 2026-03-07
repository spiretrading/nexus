import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Nexus.CountryCode;

  /** The callback to update the value. */
  update?: (newValue: Nexus.CountryCode) => void;
}

interface State {
  text: string;
}

/** A text input that converts between string and Nexus.CountryCode. */
export class CountryInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: CountryInput.toDisplayText(props.value)
    };
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(this.props.value !== prevProps.value) {
      this.setState({text: CountryInput.toDisplayText(this.props.value)});
    }
  }

  public render(): JSX.Element {
    return <input value={this.state.text} onChange={this.onChange}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const text = event.target.value;
    this.setState({text});
    const trimmed = text.trim();
    const entry = (() => {
      const upper = trimmed.toUpperCase();
      const byLetterCode =
        Nexus.defaultCountryDatabase.fromLetterCode(upper);
      if(!byLetterCode.equals(Nexus.CountryDatabase.Entry.NONE)) {
        return byLetterCode;
      }
      const numeric = parseInt(trimmed, 10);
      if(!isNaN(numeric)) {
        const byNumeric =
          Nexus.defaultCountryDatabase.fromCode(
            new Nexus.CountryCode(numeric));
        if(!byNumeric.equals(Nexus.CountryDatabase.Entry.NONE)) {
          return byNumeric;
        }
      }
      const lower = trimmed.toLowerCase();
      for(const entry of Nexus.defaultCountryDatabase) {
        if(entry.name.toLowerCase() === lower) {
          return entry;
        }
      }
      return Nexus.CountryDatabase.Entry.NONE;
    })();
    if(!entry.equals(Nexus.CountryDatabase.Entry.NONE)) {
      this.props.update?.(entry.code);
    }
  }

  private static toDisplayText(value: Nexus.CountryCode): string {
    const entry = Nexus.defaultCountryDatabase.fromCode(
      value ?? Nexus.CountryCode.NONE);
    if(entry.equals(Nexus.CountryDatabase.Entry.NONE)) {
      return '';
    }
    return entry.twoLetterCode;
  }
}
