import * as Nexus from 'nexus';
import * as React from 'react';

const database = Nexus.buildDefaultCurrencyDatabase();

interface Properties {

  /** The value of the field. */
  value?: Nexus.Currency;

  /** The callback to update the value. */
  update?: (newValue: Nexus.Currency) => void;
}

interface State {
  text: string;
}

/** A text input that converts between string and Nexus.Currency. */
export class CurrencyInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: CurrencyInput.toDisplayText(props.value)
    };
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(this.props.value !== prevProps.value) {
      this.setState({text: CurrencyInput.toDisplayText(this.props.value)});
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
      const byCode = database.fromCode(trimmed.toUpperCase());
      if(!byCode.equals(Nexus.CurrencyDatabase.Entry.NONE)) {
        return byCode;
      }
      const numeric = parseInt(trimmed, 10);
      if(!isNaN(numeric)) {
        const byCurrency =
          database.fromCurrency(new Nexus.Currency(numeric));
        if(!byCurrency.equals(Nexus.CurrencyDatabase.Entry.NONE)) {
          return byCurrency;
        }
      }
      return Nexus.CurrencyDatabase.Entry.NONE;
    })();
    if(!entry.equals(Nexus.CurrencyDatabase.Entry.NONE)) {
      this.props.update?.(entry.currency);
    }
  }

  private static toDisplayText(value: Nexus.Currency): string {
    const entry = database.fromCurrency(value ?? Nexus.Currency.NONE);
    if(entry.equals(Nexus.CurrencyDatabase.Entry.NONE)) {
      return '';
    }
    return entry.code;
  }
}
