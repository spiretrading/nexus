import * as Nexus from 'nexus';
import * as React from 'react';
import { QueryModel } from '../models';
import { ComboBox } from './combo_box';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'value' | 'defaultValue' | 'onChange'> {

  /** The selected ticker. Defaults to an empty field. */
  value?: Nexus.Ticker;

  /** The model used to parse tickers and load suggestions. */
  model: QueryModel<Nexus.Ticker>;

  /** Called with one ticker, or Ticker.NONE when the field is cleared. */
  onChange?: (value: Nexus.Ticker) => void;
}

/** Selects one ticker or accepts a properly formatted ticker directly. */
export class TickerInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const {value, model, onChange, ...rest} = this.props;
    let selected = value ?? null;
    if(selected?.equals(Nexus.Ticker.NONE)) {
      selected = null;
    }
    return <ComboBox {...rest} value={selected} model={model}
      getLabel={getLabel} isEqual={isEqual}
      placeholder={this.props.placeholder ?? 'Enter ticker'}
      invalidMessage='Enter a qualified ticker, such as ABX.TSX.'
      onChange={this.onChange}/>;
  }

  private onChange = (value: Nexus.Ticker) => {
    this.props.onChange?.(value ?? Nexus.Ticker.NONE);
  };
}

function getLabel(ticker: Nexus.Ticker): string {
  return ticker.toString();
}

function isEqual(first: Nexus.Ticker, second: Nexus.Ticker): boolean {
  return first.equals(second);
}
